// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "JpegReader.h"

// Disables "interaction between '_setjmp' and C++ object destruction is non-portable"
#ifdef _MSC_VER
#pragma warning (disable : 4611)
#endif

niExportFunc(iUnknown*) New_niUI_JpegReader(const Var& avarA, const Var&) {
  iFile* pFile = ni::VarQueryInterface<iFile>(avarA);
  if (!niIsOK(pFile)) {
    niError(_A("No valid file passed in parameter A."));
    return NULL;
  }
  return niNew cJpegReader(pFile);
}

static const int    IO_BUF_SIZE = 4096;

// A jpeglib source manager that reads from a ni::iFile.  Paraphrased
// from IJG jpeglib jdatasrc.c.
struct rw_source
{
  struct jpeg_source_mgr  m_pub;      /* public fields */

  ni::iFile* m_in_stream;        /* source stream */
  bool    m_start_of_file;        /* have we gotten any data yet? */
  JOCTET  m_buffer[IO_BUF_SIZE];  /* start of buffer */

  rw_source(ni::iFile* in)
      :
      m_in_stream(in),
      m_start_of_file(true)
      // Constructor.  The caller is responsible for closing the input stream
      // after it's done using us.
  {
    // fill in function pointers...
    m_pub.init_source = init_source;
    m_pub.fill_input_buffer = fill_input_buffer;
    m_pub.skip_input_data = skip_input_data;
    m_pub.resync_to_restart = jpeg_resync_to_restart;   // use default method
    m_pub.term_source = term_source;
    m_pub.bytes_in_buffer = 0;
    m_pub.next_input_byte = NULL;
  }

  static void init_source(j_decompress_ptr cinfo)
  {
    rw_source*  src = (rw_source*) cinfo->src;
    src->m_start_of_file = true;
  }

  static boolean  fill_input_buffer(j_decompress_ptr cinfo)
      // Read data into our input buffer.  Client calls this
      // when it needs more data from the file.
  {
    rw_source*  src = (rw_source*) cinfo->src;

    size_t  bytes_read = src->m_in_stream->ReadRaw(src->m_buffer, IO_BUF_SIZE);

    if (bytes_read <= 0) {
      // Is the file completely empty?
      if (src->m_start_of_file) {
        // Treat this as a fatal error.
        //              throw "empty jpeg source stream.";
        return FALSE;
      }
      // warn("jpeg end-of-stream");

      // Insert a fake EOI marker.
      src->m_buffer[0] = (JOCTET) 0xFF;
      src->m_buffer[1] = (JOCTET) JPEG_EOI;
      bytes_read = 2;
    }

    // Hack to work around SWF bug: sometimes data
    // starts with FFD9FFD8, when it should be
    // FFD8FFD9!
    if (src->m_start_of_file && bytes_read >= 4)
    {
      if (src->m_buffer[0] == 0xFF
          && src->m_buffer[1] == 0xD9
          && src->m_buffer[2] == 0xFF
          && src->m_buffer[3] == 0xD8)
      {
        src->m_buffer[1] = 0xD8;
        src->m_buffer[3] = 0xD9;
      }
    }

    // Expose buffer state to clients.
    src->m_pub.next_input_byte = src->m_buffer;
    src->m_pub.bytes_in_buffer = bytes_read;
    src->m_start_of_file = false;

    return TRUE;
  }

  static void skip_input_data(j_decompress_ptr cinfo, long num_bytes)
      // Called by client when it wants to advance past some
      // uninteresting data.
  {
    rw_source*  src = (rw_source*) cinfo->src;

    // According to jpeg docs, large skips are
    // infrequent.  So let's just do it the simple
    // way.
    if (num_bytes > 0) {
      while (num_bytes > (long) src->m_pub.bytes_in_buffer) {
        num_bytes -= (long) src->m_pub.bytes_in_buffer;
        fill_input_buffer(cinfo);
      }
      // Handle remainder.
      src->m_pub.next_input_byte += (size_t) num_bytes;
      src->m_pub.bytes_in_buffer -= (size_t) num_bytes;
    }
  }

