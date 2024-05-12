// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "stdafx.h"

#define MAX_FREQUENCY 441000 // allows 10 times the speed of a 44100 sample...

class cSoundMixerSoftware : public ImplRC<iSoundMixer,eImplFlags_Default,iSoundDriverBufferDataSink>
{
  niBeginClass(cSoundMixerSoftware);

 public:
  cSoundMixerSoftware() {
    ZeroMembers();
  }
  ~cSoundMixerSoftware() {
  }

  void ZeroMembers() {
    mnNumChannels = 0;
    chan = NULL;
    memset(&settings,0,sizeof(settings));
  }

  tBool __stdcall IsOK() const {
    niClassIsOK(cSoundMixerSoftware);
    return eTrue;
  }

  tBool __stdcall Initialize(eSoundFormat aFormat, tU32 anFreq, iSoundDriverBuffer* apBuffer, tU32 anNumChannels);
  void __stdcall Close(void);

  tBool __stdcall SwitchIn() {
    return eTrue;
  }
  tBool __stdcall SwitchOut() {
    return eTrue;
  }

  tBool __stdcall UpdateMixer() {
    return eTrue;
  }

  ///////////////////////////////////////////////
  void __stdcall SetMasterVolume(tI32 vol) {
#pragma niTodo("IMPLEMENT")
  }

  ///////////////////////////////////////////////
  tI32 __stdcall GetMasterVolume() const {
#pragma niTodo("IMPLEMENT")
    return 255;
  }

  tU32 __stdcall GetNumChannels() const {
    return mnNumChannels;
  }

  tBool __stdcall StartChannel(tU32 nchan);
  tBool __stdcall StopChannel(tU32 nchan);
  void __stdcall StopAllChannels(void);

  tBool __stdcall SetChannelBuffer(tU32 anChan, iSoundBuffer* apBuffer);
  iSoundBuffer* __stdcall GetChannelBuffer(tU32 anChan) const;

  void __stdcall SetChannelMasterVolume(tU32 nchan, tI32 vol);
  tI32  __stdcall GetChannelMasterVolume(tU32 nchan) const;

  void __stdcall SetChannelMasterPan(tU32 nchan, tI32 pan);
  tI32  __stdcall GetChannelMasterPan(tU32 nchan) const;

  void __stdcall SetChannelVolume(tU32 nchan, tI32 vol);
  tI32  __stdcall GetChannelVolume(tU32 nchan) const;

  void __stdcall SetChannelPan(tU32 nchan, tI32 pan);
  tI32  __stdcall GetChannelPan(tU32 nchan) const;

  void __stdcall SetChannelFrequency(tU32 nchan, tI32 frequency);
  tI32  __stdcall GetChannelFrequency(tU32 nchan) const;

  void __stdcall SetChannelLoop(tU32 nchan, tBool loop);
  tBool __stdcall GetChannelLoop(tU32 nchan) const;

  void __stdcall SetChannelPause(tU32 nchan, tBool pause);
  tBool __stdcall GetChannelPause(tU32 nchan) const;

  tBool __stdcall GetIsChannelBusy(tU32 nchan) const;

  void __stdcall SetChannelUserID(tU32 nchan, tIntPtr anID);
  tIntPtr __stdcall GetChannelUserID(tU32 nchan) const;

  void __stdcall SetAmplification(tI32 ampli);
  tI32  __stdcall GetAmplification(void) const;

  void  __stdcall SetSaturationCheck(tBool check);
  tBool __stdcall GetSaturationCheck(void) const;

  void  __stdcall SetReverseStereo(tBool reverse);
  tBool __stdcall GetReverseStereo(void) const;

  void __stdcall OnSoundDriverBufferDataSink(void* ptr, tU32 size);

 private:
  void MixChannels(void);
  bool MixDecode16BitsStereo(void *pnt, tU32 size);
  bool MixDecode16BitsMono(void *pnt, tU32 size);
  bool MixDecode8BitsStereo(void *pnt, tU32 size);
  bool MixDecode8BitsMono(void *pnt, tU32 size);
  void MixComputeSpeed(tU32 nchan);
  void MixComputeVolume(tU32 nchan);

  tU32 mnNumChannels;

