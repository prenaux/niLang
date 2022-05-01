#ifndef __FILEIO_H_7C5267C9_53F3_49F7_8E9A_DDEEE8FFC372__
#define __FILEIO_H_7C5267C9_53F3_49F7_8E9A_DDEEE8FFC372__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <niLang/IFile.h>
#include <niLang/IFileSystem.h>

/** \addtogroup niUnitTest
 * @{
 */

struct UnitTest_FileIO
{
  static const ni::tU32 knBufferSize = 4096;
  ni::iFileSystem* _fs;

  ni::tU8 buffer[knBufferSize];
  ni::tU8 tmp[knBufferSize];
  UnitTest_FileIO() {
    _fs = ni::GetLang()->GetRootFS();
    niLoop(i,knBufferSize) {
      buffer[i] = (ni::tU8)i;
    }
    niLoop(i,knBufferSize) {
      tmp[i] = 255-(ni::tU8)i;
    }
  }

  ni::tBool CheckTmp(ni::tSize offset, ni::tSize size) {
    return ni::MemCmp(buffer+offset,
                      tmp+offset,
                      size) == 0;
  }

  ni::cString RunTest_WriteBuffer(ni::iFile* fp) {
    if (!niIsOK(fp)) return _A("WriteBuffer - InvalidFP");
    fp->SeekSet(0);
    ni::tU32 writeSize = fp->WriteRaw(buffer,sizeof(buffer));
    if (writeSize != sizeof(buffer))
      return niFmt(_A("WriteBuffer - Invalid write size '%d', expected '%d'\n"),
                   writeSize,(ni::tU32)sizeof(buffer));
    return _A("OK");
  }

  ni::cString RunTest_ReadPartial(ni::iFile* fp) {
    if (!niIsOK(fp)) return _A("ReadPartial - InvalidFP");
    fp->SeekSet(0);
    if (1000 != fp->ReadRaw(tmp,1000)) return _A("ReadPartial - Can't read 1000 (1)");
    if (1000 != fp->ReadRaw(tmp+1000,1000)) return _A("ReadPartial - Can't read 1000 (2)");
    if (1000 != fp->ReadRaw(tmp+2000,1000)) return _A("ReadPartial - Can't read 1000 (3)");
    if (1000 != fp->ReadRaw(tmp+3000,1000)) return _A("ReadPartial - Can't read 1000 (4)");
    if (96 != fp->ReadRaw(tmp+4000,1000)) return _A("ReadPartial - Can't read 96 of 1000 (5)");
    if (!CheckTmp(0,4096)) return _A("ReadPartial - Can't check data read.");
    if (!fp->GetPartialRead())
      return _A("ReadPartial - Not PartialRead");
    if (!fp->GetIsEncoder() && 4096 != fp->Tell())
      return niFmt(_A("ReadPartial - Invalid Tell '%d', expected '4096'"),
                   fp->Tell());
    return _A("OK");
  }

  ni::cString RunTest_Read(ni::iFile* fp) {
    if (!niIsOK(fp)) return _A("Read - InvalidFP");
    fp->SeekSet(0);
    if (1000 != fp->ReadRaw(tmp,1000)) return _A("Read - Can't read 1000 (1)");
    if (1000 != fp->ReadRaw(tmp+1000,1000)) return _A("Read - Can't read 1000 (2)");
    if (1000 != fp->ReadRaw(tmp+2000,1000)) return _A("Read - Can't read 1000 (3)");
    if (1000 != fp->ReadRaw(tmp+3000,1000)) return _A("Read - Can't read 1000 (4)");
    if (96 != fp->ReadRaw(tmp+4000,96)) return _A("Read - Can't read 96 (5)");
    if (!CheckTmp(0,4096)) return _A("Read - Can't check data read.");
    if (fp->GetPartialRead())
      return _A("Read - should not be PartialRead");
    if (!fp->GetIsEncoder() && 4096 != fp->Tell())
      return niFmt(_A("Read - Invalid Tell '%d', expected '4096'"),
                   fp->Tell());
    return _A("OK");
  }

  ni::cString RunTest_Write(ni::iFile* fp) {
    if (!niIsOK(fp)) return _A("Write - InvalidFP");
    fp->SeekSet(0);
    if (1000 != fp->WriteRaw(tmp,1000)) return _A("Write - Can't write 1000 (1)");
    if (1000 != fp->WriteRaw(tmp,1000)) return _A("Write - Can't write 1000 (2)");
    if (1000 != fp->WriteRaw(tmp,1000)) return _A("Write - Can't write 1000 (3)");
    if (1000 != fp->WriteRaw(tmp,1000)) return _A("Write - Can't write 1000 (4)");
    if (96 != fp->WriteRaw(tmp,96)) return _A("Write - Can't write 96 (5)");
    if (!fp->GetIsEncoder() && 4096 != fp->Tell())
      return niFmt(_A("Write - Invalid Tell '%d', expected '4096'"),fp->Tell());
    if (!fp->GetCanRead()) {
      if (4096 != fp->GetSize() && 0 != fp->GetSize())
        return niFmt(_A("Write (WO) - Invalid Size '%d', expected '4096' or '0'"),fp->GetSize());
    }
    else {
      if (4096 != fp->GetSize())
        return niFmt(_A("Write (RW) - Invalid Size '%d', expected '4096'"),fp->GetSize());
    }
    return _A("OK");
  }
};

/**@}*/

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __FILEIO_H_7C5267C9_53F3_49F7_8E9A_DDEEE8FFC372__