  static void term_source(j_decompress_ptr cinfo)
      // Terminate the source.  Make sure we get deleted.
  {
    /*rw_source*    src = (rw_source*) cinfo->src;
      assert(src);

      // @@ it's kind of bogus to be deleting here
      // -- term_source happens at the end of
      // reading an image, but we're probably going
      // to want to init a source and use it to read
      // many images, without reallocating our
      // buffer.
      delete src;
      cinfo->src = NULL;*/
  }


  void    discard_partial_buffer()
  {
    // Discard existing bytes in our buffer.
    m_pub.bytes_in_buffer = 0;
    m_pub.next_input_byte = NULL;
  }
};


static void setup_rw_source(jpeg_decompress_struct* cinfo, ni::iFile* instream)
    // Set up the given decompress object to read from the given
    // stream.
{
  // assert(cinfo->src == NULL);
  cinfo->src = (jpeg_source_mgr*) (new rw_source(instream));
}

#define JPEG_CATCH_(RETT,RETF,COND) if (setjmp(mpErr->setjmp_buffer)) { \
    if (COND) {                                                         \
      mpErr->Cleanup();                                                 \
      niError(mpErr->GetError().Chars());                               \
      return RETT;                                                      \
    }                                                                   \
    else {                                                              \
      return RETF;                                                      \
    }                                                                   \
  }

#define JPEG_CATCH(RET,COND) JPEG_CATCH_(RET,RET,COND)

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
void cJpegReader::my_error_exit(j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr)cinfo->err;
  myerr->cinfo = cinfo;
  longjmp(myerr->setjmp_buffer,1);
}

///////////////////////////////////////////////
void cJpegReader::my_output_message(j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr)cinfo->err;
  myerr->cinfo = cinfo;
  niWarning(myerr->GetWarning().Chars());
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cJpegReader implementation.

///////////////////////////////////////////////
cJpegReader::cJpegReader(ni::iFile* apFile)
{
  mptrFile = apFile;

  mpErr = (my_error_ptr)niAlignedMalloc(sizeof(my_error_mgr),16);
  mInfo.err = jpeg_std_error(&mpErr->pub);
  mpErr->pub.error_exit = my_error_exit;
  mpErr->pub.output_message = my_output_message;
  mpErr->pReader = this;
  jpeg_create_decompress(&mInfo);
  setup_rw_source(&mInfo,mptrFile);
}