  // Array contains the states of each channels
  struct INTERNALCHANNEL
  {
    tU8 volume;
    tU8 pan;
    tI32 frequency;
    tU8 samplevolume;
    tU8 samplepan;
    tBool stereo;
    tBool loop;
    tBool pause;
    tI32 bits;
    Ptr<iSoundBuffer> buffer;
    tU32 resvol;
    tU32 resvoll, resvolr;
    tBool used;
    tBool cont;
    tU32 speed;
    tIntPtr user;
  } *chan;

  /* Settings utilise pour le mixage */
  struct MIXSETTINGS
  {
    Ptr<iSoundDriverBuffer> buffer;
    eSoundFormat            format;
    tBool     stereo;
    tBool     check;
    tU8       bits;
    tU32      frequency;
    tI32      ampli;
    tPtr      transfert;
    tI32*     mixl;
    tI32*     mixr;
    tI32      nsamples;
    tBool     reversestereo;
  } settings;

  niEndClass(cSoundMixerSoftware);
};

#define MYFIXED 16
#define NFIXED  (1<<MYFIXED)

//--------------------------------------------------------------------------------------------
//
//  Fonctions internes
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
// Effectue le mixage des pistes
void cSoundMixerSoftware::MixChannels(void)
{
  tI32 data,datal,datar;
  tI64 ofs;

  memset(settings.mixl, 0, settings.nsamples*4);
  memset(settings.mixr, 0, settings.nsamples*4);

  niLoop(i,mnNumChannels)
  {
    if (chan[i].used && !chan[i].pause)
    {
      ofs = 0;
      // channel contains 16bit audio
      if(chan[i].bits == 16)
      {
        if(chan[i].stereo)
        {
          chan[i].used = chan[i].buffer->ReadRaw(settings.transfert,
                                                 ((settings.nsamples*
                                                   chan[i].frequency+
                                                   (settings.frequency-1))/
                                                  settings.frequency)*2*2,
                                                 chan[i].loop);

          niLoop(j,settings.nsamples)
          {
            datal = ((tI16*)settings.transfert)[2*(ofs>>MYFIXED)];
            datar = ((tI16*)settings.transfert)[2*(ofs>>MYFIXED)+1];
            settings.mixl[j] += datal*chan[i].resvoll;
            settings.mixr[j] += datar*chan[i].resvolr;
            ofs += chan[i].speed;
          }
        }
        else
        {
          chan[i].used = chan[i].buffer->ReadRaw(settings.transfert,
                                                 (settings.nsamples*chan[i].frequency/
                                                  settings.frequency+1) * 2 * 1,
                                                 chan[i].loop);
          niLoop(j,settings.nsamples)
          {
            data = ((tI16*)settings.transfert)[ofs>>MYFIXED];
            settings.mixl[j] += data*chan[i].resvoll;
            settings.mixr[j] += data*chan[i].resvolr;
            ofs += chan[i].speed;
          }
        }
      }
      // channel contains 8 bits audio
      else
      {
        if(chan[i].stereo)
        {
          chan[i].used = chan[i].buffer->ReadRaw(settings.transfert,
                                                 (settings.nsamples*chan[i].frequency/
                                                  settings.frequency+1) * 1 * 2,
                                                 chan[i].loop);
          niLoop(j,settings.nsamples)
          {
            datal = ((tU8*)settings.transfert)[2*(ofs>>MYFIXED)]-128;
            datar = ((tU8*)settings.transfert)[2*(ofs>>MYFIXED)+1]-128;
            settings.mixl[j] += datal*chan[i].resvoll;
            settings.mixr[j] += datar*chan[i].resvolr;
            ofs += chan[i].speed;
          }
        }
        else
        {
          chan[i].used = chan[i].buffer->ReadRaw(settings.transfert,
                                                 (settings.nsamples*chan[i].frequency/
                                                  settings.frequency+1) * 1 * 1,
                                                 chan[i].loop);
          niLoop(j,settings.nsamples)
          {
            data = ((tU8*)settings.transfert)[ofs>>MYFIXED]-128;
            settings.mixl[j] += data*chan[i].resvoll;
            settings.mixr[j] += data*chan[i].resvolr;
            ofs += chan[i].speed;
          }
        }
      }
    }
  }
}

