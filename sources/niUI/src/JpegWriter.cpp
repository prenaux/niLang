// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "JpegWriter.h"
#include <niUI_ModuleDef.h>

// Disables "interaction between '_setjmp' and C++ object destruction is non-portable"
#ifdef _MSC_VER
#pragma warning (disable : 4611)
#endif

niExportFunc(iUnknown*) New_niUI_JpegWriter(const Var&, const Var&) {
  return niNew cJpegWriter();
}

static const size_t IO_BUF_SIZE = 4096;

// A jpeglib destination manager that writes to a ni::iFile.
// Paraphrased from IJG jpeglib jdatadst.c.
struct rw_dest
{
  struct jpeg_destination_mgr m_pub;  /* public fields */

  ni::iFile*  m_out_stream;   /* source stream */
  JOCTET  m_buffer[IO_BUF_SIZE];  /* start of buffer */

  rw_dest(ni::iFile* out)
      :
      m_out_stream(out)
      // Constructor.  The caller is responsible for closing
      // the output stream after it's done using us.
  {
    // fill in function pointers...
    m_pub.init_destination = init_destination;
    m_pub.empty_output_buffer = empty_output_buffer;
    m_pub.term_destination = term_destination;

    m_pub.next_output_byte = m_buffer;
    m_pub.free_in_buffer = IO_BUF_SIZE;
  }

  static void init_destination(j_compress_ptr cinfo)
  {
    rw_dest*  dest = (rw_dest*) cinfo->dest;
    niAssert(dest);

    dest->m_pub.next_output_byte = dest->m_buffer;
    dest->m_pub.free_in_buffer = IO_BUF_SIZE;
  }

  static boolean  empty_output_buffer(j_compress_ptr cinfo)
      // Write the output buffer into the stream.
  {
    rw_dest*  dest = (rw_dest*) cinfo->dest;
    niAssert(dest);

    if (dest->m_out_stream->WriteRaw(dest->m_buffer, IO_BUF_SIZE) != IO_BUF_SIZE)
    {
      // Error.
      // @@ bah, exceptions suck.  TODO consider alternatives.
      // throw "jpeg::rw_dest couldn't write data.";
      return FALSE;
    }

    dest->m_pub.next_output_byte = dest->m_buffer;
    dest->m_pub.free_in_buffer = IO_BUF_SIZE;

    return TRUE;
  }

  static void term_destination(j_compress_ptr cinfo)
      // Terminate the destination.  Flush any leftover
      // data, and make sure we get deleted.
  {
    rw_dest*  dest = (rw_dest*) cinfo->dest;
    niAssert(dest);

    // Write any remaining data.
    ssize_t datacount = IO_BUF_SIZE - dest->m_pub.free_in_buffer;
    if (datacount > 0) {
      if ((int)dest->m_out_stream->WriteRaw(dest->m_buffer, datacount) != datacount)
      {
        // Error.
        // throw "jpeg::rw_dest::term_destination couldn't write data.";
      }
    }

    // Clean ourselves up.
    niDelete dest;
    cinfo->dest = NULL;
  }
};


static void setup_rw_dest(j_compress_ptr cinfo, ni::iFile* outstream)
    // Set up the given compress object to write to the given
    // output stream.
{
  cinfo->dest = (jpeg_destination_mgr*) (new rw_dest(outstream));
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

///////////////////////////////////////////////
void cJpegWriter::my_error_exit(j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr)cinfo->err;
  myerr->cinfo = cinfo;
  longjmp(myerr->setjmp_buffer,1);
}

///////////////////////////////////////////////
void cJpegWriter::my_output_message(j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr)cinfo->err;
  myerr->cinfo = cinfo;
  niWarning(myerr->GetWarning().Chars());
}

//////////////////////////////////////////////////////////////////////////////////////////////
// cJpegWriter implementation.

///////////////////////////////////////////////
cJpegWriter::cJpegWriter() {
  mpErr = (my_error_ptr)niAlignedMalloc(sizeof(my_error_mgr),16);
}

///////////////////////////////////////////////
cJpegWriter::~cJpegWriter()
{
  EndWrite();
  niSafeFree(mpErr);
}

///////////////////////////////////////////////
//! Sanity check.
ni::tBool __stdcall cJpegWriter::IsOK() const
{
  niClassIsOK(cJpegWriter);
  return ni::eTrue;
}

