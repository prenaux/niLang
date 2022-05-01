// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "Lang.h"

using namespace ni;

static const ni::tU32 _knEncoderBufferSize = 0xFFFF;

///////////////////////////////////////////////
iFile* __stdcall cLang::CreateFile(iFileBase* apBase) {
  return ni::CreateFile(apBase);
}

///////////////////////////////////////////////
iFileBase* __stdcall cLang::CreateFileBaseWriteDummy() {
  return ni::CreateFileBaseWriteDummy();
}
iFile* __stdcall cLang::CreateFileWriteDummy() {
  return ni::CreateFileWriteDummy();
}

///////////////////////////////////////////////
iFileBase* __stdcall cLang::CreateFileBaseMemory(tPtr pMem, tSize nSize, tBool bFree, const achar* aszPath) {
  return ni::CreateFileBaseMemory(pMem, nSize, bFree, aszPath);
}
iFile* __stdcall cLang::CreateFileMemory(tPtr pMem, tSize nSize, tBool bFree, const achar* aszPath) {
  return ni::CreateFileMemory(pMem, nSize, bFree, aszPath);
}

///////////////////////////////////////////////
iFileBase* __stdcall cLang::CreateFileBaseMemoryAlloc(tSize nSize, const achar* aszPath) {
  return ni::CreateFileBaseMemoryAlloc(nSize, aszPath);
}
iFile* __stdcall cLang::CreateFileMemoryAlloc(tSize nSize, const achar* aszPath) {
  return ni::CreateFileMemoryAlloc(nSize, aszPath);
}

///////////////////////////////////////////////
iFileBase* __stdcall cLang::CreateFileBaseDynamicMemory(tSize anSize, const achar* aszPath) {
  return ni::CreateFileBaseDynamicMemory(anSize, aszPath);
}
iFile* __stdcall cLang::CreateFileDynamicMemory(tSize anSize, const achar* aszPath) {
  return ni::CreateFileDynamicMemory(anSize, aszPath);
}

///////////////////////////////////////////////
iFileBase* __stdcall cLang::CreateFileBaseWindow(iFileBase* apBase, tI64 anBase, tI64 anSize, const achar* aaszPath, tBool abAutoSeekSet)
{
  return ni::CreateFileBaseWindow(apBase,anBase,anSize,aaszPath,abAutoSeekSet);
}
iFile* __stdcall cLang::CreateFileWindow(iFileBase* apBase, tI64 anBase, tI64 anSize, const achar* aaszPath, tBool abAutoSeekSet)
{
  return ni::CreateFileWindow(apBase,anBase,anSize,aaszPath,abAutoSeekSet);
}

///////////////////////////////////////////////
iFileSystem* __stdcall cLang::GetRootFS() const
{
  return ni::GetRootFS();
}

///////////////////////////////////////////////
iFileSystem* __stdcall cLang::CreateFileSystemDir(const achar* aaszDir, tFileSystemRightsFlags aRights) const
{
  return ni::CreateFileSystemDir(aaszDir,aRights);
}

///////////////////////////////////////////////
iFileSystem* __stdcall cLang::CreateFileSystemHashed(const iFileSystem* apFS) const {
  return ni::CreateFileSystemHashed(apFS);
}

///////////////////////////////////////////////
iFileBase* __stdcall cLang::CreateFileBaseBufferEncoder(iFileBase* apBase, iBufferEncoder* apEnc) {
  return ni::CreateFileBaseBufferEncoder(apBase, apEnc);
}

iFile* __stdcall cLang::CreateFileBufferEncoder(iFileBase* apBase, iBufferEncoder* apEnc) {
  return ni::CreateFileBufferEncoder(apBase, apEnc);
}

iFileBase* __stdcall cLang::CreateFileBaseBufferDecoder(iFileBase* apBase, iBufferDecoder* apDec, tSize aDecodedSize) {
  return ni::CreateFileBaseBufferDecoder(apBase, apDec, aDecodedSize);
}

iFile* __stdcall cLang::CreateFileBufferDecoder(iFileBase* apBase, iBufferDecoder* apDec, tSize aDecodedSize) {
  return ni::CreateFileBufferDecoder(apBase, apDec, aDecodedSize);
}

tSize __stdcall cLang::FileBufferEncode(iBufferEncoder* apEnc, iFile* apSrc, tSize anSrcSize, iFile* apDest) {
  return ni::FileBufferEncode(apEnc, apSrc, anSrcSize, apDest);
}

tSize __stdcall cLang::FileBufferDecode(iBufferDecoder* apDec, iFile* apSrc, tSize anSrcSize, iFile* apDest, tSize anDestSize) {
  return ni::FileBufferDecode(apDec, apSrc, anSrcSize, apDest, anDestSize);
}
