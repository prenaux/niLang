// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"
#include "API/niSound_ModuleDef.h"

#define RIFF        0x46464952    // "RIFF"
#define WAVE        0x45564157    // "WAVE"

//////////////////////////////////////////////////////////////////////////////////////////////
// WAV decoding

// Define values for Microsoft WAVE format
static const tU32 FMT            = 0x20746D66; // "fmt "
static const tU32 DATA           = 0x61746164; // "data"
static const tU32 PCM_CODE       = 0x0001;
static const tU32 MS_ADPCM_CODE  = 0x0002;
static const tU32 IMA_ADPCM_CODE = 0x0011;

//Normally, these three chunks come consecutively in a WAVE file
struct sWaveFormat
{
  //
  // Not saved in the chunk we read:
  // tU32 FMTchunk;
  // tU32 fmtlen;
  //
  tU16 encoding;
  tU16 channels;    // 1 = mono, 2 = stereo
  tU32 frequency;   // One of 11025, 22050, or 44100 Hz
  tU32 byterate;    // Average bytes per second
  tU16 blockalign;    // Bytes per sample block
  tU16 bitspersample; // One of 8, 12, 16, or 4 for ADPCM
};

// The general chunk found in the WAVE file
struct sWaveChunk
{
  tU32 magic;
  astl::vector<tU8> data;
};

// For the MS ADPCM decoding
struct sMsADPCMDecodeState
{
  tU8  hPredictor;
  tU16 iDelta;
  tI16 iSamp1;
  tI16 iSamp2;
};

struct sMsADPCMDecoder
{
  sWaveFormat wavefmt;
  tU16 wSamplesPerBlock;
  tU16 wNumCoef;
  tI16 aCoeff[7][2];
  sMsADPCMDecodeState state[2];
};

// For the IMA ADPCM decoding
struct sImaADPCMDecodeState
{
  tI32 sample;
  tI8 index;
};

struct sImaADPCMDecoder
{
  sWaveFormat wavefmt;
  tU16 wSamplesPerBlock;
  sImaADPCMDecodeState state[2];
};

static int ReadChunk(iFile *src, sWaveChunk *chunk);

//////////////////////////////////////////////////////////////////////////////////////////////
// MS ADPCM Decoding
static tBool InitMS_ADPCM(sMsADPCMDecoder& aDecoder, sWaveFormat *format)
{
  tU8* rogue_feel;
  int i;

  // Set the rogue pointer to the MS_ADPCM specific data
  aDecoder.wavefmt.encoding = niSwapLE16(format->encoding);
  aDecoder.wavefmt.channels = niSwapLE16(format->channels);
  aDecoder.wavefmt.frequency = niSwapLE32(format->frequency);
  aDecoder.wavefmt.byterate = niSwapLE32(format->byterate);
  aDecoder.wavefmt.blockalign = niSwapLE16(format->blockalign);
  aDecoder.wavefmt.bitspersample = niSwapLE16(format->bitspersample);

  rogue_feel = (tU8*)format+sizeof(*format);
  if (sizeof(*format) == 16)
  {
    // extra_info = ((rogue_feel[1]<<8)|rogue_feel[0]);
    rogue_feel += sizeof(tU16);
  }

  aDecoder.wSamplesPerBlock = ((rogue_feel[1]<<8)|rogue_feel[0]);
  rogue_feel += sizeof(tU16);

  aDecoder.wNumCoef = ((rogue_feel[1]<<8)|rogue_feel[0]);
  rogue_feel += sizeof(tU16);

  if (aDecoder.wNumCoef != 7)
  {
    niError(_A("Unknown set of MS_ADPCM coefficients."));
    return eFalse;
  }

  for (i = 0; i < aDecoder.wNumCoef; i++)
  {
    aDecoder.aCoeff[i][0] = ((rogue_feel[1]<<8)|rogue_feel[0]);
    rogue_feel += sizeof(tU16);

    aDecoder.aCoeff[i][1] = ((rogue_feel[1]<<8)|rogue_feel[0]);
    rogue_feel += sizeof(tU16);
  }

  return eTrue;
}