///////////////////////////////////////////////
tBool cJpegWriter::BeginWrite(iFile *apDest,
                              tU32 anWidth, tU32 anHeight,
                              eColorSpace aInCS, tU32 anInC,
                              tU32 anQuality, tJpegWriteFlags aFlags)
{
  if (niIsOK(mptrFile)) {
    niError(_A("Write already began."));
    return eFalse;
  }

  mptrFile = apDest;
  if (!niIsOK(mptrFile)) {
    niError(_A("Invalid destination file."));
    return eFalse;
  }

  mInfo.err = jpeg_std_error(&mpErr->pub);
  mpErr->pub.error_exit = my_error_exit;
  mpErr->pub.output_message = my_output_message;
  mpErr->pWriter = this;
  jpeg_create_compress(&mInfo);
  setup_rw_dest(&mInfo,mptrFile);

  mInfo.image_width = anWidth;
  mInfo.image_height = anHeight;
  mInfo.input_components = anInC;
  switch (aInCS) {
    case eColorSpace_RGB:
      mInfo.in_color_space = anInC==4 ? JCS_RGBA : JCS_RGB;
      break;
    case eColorSpace_YCbCr:
      mInfo.in_color_space = JCS_YCbCr;
      break;
    case eColorSpace_YCoCg:
      mInfo.in_color_space = anInC==4 ? JCS_YCoCgA : JCS_YCoCg;
      break;
    case eColorSpace_CMYK:
      mInfo.in_color_space = JCS_CMYK;
      break;
    default:
      niError(niFmt("Unsupported color space '%s'.", niEnumToChars(eColorSpace,aInCS)));
      return eFalse;
  }

  JPEG_CATCH(eFalse,1);

  jpeg_set_defaults(&mInfo);
  if (niFlagIs(aFlags,eJpegWriteFlags_Alpha)) {
    jpeg_set_colorspace(&mInfo,JCS_YCoCgA);
  }
  else if (niFlagIs(aFlags,eJpegWriteFlags_YCoCg)) {
    jpeg_set_colorspace(&mInfo,JCS_YCoCg);
  }
  jpeg_set_quality(&mInfo, anQuality, TRUE);
  jpeg_start_compress(&mInfo, TRUE);

  mvOutBuffer.resize(anWidth*anInC);
  return eTrue;
}

///////////////////////////////////////////////
void cJpegWriter::_EndWriteCleanup() {
  rw_dest* dest = (rw_dest*)mInfo.dest;
  niSafeDelete(dest);
  mvOutBuffer.clear();
  mInfo.dest = NULL;
  jpeg_destroy_compress(&mInfo);
  mptrFile = NULL;
}

///////////////////////////////////////////////
tBool cJpegWriter::EndWrite()
{
  if (!mptrFile.IsOK()) return eFalse;
  JPEG_CATCH(eFalse,1);
  jpeg_finish_compress(&mInfo);
  _EndWriteCleanup();
  return eTrue;
}

///////////////////////////////////////////////
tBool cJpegWriter::WriteScanline(iFile *apSrc)
{
  JPEG_CATCH(eFalse,1);
  apSrc->WriteRaw(&mvOutBuffer[0],mInfo.image_width*mInfo.input_components);
  tPtr pLine = mvOutBuffer.data();
  jpeg_write_scanlines(&mInfo, &pLine, 1);
  return eTrue;
}

///////////////////////////////////////////////
tBool cJpegWriter::WriteBitmap(iFile* apDest, const iBitmap2D *apBmp, tU32 anQuality, tJpegWriteFlags aFlags)
{
  eColorSpace cs = eColorSpace_RGB;
  tU32 numC = 0;
  tU32 nFlags = aFlags;
  if (ni::StrCmp(apBmp->GetPixelFormat()->GetFormat(),_A("R8G8B8")) == 0) {
    numC = 3;
    niFlagOff(nFlags,eJpegWriteFlags_Alpha);
  }
  else if (ni::StrCmp(apBmp->GetPixelFormat()->GetFormat(),_A("R8G8B8A8")) == 0) {
    numC = 4;
    niFlagOn(nFlags,eJpegWriteFlags_Alpha);
  }
  else {
    niError(niFmt(_A("Unsupported pixel format '%s'."),apBmp->GetPixelFormat()->GetFormat()));
    return eFalse;
  }

  if (!BeginWrite(apDest,
                  apBmp->GetWidth(),apBmp->GetHeight(),
                  cs,numC,
                  anQuality,nFlags)) {
    niError(_A("BeginWrite failed."));
    return eFalse;
  }

  JPEG_CATCH(eFalse,1);
  for (tU32 i = 0; i < apBmp->GetHeight(); ++i) {
    tPtr ptrOut = apBmp->GetData()+(apBmp->GetPitch()*i);
    jpeg_write_scanlines(&mInfo,&ptrOut,1);
  }

  return EndWrite();
}
