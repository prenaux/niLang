// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "stdafx.h"
#include "FreetypeWrapper.h"

using namespace ni;

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_SYSTEM_H
#include FT_GLYPH_H
#include FT_MODULE_H

#include <freetype/internal/ftobjs.h>

//////////////////////////////////////////////////////////////////////////////////////////////
/* Freetype lib fixes :

   FT_BASE_DEF( void )
   FT_Stream_Close( FT_Stream  stream ) {
     if ( stream && stream->close ) {
       FT_Stream_CloseFunc closefunc = stream->close;
       stream->close = NULL;
       closefunc( stream );
     }
   }

*/

cString MyFT_GetErrorString(FT_Error error)
{
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
  static const struct
  {
    int          err_code;
    const char*  err_msg;
  } ft_errors[] = {
#include FT_ERRORS_H
  };

  for (int i = 0; i < ((sizeof ft_errors)/(sizeof ft_errors[0])); ++i)
  {
    if (error == ft_errors[i].err_code)
    {
      return ft_errors[i].err_msg;
    }
  }

  return AZEROSTR;
}

static void* MyFT_Alloc(FT_Memory memory, long size);
static void MyFT_Free(FT_Memory memory, void* block);
static void* MyFT_Realloc(FT_Memory memory, long cur_size, long new_size, void* block);

static FT_MemoryRec_  _MyFT_Memory = {
  NULL,
  MyFT_Alloc,
  MyFT_Free,
  MyFT_Realloc
};

static void* MyFT_Alloc(FT_Memory memory, long size) {
  return niMalloc(size);
}
static void MyFT_Free(FT_Memory memory, void* block) {
  niAssert (block != &_MyFT_Memory);
  niFree(block);
}
static void* MyFT_Realloc(FT_Memory memory, long cur_size, long new_size, void* block) {
  return niRealloc(block,new_size);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Freetype stream

static unsigned long MyFT_Stream_IoFunc(FT_Stream stream, unsigned long   offset, unsigned char*  buffer, unsigned long   count)
{
  iFile* pFile = reinterpret_cast<iFile*>(stream->descriptor.pointer);
  pFile->SeekSet(offset);
  if (count)
    return pFile->ReadRaw((void*)buffer,count);
  else
    return 0;
}

static void MyFT_Stream_Close(FT_Stream stream)
{
  iFile* pFile = reinterpret_cast<iFile*>(stream->descriptor.pointer);
  //     niDebugFmt((_A("# STREAM CLOSE: %s\n"),pFile->GetSourcePath()));
  niSafeRelease(pFile);
  niFree(stream);
}

niExportFunc(FT_Face) MyFT_Face_Open(FT_Library aLib, iFile* apFile)
{
  if (!niIsOK(apFile)) {
    niError(_A("Invalid iFile."));
    return NULL;
  }

  FT_Stream stm = (FT_Stream)niMalloc(sizeof(*stm));
  if (stm == NULL) {
    niError(_A("Can't allocate stream memory."));
    return NULL;
  }
  memset(stm,0,sizeof(*stm));

  stm->descriptor.pointer = (void*)apFile;
  apFile->AddRef();
  stm->memory = aLib->memory;
  stm->read = MyFT_Stream_IoFunc;
  stm->close = MyFT_Stream_Close;
  stm->pos = (size_t)apFile->Tell();
  stm->size = (size_t)apFile->GetSize();

  FT_Face face = NULL;
  FT_Open_Args args;
  args.flags = FT_OPEN_STREAM;
  args.stream = stm;

  FT_Error err = FT_Open_Face(aLib, &args, 0, &face);
  if (FT_FAILED(err)) {
    // We don't need to free or release the file, FreeType calls the Stream_Close
    // callback in case of error.
    niError(niFmt(_A("Can't open face '%s' !\nFTERROR: %s."), apFile->GetSourcePath(), MyFT_GetErrorString(err).Chars()));
    return NULL;
  }

  return face;
}

//////////////////////////////////////////////////////////////////////////////////////////////

static tBool MyFT_Startup(FT_Library* apLibrary)
{
  if (FT_FAILED(FT_New_Library(&_MyFT_Memory,apLibrary)))
    return eFalse;
  (*apLibrary)->version_major = FREETYPE_MAJOR;
  (*apLibrary)->version_minor = FREETYPE_MINOR;
  (*apLibrary)->version_patch = FREETYPE_PATCH;
  FT_Add_Default_Modules(*apLibrary);
  return eTrue;
}

static void MyFT_Shutdown(FT_Library* apLibrary)
{
  if (*apLibrary)
  {
    FT_Done_Library(*apLibrary);
    *apLibrary = NULL;
  }
}

sMyFTLibrary::sMyFTLibrary()
{
  if (!MyFT_Startup(&mLibrary))
  {
    mLibrary = NULL;
  }
}

sMyFTLibrary::~sMyFTLibrary()
{
  MyFT_Shutdown(&mLibrary);
}