///////////////////////////////////////////////
// Mixe un son 16 bits signe stereo
bool cSoundMixerSoftware::MixDecode16BitsStereo(void *pnt, tU32 size)
{
  tI32 j;
  tU32 adr=0;
  tI32 resl,resr;
  MixChannels();

  /* Normalisation et transfert vers la carte */
  if(settings.check)
  {
    for(j = 0; j < settings.nsamples; j++)
    {
      resl = (settings.mixl[j]>>(16-settings.ampli));
      resr = (settings.mixr[j]>>(16-settings.ampli));
      if(resl < -32768)
        resl = -32768;
      if(resr < -32768)
        resr = -32768;
      if(resl > 32767)
        resl = 32767;
      if(resr > 32767)
        resr = 32767;
      ((tI16*)pnt)[adr++] = (tI16)resl;
      ((tI16*)pnt)[adr++] = (tI16)resr;
    }
  }
  else
  {
    for(j = 0; j < settings.nsamples; j++)
    {
      resl = (settings.mixl[j]>>(16-settings.ampli));
      resr = (settings.mixr[j]>>(16-settings.ampli));
      ((tI16*)pnt)[adr++] = (tI16)resl;
      ((tI16*)pnt)[adr++] = (tI16)resr;
    }
  }

  return true;
}

///////////////////////////////////////////////
// Mixe un son 16 bit signe mono
bool cSoundMixerSoftware::MixDecode16BitsMono(void *pnt, tU32 size)
{
  tI32 j;
  tU32 adr = 0;
  tI32 resl, resr;

  MixChannels();

  // Normalisation et transfert vers la carte
  if(settings.check)
  {
    for(j = 0; j < settings.nsamples; j++)
    {
      resl = (settings.mixl[j]>>(16-settings.ampli));
      resr = (settings.mixr[j]>>(16-settings.ampli));
      if(resl < -32768)
        resl = -32768;
      if(resr < -32768)
        resr = -32768;
      if(resl > 32767)
        resl = 32767;
      if(resr > 32767)
        resr = 32767;
      ((tI16*)pnt)[adr++] = (tI16)(resl+resr)>>1;
    }
  }
  else
  {
    for(j = 0; j < settings.nsamples; j++)
    {
      resl = (settings.mixl[j]>>(16-settings.ampli));
      resr = (settings.mixr[j]>>(16-settings.ampli));
      ((tI16*)pnt)[adr++] = (tI16)(resl+resr)>>1;
    }
  }

  return true;
}

///////////////////////////////////////////////
// Mixe un son 8 bits non-signe stereo
bool cSoundMixerSoftware::MixDecode8BitsStereo(void *pnt, tU32 size)
{
  tI32 j;
  tU32 adr = 0;
  tI32 resl, resr;

  MixChannels();

  // Normalisation et transfert vers la carte
  if(settings.check)
  {
    for(j = 0; j < settings.nsamples; j++)
    {
      resl = (settings.mixl[j]>>(24-settings.ampli))+128;
      resr = (settings.mixr[j]>>(24-settings.ampli))+128;
      if(resl < 0)
        resl = 0;
      if(resr < 0)
        resr = 0;
      if(resl > 255)
        resl = 255;
      if(resr > 255)
        resr = 255;
      ((char*)pnt)[adr++] = (char)resl;
      ((char*)pnt)[adr++] = (char)resr;
    }
  }
  else
  {
    for(j = 0; j < settings.nsamples; j++)
    {
      resl = (settings.mixl[j]>>(24-settings.ampli))+128;
      resr = (settings.mixr[j]>>(24-settings.ampli))+128;
      ((char*)pnt)[adr++] = (char)resl;
      ((char*)pnt)[adr++] = (char)resr;
    }
  }

  return true;
}

///////////////////////////////////////////////
// Mixe un son 8 bits non-signe mono
bool cSoundMixerSoftware::MixDecode8BitsMono(void *pnt, tU32 size)
{
  tI32 j;
  tU32 adr = 0;
  tI32 resl, resr;

  MixChannels();

  // Normalisation et transfert vers la carte
  if(settings.check)
  {
    for(j = 0; j < settings.nsamples; j++)
    {
      resl = (settings.mixl[j]>>(24-settings.ampli))+128;
      resr = (settings.mixr[j]>>(24-settings.ampli))+128;
      if(resl < 0)
        resl = 0;
      if(resr < 0)
        resr = 0;
      if(resl > 255)
        resl = 255;
      if(resr > 255)
        resr = 255;
      ((char*)pnt)[adr++] = (char)(resl+resr)>>1;
    }
  }
  else
  {
    for(j = 0; j < settings.nsamples; j++)
    {
      resl = (settings.mixl[j]>>(24-settings.ampli))+128;
      resr = (settings.mixr[j]>>(24-settings.ampli))+128;
      ((char*)pnt)[adr++] = (char)(resl+resr)>>1;
    }
  }

  return true;
}