///////////////////////////////////////////////
cJpegReader::~cJpegReader()
{
  EndRead();

  rw_source* src = (rw_source*)mInfo.src;
  niSafeDelete(src);
  mInfo.src = NULL;

  jpeg_destroy_decompress(&mInfo);
  niFree(mpErr);
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cJpegReader::IsOK() const
{
  niClassIsOK(cJpegReader);
  return mptrFile.IsOK();
}

///////////////////////////////////////////////
tBool cJpegReader::ReadHeaderTables()
{
  ResetError();
  JPEG_CATCH(eFalse,1);
  int ret = jpeg_read_header(&mInfo,FALSE);
  if (ret != JPEG_HEADER_OK && ret != JPEG_HEADER_TABLES_ONLY) {
    niError(_A("Can't read jpeg header."));
    return eFalse;
  }

  jpeg_calc_output_dimensions(&mInfo);
  return eTrue;
}

///////////////////////////////////////////////
tBool cJpegReader::BeginRead()
{
  if (mpOutBuffer) {
    niError("Reading already started.");
    return eFalse;
  }

  JPEG_CATCH(eFalse,1);

  if (jpeg_read_header(&mInfo, TRUE) != JPEG_HEADER_OK) {
    niError(_A("Can't read header."));
    return eFalse;
  }

  if (!jpeg_start_decompress(&mInfo)) {
    niError(_A("Can't start jpeg decompression."));
    return eFalse;
  }

  mpOutBuffer = (tPtr)niMalloc(GetWidth()*GetNumComponents());
  return eTrue;
}

///////////////////////////////////////////////
tBool cJpegReader::EndRead()
{
  if (!mpOutBuffer) return eFalse;
  DiscardBuffer();
  niSafeFree(mpOutBuffer);
  JPEG_CATCH_(eFalse,eTrue,(mpErr->ErrCode() != JERR_TOO_LITTLE_DATA));
  jpeg_finish_decompress(&mInfo);
  ResetError();
  return eTrue;
}

///////////////////////////////////////////////
tBool cJpegReader::DiscardBuffer()
{
  rw_source* src = (rw_source*)mInfo.src;
  if (src) {
    src->discard_partial_buffer();
    return eTrue;
  }
  return eFalse;
}

///////////////////////////////////////////////
tU32 cJpegReader::GetWidth() const
{
  return mInfo.output_width;
}

///////////////////////////////////////////////
tU32 cJpegReader::GetHeight() const
{
  return mInfo.output_height;
}

///////////////////////////////////////////////
tU32 cJpegReader::GetNumComponents() const
{
  return mInfo.output_components;
}

///////////////////////////////////////////////
eColorSpace __stdcall cJpegReader::GetColorSpace() const
{
  switch (mInfo.out_color_space) {
    case JCS_CMYK:
      return eColorSpace_CMYK;
    case JCS_YCbCr:
      return eColorSpace_YCbCr;
    default:
      break;
  }
  return eColorSpace_RGB;
}

///////////////////////////////////////////////
tBool cJpegReader::ReadScanline(iFile *apOut)
{
  if (!mpOutBuffer) {
    niError(_A("Reading not began."));
    return eFalse;
  }

  JPEG_CATCH(eFalse,1);

  int lines_read = jpeg_read_scanlines(&mInfo,&mpOutBuffer,1);
  if (lines_read != 1) {
    niError(_A("Can't read next scanline."));
    return eFalse;
  }

  apOut->WriteRaw(mpOutBuffer,GetWidth()*GetNumComponents());
  return eTrue;
}

///////////////////////////////////////////////
iBitmap2D * cJpegReader::ReadBitmap(iGraphics *apGraphics)
{
  if (!BeginRead()) {
    niError(_A("Can't begin read."));
    return NULL;
  }

  const achar* aszFormat = NULL;
  switch (mInfo.out_color_space) {
    case JCS_CMYK:
      niError(_A("CMYK color space output not supported."));
      return NULL;
    case JCS_YCbCr:
      niError(_A("YCbCr color space output not supported."));
      return NULL;
    case JCS_GRAYSCALE:
      if (GetNumComponents() != 1) {
        niError(_A("Can output Grayscale with one component only."));
        return NULL;
      }
      aszFormat = _A("A8");
      break;
    case JCS_RGBA:
    case JCS_RGB:
      if (GetNumComponents() != 3 && GetNumComponents() != 4) {
        niError(_A("Can output RGB with three or four components only."));
        return NULL;
      }
      aszFormat =  GetNumComponents()==3?_A("R8G8B8"):_A("R8G8B8A8");
      break;
    default:
      break;
  }
  if (!aszFormat) {
    niError(_A("Unknown Jpeg output format."));
    return NULL;
  }

  Ptr<iBitmap2D> ptrBmp = apGraphics->CreateBitmap2D(GetWidth(),GetHeight(),aszFormat);
  if (!ptrBmp.IsOK()) {
    niError(_A("Can't create destination bitmap."));
    return NULL;
  }

  ResetError();
  JPEG_CATCH(NULL,1);

  const tU32 pitch = ptrBmp->GetPitch();
  const tPtr base = ptrBmp->GetData();
  niLoop(i,GetHeight()) {
    tPtr ptrOut = base+(pitch*i);
    int lines_read = jpeg_read_scanlines(&mInfo,&ptrOut,1);
    if (lines_read != 1) {
      niError(niFmt(_A("Can't read scanline '%d' (height:%d)."),i,GetHeight()));
      EndRead();
      return NULL;
    }
  }

  EndRead();
  return ptrBmp.GetRawAndSetNull();
}

///////////////////////////////////////////////
void cJpegReader::ResetError()
{
}
