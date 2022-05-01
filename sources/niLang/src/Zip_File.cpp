// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Zip_File.h"

#ifndef niEmbedded

#include "zlib/zlib.h"

using namespace ni;

const int ZBUF_SIZE = 4096;

struct inflater_impl
{
  iFileBase*  m_in;
  z_stream  m_zstream;
  tI64      m_initial_stream_pos; // position of the input stream where we started inflating.
  tI64      m_logical_stream_pos; // current stream position of uncompressed data.
  bool      m_at_eof;

  unsigned char m_rawdata[ZBUF_SIZE];

  int m_error;


  inflater_impl(iFileBase* in)
      // Constructor.
      :
      m_in(in),
      m_initial_stream_pos(in->Tell()),
      m_logical_stream_pos(0),
      m_at_eof(false),
      m_error(0)
  {
    niAssert(m_in);

    m_zstream.zalloc = (zlib_alloc_func)0;
    m_zstream.zfree = (zlib_free_func)0;
    m_zstream.opaque = (voidpf)0;

    m_zstream.next_in  = 0;
    m_zstream.avail_in = 0;

    m_zstream.next_out = 0;
    m_zstream.avail_out = 0;

    int err = inflateInit(&m_zstream);
    if (err != Z_OK) {
      //log_error("error: inflater_impl::ctor() inflateInit() returned %d\n", err);
      m_error = 1;
      return;
    }

    // Ready to go!
  }


  void  reset()
      // Discard current results and rewind to the beginning.
      // Necessary in order to seek backwards.
  {
    m_error = 0;
    m_at_eof = 0;
    int err = inflateReset(&m_zstream);
    if (err != Z_OK) {
      m_error = 1;
      return;
    }

    m_zstream.next_in = 0;
    m_zstream.avail_in = 0;

    m_zstream.next_out = 0;
    m_zstream.avail_out = 0;

    // Rewind the underlying stream.
    m_in->SeekSet(m_initial_stream_pos);

    m_logical_stream_pos = 0;
  }


  tI32 inflate_from_stream(void* dst, tI32 bytes)
  {
    if (m_error) {
      return 0;
    }

    m_zstream.next_out = (unsigned char*) dst;
    m_zstream.avail_out = bytes;

    for (;;)
    {
      if (m_zstream.avail_in == 0)
      {
        // Get more raw data.
        int new_bytes = m_in->ReadRaw(m_rawdata, ZBUF_SIZE);
        if (new_bytes == 0)
        {
          // The cupboard is bare!  We have nothing to feed to inflate().
          break;
        }
        else
        {
          m_zstream.next_in = m_rawdata;
          m_zstream.avail_in = new_bytes;
        }
      }

      int err = inflate(&m_zstream, Z_SYNC_FLUSH);
      if (err == Z_STREAM_END)
      {
        m_at_eof = true;
        break;
      }
      if (err != Z_OK)
      {
        // something's wrong.
        m_error = 1;
        break;
      }

      if (m_zstream.avail_out == 0)
      {
        break;
      }
    }

    int bytes_read = bytes - m_zstream.avail_out;
    m_logical_stream_pos += bytes_read;

    return bytes_read;
  }

  void  rewind_unused_bytes()
      // If we have unused bytes in our input buffer, rewind
      // to before they started.
  {
    if (m_zstream.avail_in > 0)
    {
      tI64  pos = m_in->Tell();
      tI64  rewound_pos = pos - m_zstream.avail_in;
      niAssert(pos >= 0);
      niAssert(pos >= m_initial_stream_pos);
      niAssert(rewound_pos >= 0);
      niAssert(rewound_pos >= m_initial_stream_pos);

      m_in->SeekSet(rewound_pos);
    }
  }
};


static int  inflate_read(void* dst, int bytes, void* appdata)
    // Return number of bytes actually read.
{
  inflater_impl*  inf = (inflater_impl*) appdata;
  if (inf->m_error)
  {
    return 0;
  }

  return inf->inflate_from_stream(dst, bytes);
}

