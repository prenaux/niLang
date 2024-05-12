#ifndef __JPEGWRITER_31712392_H__
#define __JPEGWRITER_31712392_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niUI/IJpegWriter.h"

extern "C" {
#include "jpeglib/jpeglib.h"
}
#include <setjmp.h>

//////////////////////////////////////////////////////////////////////////////////////////////
// cJpegWriter declaration.
class cJpegWriter : public ni::ImplRC<ni::iJpegWriter,ni::eImplFlags_Default>
{
  niBeginClass(cJpegWriter);

 public:
  //! Constructor.
  cJpegWriter();
  //! Destructor.
  ~cJpegWriter();

  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iJpegWriter /////////////////////////
  tBool __stdcall BeginWrite(iFile *apDest,
                             tU32 anWidth, tU32 anHeight,
                             eColorSpace aInCS, tU32 anC,
                             tU32 anQuality, tJpegWriteFlags aFlags);
  tBool __stdcall EndWrite();
  tBool __stdcall WriteScanline(iFile *apSrc);
  tBool __stdcall WriteBitmap(iFile* apDest, const iBitmap2D *apBmp, tU32 anQuality, tJpegWriteFlags aFlags);
  //// ni::iJpegWriter /////////////////////////

 private:
  struct my_error_mgr
  {
    // setjmp_buffer must be the first member of the struct to make sure its
    // memory aligned
    jmp_buf        setjmp_buffer;
    jpeg_error_mgr pub;
    cJpegWriter*   pWriter;
    j_common_ptr   cinfo;
    void Cleanup() {
      if (pWriter) { pWriter->_EndWriteCleanup(); }
    }
    int ErrCode() const {
      return pub.msg_code;
    }
    cString GetError() {
      char szBuffer[JMSG_LENGTH_MAX];
      (*cinfo->err->format_message)(cinfo, szBuffer);
      return niFmt(_A("JPEG WRITE: %s"), _ASZ(szBuffer));
    }
    cString GetWarning() {
      char szBuffer[JMSG_LENGTH_MAX];
      (*cinfo->err->format_message)(cinfo, szBuffer);
      return niFmt(_A("JPEG WRITE: %s"), _ASZ(szBuffer));
    }
  } niAligned(16); // jmp_buf requires 16 bytes alignment
  typedef struct my_error_mgr *my_error_ptr;
  static void my_error_exit(j_common_ptr cinfo);
  static void my_output_message(j_common_ptr cinfo);

  void _EndWriteCleanup();

  Ptr<iFile>           mptrFile;
  astl::vector<tU8>    mvOutBuffer;
  jpeg_compress_struct mInfo;
  my_error_ptr         mpErr;

  niEndClass(cJpegWriter);
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __JPEGWRITER_31712392_H__