///////////////////////////////////////////////
// Calcule la vitesse de mixage
void cSoundMixerSoftware::MixComputeSpeed(tU32 nchan)
{
  chan[nchan].speed = (tI32)(NFIXED*(float)chan[nchan].frequency/(float)settings.frequency);
}

///////////////////////////////////////////////
// Calcule le volume d'une piste
void cSoundMixerSoftware::MixComputeVolume(tU32 nchan)
{
  chan[nchan].resvol = chan[nchan].samplevolume*chan[nchan].volume;
  if(settings.reversestereo)
  {
    chan[nchan].resvolr = (((256-chan[nchan].pan)+(256-chan[nchan].samplepan))*chan[nchan].resvol);
    chan[nchan].resvoll = ((chan[nchan].pan+chan[nchan].samplepan)*chan[nchan].resvol);
  }
  else
  {
    chan[nchan].resvoll = (((256-chan[nchan].pan)+(256-chan[nchan].samplepan))*chan[nchan].resvol);
    chan[nchan].resvolr = ((chan[nchan].pan+chan[nchan].samplepan)*chan[nchan].resvol);
  }

  if(chan[nchan].bits == 16) {
    chan[nchan].resvoll >>= 15;
    chan[nchan].resvolr >>= 15;
  }
  else {
    chan[nchan].resvoll >>= 7;
    chan[nchan].resvolr >>= 7;
  }
}

//--------------------------------------------------------------------------------------------
//
//  FONCTIONS DE L'INTERFACE
//
//--------------------------------------------------------------------------------------------

///////////////////////////////////////////////
// Initialise le mixer son
tBool cSoundMixerSoftware::Initialize(eSoundFormat aFormat, tU32 anFreq, iSoundDriverBuffer* apBuffer, tU32 anNumChannels)
{
  Close();

  tBool err;

  if (aFormat == eSoundFormat_Unknown) {
    aFormat = eSoundFormat_Stereo16;
  }

  settings.format = aFormat;
  settings.buffer = apBuffer;
  settings.frequency = anFreq;
  switch (settings.format) {
    case eSoundFormat_Mono8:
      settings.bits = 8;
      settings.stereo = false;
      break;
    case eSoundFormat_Mono16:
      settings.bits = 16;
      settings.stereo = false;
      break;
    case eSoundFormat_Stereo8:
      settings.bits = 8;
      settings.stereo = true;
      break;
    case eSoundFormat_Stereo16:
      settings.bits = 16;
      settings.stereo = true;
      break;
    default:
      niError(niFmt(_A("Invalid format for the mixer, %d bits, %d channels."),SoundFormatGetBits(aFormat),SoundFormatGetChannels(aFormat)));
      return eFalse;
  }

  settings.ampli = 6;
  settings.reversestereo = false;
  settings.check = true;

  // Initialisation des valeurs par defaut
  mnNumChannels = anNumChannels;
  chan = niTMalloc(INTERNALCHANNEL,mnNumChannels);
  memset(chan,0,sizeof(INTERNALCHANNEL)*mnNumChannels);
  niLoop(i,mnNumChannels) {
    chan[i].used = false;
    chan[i].volume = 255;
    chan[i].pan = 128;
    chan[i].samplevolume = 255;
    chan[i].samplepan = 128;
  }

  // On commence a jouer le son
  err = apBuffer->Play(aFormat,anFreq);

  // Initialisation de divers variables de mixage
  settings.nsamples = apBuffer->GetSize();

  if (settings.stereo)
    settings.nsamples >>= 1;

  if (settings.bits == 16)
    settings.nsamples >>= 1;

  const tU32 transfertBufferSize = 4*apBuffer->GetSize()*(MAX_FREQUENCY/settings.frequency)+4;
  // niDebugFmt(("... transfertBufferSize: %d",transfertBufferSize));
  settings.transfert = (tPtr)niMalloc(transfertBufferSize);
  settings.mixl = (tI32*)niMalloc(4*settings.nsamples);
  settings.mixr = (tI32*)niMalloc(4*settings.nsamples);

  return err;
}