static tI32 MS_ADPCM_nibble(struct sMsADPCMDecodeState *state, tU8 nybble, tI16 *coeff)
{
  const tI32 max_audioval = ((1<<(16-1))-1);
  const tI32 min_audioval = -(1<<(16-1));
  const tI32 adaptive[] =
      {
        230, 230, 230, 230, 307, 409, 512, 614,
        768, 614, 512, 409, 307, 230, 230, 230
      };
  tI32 new_sample, delta;

  new_sample = ((state->iSamp1 * coeff[0]) + (state->iSamp2 * coeff[1]))/256;
  if (nybble & 0x08)
  {
    new_sample += state->iDelta * (nybble-0x10);
  }
  else
  {
    new_sample += state->iDelta * nybble;
  }

  if (new_sample < min_audioval)
  {
    new_sample = min_audioval;
  }
  else if (new_sample > max_audioval)
  {
    new_sample = max_audioval;
  }

  delta = ((tI32)state->iDelta * adaptive[nybble])/256;
  if (delta < 16)
  {
    delta = 16;
  }

  state->iDelta = (tU16)delta;
  state->iSamp2 = (tI16)state->iSamp1;
  state->iSamp1 = (tU8)new_sample;

  return new_sample;
}

static tBool MS_ADPCM_decode(sMsADPCMDecoder& aDecoder, tU8 **audio_buf, tU32 *audio_len)
{
  struct sMsADPCMDecodeState *state[2];
  tU8  *freeable, *encoded, *decoded;
  tI32 encoded_len, samplesleft;
  tI8  nybble, stereo;
  tI16 *coeff[2];
  tI32 new_sample;

  // Allocate the proper sized output buffer
  encoded_len = *audio_len;
  encoded = *audio_buf;
  freeable = *audio_buf;
  *audio_len = (encoded_len/aDecoder.wavefmt.blockalign) *
      aDecoder.wSamplesPerBlock*
      aDecoder.wavefmt.channels*sizeof(tI16);

  *audio_buf = (tU8*)niMalloc(*audio_len);
  if (*audio_buf == NULL)
  {
    niError(_A("Unable to alloc audio_buf."));
    return eFalse;
  }

  decoded = *audio_buf;

  stereo = (aDecoder.wavefmt.channels == 2);
  state[0] = &aDecoder.state[0];
  state[1] = &aDecoder.state[stereo];
  while (encoded_len >= aDecoder.wavefmt.blockalign)
  {
    // Grab the initial information for this block
    state[0]->hPredictor = *encoded++;
    if (stereo)
    {
      state[1]->hPredictor = *encoded++;
    }
    state[0]->iDelta = ((encoded[1]<<8)|encoded[0]);
    encoded += sizeof(tI16);
    if (stereo)
    {
      state[1]->iDelta = ((encoded[1]<<8)|encoded[0]);
      encoded += sizeof(tI16);
    }

    state[0]->iSamp1 = ((encoded[1]<<8)|encoded[0]);
    encoded += sizeof(tI16);
    if (stereo)
    {
      state[1]->iSamp1 = ((encoded[1]<<8)|encoded[0]);
      encoded += sizeof(tI16);
    }

    state[0]->iSamp2 = ((encoded[1]<<8)|encoded[0]);
    encoded += sizeof(tI16);
    if (stereo)
    {
      state[1]->iSamp2 = ((encoded[1]<<8)|encoded[0]);
      encoded += sizeof(tI16);
    }

    coeff[0] = aDecoder.aCoeff[state[0]->hPredictor];
    coeff[1] = aDecoder.aCoeff[state[1]->hPredictor];

    // Store the two initial samples we start with
    decoded[0] = state[0]->iSamp2&0xFF;
    decoded[1] = state[0]->iSamp2>>8;
    decoded += 2;
    if (stereo)
    {
      decoded[0] = state[1]->iSamp2&0xFF;
      decoded[1] = state[1]->iSamp2>>8;
      decoded += 2;
    }

    decoded[0] = state[0]->iSamp1&0xFF;
    decoded[1] = state[0]->iSamp1>>8;
    decoded += 2;
    if(stereo)
    {
      decoded[0] = state[1]->iSamp1&0xFF;
      decoded[1] = state[1]->iSamp1>>8;
      decoded += 2;
    }

    // Decode and store the other samples in this block
    samplesleft = (aDecoder.wSamplesPerBlock-2)*aDecoder.wavefmt.channels;
    while (samplesleft > 0)
    {
      nybble = (*encoded)>>4;
      new_sample = MS_ADPCM_nibble(state[0],nybble,coeff[0]);
      decoded[0] = (tU8)(new_sample&0xFF);
      new_sample >>= 8;
      decoded[1] = (tU8)(new_sample&0xFF);
      decoded += 2;

      nybble = (*encoded)&0x0F;
      new_sample = MS_ADPCM_nibble(state[1],nybble,coeff[1]);
      decoded[0] = (tU8)(new_sample&0xFF);
      new_sample >>= 8;
      decoded[1] = (tU8)(new_sample&0xFF);
      decoded += 2;

      encoded++;
      samplesleft -= 2;
    }

    encoded_len -= aDecoder.wavefmt.blockalign;
  }

  niFree(freeable);
  return eTrue;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// IMA ADPCM Decoding
static tBool InitIMA_ADPCM(sImaADPCMDecoder& aDecoder, sWaveFormat *format)
{
  tU8 *rogue_feel;

  aDecoder.wavefmt.encoding = niSwapLE16(format->encoding);
  aDecoder.wavefmt.channels = niSwapLE16(format->channels);
  aDecoder.wavefmt.frequency = niSwapLE32(format->frequency);
  aDecoder.wavefmt.byterate = niSwapLE32(format->byterate);
  aDecoder.wavefmt.blockalign = niSwapLE16(format->blockalign);
  aDecoder.wavefmt.bitspersample = niSwapLE16(format->bitspersample);

  rogue_feel = (tU8*)format+sizeof(*format);

  if (sizeof(*format) == 16)
  {
    // extra_info = ((rogue_feel[1]<<8)|rogue_feel[0]);
    rogue_feel += sizeof(tU16);
  }

  aDecoder.wSamplesPerBlock = ((rogue_feel[1]<<8)|rogue_feel[0]);
  return eTrue;
}

static tI32 IMA_ADPCM_nibble(sImaADPCMDecodeState *state, tU8 nybble)
{
  const tI32 max_audioval = ((1<<(16-1))-1);
  const tI32 min_audioval = -(1<<(16-1));
  const int index_table[16] =
      {
        -1, -1, -1, -1,
        2,  4,  6,  8,
        -1, -1, -1, -1,
        2,  4,  6,  8
      };

  const tI32 step_table[89] =
      {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31,
        34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130,
        143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408,
        449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282,
        1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327,
        3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630,
        9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350,
        22385, 24623, 27086, 29794, 32767
      };

  tI32 delta, step;

  // Compute difference and new sample value
  step = step_table[state->index];
  delta = step >> 3;
  if(nybble & 0x04) delta += step;
  if(nybble & 0x02) delta += (step >> 1);
  if(nybble & 0x01) delta += (step >> 2);
  if(nybble & 0x08) delta = -delta;
  state->sample += delta;

  // Update index value
  state->index += index_table[nybble];
  if(state->index > 88)
  {
    state->index = 88;
  }
  else if(state->index < 0)
  {
    state->index = 0;
  }

  // Clamp output sample
  if(state->sample > max_audioval)
  {
    state->sample = max_audioval;
  }
  else if(state->sample < min_audioval)
  {
    state->sample = min_audioval;
  }

  return state->sample;
}

// Fill the decode buffer with a channel block of data (8 samples)
static void Fill_IMA_ADPCM_block(tU8 *decoded, tU8 *encoded,
                                 int channel, int numchannels, struct sImaADPCMDecodeState *state)
{
  int i;
  tI8 nybble;
  tI32 new_sample;

  decoded += (channel * 2);
  for(i = 0; i < 4; i++)
  {
    nybble = (*encoded)&0x0F;
    new_sample = IMA_ADPCM_nibble(state, nybble);

    decoded[0] = (tU8)(new_sample&0xFF);
    new_sample >>= 8;
    decoded[1] = (tU8)(new_sample&0xFF);
    decoded += 2 * numchannels;

    nybble = (*encoded)>>4;
    new_sample = IMA_ADPCM_nibble(state, nybble);

    decoded[0] = (tU8)(new_sample&0xFF);
    new_sample >>= 8;
    decoded[1] = (tU8)(new_sample&0xFF);
    decoded += 2 * numchannels;

    encoded++;
  }
}

static tBool IMA_ADPCM_decode(sImaADPCMDecoder& aDecoder, tU8 **audio_buf, tU32 *audio_len)
{
  struct sImaADPCMDecodeState *state;
  tU8 *freeable, *encoded, *decoded;
  tI32 encoded_len, samplesleft;
  int c, channels;

  // Check to make sure we have enough variables in the state array
  channels = aDecoder.wavefmt.channels;
  if (channels > niCountOf(aDecoder.state)) {
    niError(niFmt(_A("IMA ADPCM decoder can only handle %d channels"), niCountOf(aDecoder.state)));
    return eFalse;
  }
  state = aDecoder.state;

  // Allocate the proper sized output buffer
  encoded_len = *audio_len;
  encoded = *audio_buf;
  freeable = *audio_buf;
  *audio_len = (encoded_len/aDecoder.wavefmt.blockalign) *  aDecoder.wSamplesPerBlock * aDecoder.wavefmt.channels*sizeof(tI16);

  *audio_buf = (tU8 *)niMalloc(*audio_len);
  if (*audio_buf == NULL)
  {
    niError(_A("Unable to alloc audio_buf."));
    return eFalse;
  }

  decoded = *audio_buf;

  while (encoded_len >= aDecoder.wavefmt.blockalign)
  {
    // Grab the initial information for this block
    for (c = 0; c < channels; c++)
    {
      // Fill the state information for this block
      state[c].sample = ((encoded[1]<<8)|encoded[0]);
      encoded += 2;
      if(state[c].sample & 0x8000)
      {
        state[c].sample -= 0x10000;
      }

      state[c].index = *encoded++;
      // Reserved byte in buffer header, should be 0
      if(*encoded++ != 0)
      {
        // Uh oh, corrupt data? Buggy code?;
      }

      // Store the initial sample we start with
      decoded[0] = (tU8)(state[c].sample&0xFF);
      decoded[1] = (tU8)(state[c].sample>>8);
      decoded += 2;
    }

    // Decode and store the other samples in this block
    samplesleft = (aDecoder.wSamplesPerBlock-1)*channels;
    while(samplesleft > 0)
    {
      for(c = 0; c < channels; c++)
      {
        Fill_IMA_ADPCM_block(decoded, encoded, c, channels, &state[c]);
        encoded += 4;
        samplesleft -= 8;
      }
      decoded += (channels * 8 * 2);
    }
    encoded_len -= aDecoder.wavefmt.blockalign;
  }

  niFree(freeable);
  return 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Misc functions
static int ReadChunk(iFile *src, sWaveChunk *chunk)
{
  chunk->magic = src->ReadLE32();
  tU32 len = src->ReadLE32();
  chunk->data.resize(len);

  if (src->ReadRaw(chunk->data.data(),chunk->data.size()) != len) {
    return -1;
  }

  return chunk->data.size();
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cSoundDataWAV implementation.

class cSoundDataWAV : public ImplRC<iSoundData>
{
  niBeginClass(cSoundDataWAV);

 public:
  eSoundFormat  mFormat;
  tU32          mnFreq;
  Ptr<iFile>    mptrData;

  ///////////////////////////////////////////////
  cSoundDataWAV(iFile* apFile)
  {
    sWaveChunk chunk;

    // Check the file header
    if (apFile->ReadLE32() != RIFF)
    {
      niError(_A("Not a RIFF file."));
      return;
    }

    {
      tU32 nWaveSize = apFile->ReadLE32();
      niUnused(nWaveSize);
    }

    if (apFile->ReadLE32() != WAVE) {
      niError(_A("Not a WAVE riff file."));
      return;
    }

    // Read the audio data format chunk
    tBool foundFMT = eFalse;
    for (;;)
    {
      tI32 readLen = ReadChunk(apFile, &chunk);
      if (readLen < 0) {
        niError(_A("Chunk reading failed."));
        return;
      }
      if (chunk.magic == FMT) {
        foundFMT = eTrue;
        break;
      }
    }
    if (!foundFMT) {
      niError("Couldn't find the FMT chunk.");
      return;
    }

    // audio data format
    tBool bMsADPCM = eFalse;
    sMsADPCMDecoder MsDecoder;
    tBool bImaADPCM = eFalse;
    sImaADPCMDecoder ImaDecoder;

    // Decode the audio data format
    {
      sWaveFormat* format = (sWaveFormat*)chunk.data.data();
      if (chunk.magic != FMT) {
        niError(niFmt("Expected FMT chunk, got '%X'.", niSwapBE32(chunk.magic)));
        return;
      }

      switch (niSwapLE16(format->encoding))
      {
        case PCM_CODE:
          {
            // We can understand this
            break;
          }

        case MS_ADPCM_CODE:
          {
            // Try to understand this
            if (!InitMS_ADPCM(MsDecoder, format))
            {
              niError(_A("Can't initialize the MS ADPCM decoder."));
              return;
            }
            bMsADPCM = eTrue;
            break;
          }

        case IMA_ADPCM_CODE:
          {
            if (!InitIMA_ADPCM(ImaDecoder, format))
            {
              niError(_A("Can't initialize the IMA ADPCM decoder."));
              return;
            }
            bImaADPCM = eTrue;
            break;
          }

        default:
          {
            niError(niFmt(_A("Unknown WAVE data format: 0x%.4x."), niSwapLE16(format->encoding)));
            return;
          }
      }

      switch (niSwapLE16(format->bitspersample))
      {
        case 4:
          {
            if (bMsADPCM || bImaADPCM)
            {
              mFormat = (niSwapLE16(format->channels)==2)?eSoundFormat_Stereo16:eSoundFormat_Mono16;
            }
            else
            {
              niError(_A("Can't decoder 4bits PCM data."));
              return;
            }
            break;
          }

        case 8:
          {
            mFormat = (niSwapLE16(format->channels)==2)?eSoundFormat_Stereo8:eSoundFormat_Mono8;
            break;
          }

        case 16:
          {
            mFormat = (niSwapLE16(format->channels)==2)?eSoundFormat_Stereo16:eSoundFormat_Mono16;
            break;
          }

        default:
          {
            niError(niFmt(_A("Can't decode a %d bits format."), niSwapLE16(format->bitspersample)));
            break;
          }
      }

      mnFreq = niSwapLE32(format->frequency);
    }

    // Read the audio data chunk in memory
    tU32 audio_len = 0;
    tU8* audio_buf = NULL;
    do
    {
      tI32 lenRead = ReadChunk(apFile, &chunk);
      if (lenRead < 0) {
        niError(_A("Can't read chunk for decoding."));
        return;
      }
      audio_len = lenRead;
      audio_buf = chunk.data.data();
    } while(chunk.magic != DATA);

    if (bMsADPCM) {
      if (!MS_ADPCM_decode(MsDecoder, &audio_buf, &audio_len))
      {
        niError(_A("Can't decode MS ADPCM data."));
        return;
      }
    }
    else if(bImaADPCM) {
      if(!IMA_ADPCM_decode(ImaDecoder, &audio_buf, &audio_len)) {
        niError(_A("Can't decode IMA ADPCMA data."));
        return;
      }
    }

    // Don't return a buffer that isn't a multiple of samplesize
    if (mFormat == eSoundFormat_Mono8)   audio_len &= ~1;
    else if (mFormat == eSoundFormat_Mono16)  audio_len &= ~2;
    else if (mFormat == eSoundFormat_Stereo8) audio_len &= ~2;
    else if (mFormat == eSoundFormat_Stereo16)  audio_len &= ~4;

    Ptr<iFile> ptrMem = ni::CreateFileMemoryAlloc(audio_len,NULL);
    memcpy(ptrMem->GetBase(), audio_buf, audio_len);
    mptrData = ptrMem;
  }

  ///////////////////////////////////////////////
  cSoundDataWAV(iFile* apFile, eSoundFormat aFormat, tU32 anFreq) {
    mFormat = aFormat;
    mnFreq = anFreq;
    mptrData = apFile;
  }

  ///////////////////////////////////////////////
  ~cSoundDataWAV() {
    mptrData = NULL;
  }

  ///////////////////////////////////////////////
  //! Sanity check.
  tBool __stdcall IsOK() const
  {
    niClassIsOK(cSoundDataWAV);
    return niIsOK(mptrData);
  }

  ///////////////////////////////////////////////
  //! Get the sound data format.
  eSoundFormat __stdcall GetFormat() const
  {
    return mFormat;
  }

  ///////////////////////////////////////////////
  //! Get the sound frequency in hertz.
  tU32 __stdcall GetFrequency() const
  {
    return mnFreq;
  }

  ///////////////////////////////////////////////
  //! Read a chunk of the specified size.
  tI32 __stdcall ReadRaw(tPtr apOut, tU32 anSize)
  {
    if (!mptrData.IsOK()) return 0;
    return mptrData->ReadRaw(apOut, anSize);
  }

  ///////////////////////////////////////////////
  //! Reset data stream to the begining.
  void __stdcall Reset()
  {
    if (mptrData.IsOK())
      mptrData->SeekSet(0);
  }

  ///////////////////////////////////////////////
  iSoundData* __stdcall Clone() const
  {
#pragma niTodo("Should reopen a file handle and not use auto seek...")
    if (!mptrData.IsOK()) return NULL;
    Ptr<iFile> ptrFile = ni::CreateFileWindow(mptrData->GetFileBase(),0,0,NULL,eTrue);
    return niNew cSoundDataWAV(ptrFile,mFormat,mnFreq);
  }

  ///////////////////////////////////////////////
  tU32 __stdcall GetLength() const {
    return mptrData->GetSize() / SoundFormatGetFrameSize(mFormat);
  }

  niEndClass(cSoundDataWAV);
};

//////////////////////////////////////////////////////////////////////////////////////////////
// WAV loader

class cSoundDataLoaderWAV : public ImplRC<iSoundDataLoader>
{
  iSoundData* __stdcall LoadSoundData(iFile* apFile) {
    return niNew cSoundDataWAV(apFile);
  }
};

iSoundDataLoader* New_SoundDataLoaderWAV() {
  return niNew cSoundDataLoaderWAV();
}