static tI64 inflate_seek(tI64 pos, void* appdata)
    // Try to go to pos.  Return actual pos.
{
  inflater_impl*  inf = (inflater_impl*) appdata;
  if (inf->m_error) {
    return inf->m_logical_stream_pos;
  }

  // If we're seeking backwards, then restart from the beginning.
  if (pos < inf->m_logical_stream_pos)
  {
    inf->reset();
  }

  unsigned char temp[ZBUF_SIZE];

  // Now seek forwards, by just reading data in blocks.
  while (inf->m_logical_stream_pos < pos)
  {
    tI64 to_read = pos - inf->m_logical_stream_pos;
    tI32 to_read_this_time = (tI32)ni::Min(to_read, ZBUF_SIZE);
    niAssert(to_read_this_time > 0);

    int bytes_read = inf->inflate_from_stream(temp, to_read_this_time);
    niAssert(bytes_read <= to_read_this_time);
    if (bytes_read == 0) {
      // Trouble; can't seek any further.
      break;
    }
  }

  niAssert(inf->m_logical_stream_pos <= pos);

  return inf->m_logical_stream_pos;
}


static tI64 inflate_seek_to_end(void* appdata)
{
  inflater_impl*  inf = (inflater_impl*) appdata;
  if (inf->m_error) {
    return inf->m_logical_stream_pos;
  }

  // Keep reading until we can't read any more.

  unsigned char temp[ZBUF_SIZE];

  // Seek forwards.
  for (;;)
  {
    int bytes_read = inf->inflate_from_stream(temp, ZBUF_SIZE);
    if (bytes_read == 0)
    {
      // We've seeked as far as we can.
      break;
    }
  }

  return inf->m_logical_stream_pos;
}

static tI64 inflate_tell(const void* appdata)
{
  inflater_impl*  inf = (inflater_impl*) appdata;
  return inf->m_logical_stream_pos;
}

// static tBool inflate_get_eof(void* appdata)
// {
//  inflater_impl*  inf = (inflater_impl*) appdata;
//  return inf->m_at_eof;
// }

static tBool inflate_close(void* appdata)
{
  inflater_impl*  inf = (inflater_impl*) appdata;

  inf->rewind_unused_bytes();
  int err = inflateEnd(&(inf->m_zstream));

  delete inf;

  return (err == Z_OK);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// File interface implentation

///////////////////////////////////////////////
cZipFile::cZipFile(iFileBase* apFile, tU32 aulCompressionMode)
{
  ZeroMembers();

  if (niFlagIsNot(apFile->GetFileFlags(),eFileFlags_Read)) {
    niError(_A("Invalid file base, zip inflater is read only."));
    return;
  }

  mpInflater = new inflater_impl(apFile);
  if (!mpInflater || mpInflater->m_error) {
    niError(_A("Can't open Zip inflater."));
    return;
  }

  mptrFile = apFile;
}

///////////////////////////////////////////////
cZipFile::~cZipFile()
{
  if (mpInflater) {
    inflate_close(mpInflater);
    mpInflater = NULL;
  }
  mptrFile = NULL;
}

///////////////////////////////////////////////
void cZipFile::ZeroMembers()
{
  mnSize = 0;
  mpInflater = NULL;
}

///////////////////////////////////////////////
tBool cZipFile::IsOK() const
{
  return (mptrFile.IsOK());
}

///////////////////////////////////////////////
tBool cZipFile::Seek(tI64 offset)
{
  inflate_seek(inflate_tell(mpInflater)+offset,mpInflater);
  return mpInflater->m_error != 1;
}

///////////////////////////////////////////////
tBool cZipFile::SeekSet(tI64 offset)
{
  inflate_seek(offset,mpInflater);
  return mpInflater->m_error != 1;
}

///////////////////////////////////////////////
tSize cZipFile::ReadRaw(void *pOut, tSize nSize)
{
  return inflate_read(pOut,nSize,mpInflater);
}

///////////////////////////////////////////////
tSize cZipFile::WriteRaw(const void *pOut, tSize nSize)
{
  return 0;
}

///////////////////////////////////////////////
tI64 cZipFile::Tell()
{
  return inflate_tell(mpInflater);
}

///////////////////////////////////////////////
tI64 cZipFile::GetSize() const
{
  if (!mnSize) {
    tI64 nPos = niThis(cZipFile)->Tell();
    inflate_seek_to_end(mpInflater);
    niThis(cZipFile)->mnSize = niThis(cZipFile)->Tell();
    niThis(cZipFile)->SeekSet((tSize)nPos);
  }
  return mnSize;
}

///////////////////////////////////////////////
const achar* cZipFile::GetSourcePath() const
{
  return mptrFile->GetSourcePath();
}

#endif // niEmbedded