///////////////////////////////////////////////
// Ferme le mixer son
void cSoundMixerSoftware::Close(void)
{
  StopAllChannels();
  if (settings.buffer.IsOK()) {
    settings.buffer->Stop();
  }
  niSafeFree(settings.transfert);
  niSafeFree(settings.mixl);
  niSafeFree(settings.mixr);
  niSafeFree(chan);
}

///////////////////////////////////////////////
// Joue une sample sur la piste indiquee
tBool cSoundMixerSoftware::StartChannel(tU32 nchan)
{
  if (!chan[nchan].buffer.IsOK())
    return eFalse;

  //niPrintln(niFmt(_A("StartChannel: %d\n"),nchan));

  MixComputeSpeed(nchan);
  MixComputeVolume(nchan);

  chan[nchan].used = eTrue;
  return eTrue;
}

///////////////////////////////////////////////
// Arrete de jouer sur la piste indiquee
tBool cSoundMixerSoftware::StopChannel(tU32 nchan)
{
  //niPrintln(niFmt(_A("StopChannel: %d\n"),nchan));
  chan[nchan].used = eFalse;
  chan[nchan].user = 0;
  return eTrue;
}

///////////////////////////////////////////////
// Arrete de jouer sur toute les pistes
void cSoundMixerSoftware::StopAllChannels(void)
{
  niLoop(i,mnNumChannels) {
    StopChannel(i);
  }
}

///////////////////////////////////////////////
tBool __stdcall cSoundMixerSoftware::SetChannelBuffer(tU32 nchan, iSoundBuffer* apBuffer)
{
  chan[nchan].buffer = NULL;
  niCheckSilent(apBuffer!=NULL,eFalse);
  switch (apBuffer->GetFormat()) {
    case eSoundFormat_Mono8:
      chan[nchan].stereo = eFalse;
      chan[nchan].bits = 8;
      break;
    case eSoundFormat_Mono16:
      chan[nchan].stereo = eFalse;
      chan[nchan].bits = 16;
      break;
    case eSoundFormat_Stereo16:
      chan[nchan].stereo = eTrue;
      chan[nchan].bits = 16;
      break;
    case eSoundFormat_Stereo8:
      chan[nchan].stereo = eFalse;
      chan[nchan].bits = 8;
      break;
    default:
      return eFalse;
  }

  chan[nchan].frequency = apBuffer->GetFrequency();
  chan[nchan].buffer = apBuffer;
  return eTrue;
}

///////////////////////////////////////////////
iSoundBuffer* __stdcall cSoundMixerSoftware::GetChannelBuffer(tU32 anChan) const
{
  return chan[anChan].buffer;
}

///////////////////////////////////////////////
void cSoundMixerSoftware::SetChannelMasterVolume(tU32 nchan, tI32 vol)
{
  chan[nchan].volume = (tU8)ni::Clamp<tI32>(vol,0,255);
  MixComputeVolume(nchan);
}

///////////////////////////////////////////////
tI32 cSoundMixerSoftware::GetChannelMasterVolume(tU32 nchan) const
{
  return chan[nchan].volume;
}

///////////////////////////////////////////////
void cSoundMixerSoftware::SetChannelMasterPan(tU32 nchan, tI32 pan)
{
  chan[nchan].pan = (tU8)ni::Clamp<tI32>(0,255,pan);
  MixComputeVolume(nchan);
}

///////////////////////////////////////////////
tI32 cSoundMixerSoftware::GetChannelMasterPan(tU32 nchan) const
{
  return chan[nchan].pan;
}

///////////////////////////////////////////////
void cSoundMixerSoftware::SetChannelVolume(tU32 nchan, tI32 vol)
{
  chan[nchan].volume = (tU8)ni::Clamp<tI32>(vol,0,255);
  MixComputeVolume(nchan);
}

///////////////////////////////////////////////
tI32 cSoundMixerSoftware::GetChannelVolume(tU32 nchan) const
{
  return chan[nchan].volume;
}

///////////////////////////////////////////////
void cSoundMixerSoftware::SetChannelPan(tU32 nchan, tI32 pan)
{
  chan[nchan].pan = (tU8)ni::Clamp<tI32>(pan,0,255);
  MixComputeVolume(nchan);
}

