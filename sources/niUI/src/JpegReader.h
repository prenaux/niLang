#ifndef __JPEGREADER_18852172_H__
#define __JPEGREADER_18852172_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "API/niUI/IJpegReader.h"

extern "C" {
#include "jpeglib/jpeglib.h"
#include "jpeglib/jerror.h"
}
#include <setjmp.h>

//////////////////////////////////////////////////////////////////////////////////////////////
// cJpegReader declaration.
class cJpegReader : public ni::ImplRC<ni::iJpegReader,ni::eImplFlags_Default>
{
  niBeginClass(cJpegReader);

 public:
  struct my_error_mgr
  {
    jmp_buf         setjmp_buffer;
    jpeg_error_mgr  pub;
    cJpegReader*    pReader;
    j_common_ptr    cinfo;
    void Cleanup() {
      if (pReader) { pReader->EndRead(); }
    }
    int ErrCode() const {
      return pub.msg_code;
    }
    cString GetError() {
      char szBuffer[JMSG_LENGTH_MAX];
      (*cinfo->err->format_message)(cinfo, szBuffer);
      return niFmt(_A("JPEG READ: %s"), _ASZ(szBuffer));
    }
    cString GetWarning() {
      char szBuffer[JMSG_LENGTH_MAX];
      (*cinfo->err->format_message)(cinfo, szBuffer);
      return niFmt(_A("JPEG READ: %s"), _ASZ(szBuffer));
    }
  } niAligned(16); // jmp_buf requires 16 bytes alignment
  typedef struct my_error_mgr *my_error_ptr;
  static void my_error_exit(j_common_ptr cinfo);
  static void my_output_message(j_common_ptr cinfo);

 public:
  //! Constructor.
  cJpegReader(ni::iFile* apFile);
  //! Destructor.
  ~cJpegReader();

  //! Sanity check.
  ni::tBool __stdcall IsOK() const;

  //// ni::iJpegReader /////////////////////////
  iFile* __stdcall GetFile() const { return mptrFile; }
  tBool __stdcall ReadHeaderTables();
  tBool __stdcall BeginRead();
  tBool __stdcall EndRead();
  tBool __stdcall DiscardBuffer();
  tU32 __stdcall GetWidth() const;
  tU32 __stdcall GetHeight() const;
  tU32 __stdcall GetNumComponents() const;
  eColorSpace __stdcall GetColorSpace() const;
  tBool __stdcall ReadScanline(iFile *apOut);
  iBitmap2D * __stdcall ReadBitmap(iGraphics *apGraphics);
  //// ni::iJpegReader /////////////////////////

  void ResetError();

 private:
  Ptr<iFile>             mptrFile;
  tPtr                   mpOutBuffer;
  jpeg_decompress_struct mInfo;
  my_error_ptr           mpErr;

  niEndClass(cJpegReader);
};



/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __JPEGREADER_18852172_H__