///////////////////////////////////////////////
tI32 cSoundMixerSoftware::GetChannelPan(tU32 nchan) const
{
  return chan[nchan].pan;
}

///////////////////////////////////////////////
// Change la frequence d'une piste
void cSoundMixerSoftware::SetChannelFrequency(tU32 nchan, tI32 frequency)
{
  chan[nchan].frequency = ni::Clamp<tI32>(frequency,0,MAX_FREQUENCY);
  MixComputeSpeed(nchan);
}

///////////////////////////////////////////////
// Retourne la frequence d'une piste
tI32 cSoundMixerSoftware::GetChannelFrequency(tU32 nchan) const
{
  return chan[nchan].frequency;
}

///////////////////////////////////////////////
void __stdcall cSoundMixerSoftware::SetChannelLoop(tU32 nchan, tBool loop)
{
  chan[nchan].loop = loop;
}

///////////////////////////////////////////////
tBool __stdcall cSoundMixerSoftware::GetChannelLoop(tU32 nchan) const
{
  return chan[nchan].loop;
}

///////////////////////////////////////////////
void __stdcall cSoundMixerSoftware::SetChannelPause(tU32 nchan, tBool pause) {
  chan[nchan].pause = pause;
}

///////////////////////////////////////////////
tBool __stdcall cSoundMixerSoftware::GetChannelPause(tU32 nchan) const {
  return chan[nchan].pause;
}


///////////////////////////////////////////////
tBool cSoundMixerSoftware::GetIsChannelBusy(tU32 nchan) const
{
  return chan[nchan].used;
}

///////////////////////////////////////////////
void __stdcall cSoundMixerSoftware::SetChannelUserID(tU32 nchan, tIntPtr anID)
{
  chan[nchan].user = anID;
}

///////////////////////////////////////////////
tIntPtr __stdcall cSoundMixerSoftware::GetChannelUserID(tU32 nchan) const
{
  return chan[nchan].user;
}

///////////////////////////////////////////////
// Change la valeur d'amplification
void cSoundMixerSoftware::SetAmplification(tI32 ampli)
{
  if(ampli > 16)
    ampli = 16;

  settings.ampli = ampli;
}

///////////////////////////////////////////////
// Retourne la valeur d'amplification
tI32 cSoundMixerSoftware::GetAmplification(void) const
{
  return settings.ampli;
}

///////////////////////////////////////////////
// Branche ou coupe la verification de saturation au mixage
void cSoundMixerSoftware::SetSaturationCheck(tBool check)
{
  settings.check = check;
}

///////////////////////////////////////////////
// Retourne true si la verification de saturation au mixage est activee
tBool cSoundMixerSoftware::GetSaturationCheck(void) const
{
  return settings.check;
}

///////////////////////////////////////////////
// Branche ou coupe l'inversion stereo
void cSoundMixerSoftware::SetReverseStereo(tBool reverse)
{
  settings.reversestereo = reverse;
  niLoop(i,mnNumChannels)
      MixComputeVolume(i);
}

///////////////////////////////////////////////
// Retourne l'etat de l'inversion stereo
tBool cSoundMixerSoftware::GetReverseStereo(void) const
{
  return settings.reversestereo;
}

///////////////////////////////////////////////
void __stdcall cSoundMixerSoftware::OnSoundDriverBufferDataSink(void* ptr, tU32 size) {
  switch (settings.format) {
    case eSoundFormat_Mono8:
      MixDecode8BitsMono(ptr,size);
      break;
    case eSoundFormat_Mono16:
      MixDecode16BitsMono(ptr,size);
      break;
    case eSoundFormat_Stereo8:
      MixDecode8BitsStereo(ptr,size);
      break;
    case eSoundFormat_Stereo16:
      MixDecode16BitsStereo(ptr,size);
      break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
iSoundMixer* __stdcall New_SoundMixerSoftware(eSoundFormat aFormat, tU32 anFreq, iSoundDriverBuffer* apBuffer, tU32 anNumChannels) {
  Ptr<cSoundMixerSoftware> ptrSM = niNew cSoundMixerSoftware();
  if (!ptrSM->Initialize(aFormat,anFreq,apBuffer,anNumChannels)) {
    niError(_A("Can't create the software sound mixer."));
    return NULL;
  }
  return ptrSM.GetRawAndSetNull();
}
