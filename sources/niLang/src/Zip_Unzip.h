/* unzip.h -- IO for uncompress .zip files using zlib
   Version 0.15 beta, Mar 19th, 1998,

   Copyright (C) 1998 Gilles Vollant

   This unzip package allow extract file from .ZIP file, compatible with PKZip 2.04g
   WinZip, InfoZip tools and compatible.
   Encryption and multi volume ZipFile (span) are not supported.
   Old compressions used by old PKZip 1.x are not supported

   THIS IS AN ALPHA VERSION. AT THIS STAGE OF DEVELOPPEMENT, SOMES API OR STRUCTURE
   CAN CHANGE IN FUTURE VERSION !!
   I WAIT FEEDBACK at mail info@winimage.com
   Visit also http://www.winimage.com/zLibDll/unzip.htm for evolution

   Condition of use and distribution are the same than zlib :

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
   3. This notice may not be removed or altered from any source distribution.


*/
/* for more info about .ZIP format, see
   ftp://ftp.cdrom.com/pub/infozip/doc/appnote-970311-iz.zip
   PkWare has also a specification at :
   ftp://ftp.pkware.com/probdesc.zip */

#ifndef _unz_H
#define _unz_H

#include "API/niLang/IZip.h"
#include "zlib/zlib.h"
using namespace ni;

#if defined(STRICTUNZIP) || defined(STRICTZIPUNZIP)
/* like the STRICT of WIN32, we define a pointer that cannot be converted
   from (void*) without cast */
typedef struct TagunzFile__ { int unused; } unzFile__;
typedef unzFile__ *unzFile;
#else
typedef voidp unzFile;
#endif

niExportFunc(int) niUnzip_StringFileNameCompare(const char* fileName1, const char* fileName2, int iCaseSensitivity);
/*
  Compare two filename (fileName1,fileName2).
  If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
  If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
  or strcasecmp)
  If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
  (like 1 on Unix, 2 on Windows)
*/


niExportFunc(unzFile) niUnzip_Open(ni::iFile*);
/*
  Open a Zip file. path contain the full pathname (by example,
  on a Windows NT computer "c:\\zlib\\zlib111.zip" or on an Unix computer
  "zlib/zlib111.zip".
  If the zipfile cannot be opened (file don't exist or in not valid), the
  return value is NULL.
  Else, the return value is a unzFile Handle, usable with other function
  of this unzip package.
*/

niExportFunc(int) niUnzip_Close(unzFile file);
/*
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzOpenCurrentFile (see later),
  these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. */

niExportFunc(int) niUnzip_GetGlobalInfo(unzFile file,
                         unz_global_info *pglobal_info);
/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */


niExportFunc(int) niUnzip_GetGlobalComment(unzFile file,
                            char *szComment,
                            unsigned long uSizeBuf);
/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/


/***************************************************************************/
/* Unzip package allow you browse the directory of the zipfile */

niExportFunc(int) niUnzip_GoToFirstFile(unzFile file);
/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/

niExportFunc(int) niUnzip_GoToNextFile(unzFile file);
/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/

niExportFunc(int) niUnzip_LocateFile(unzFile file,
                      const char *szFileName,
                      int iCaseSensitivity);

niExportFunc(int) niUnzip_LocateFileMy(unzFile file, unsigned long num, unsigned long pos);
/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/


niExportFunc(int) niUnzip_GetCurrentFileInfo(unzFile file,
                              unz_file_info *pfile_info,
                              char *szFileName,
                              unsigned long fileNameBufferSize,
                              void *extraField,
                              unsigned long extraFieldBufferSize,
                              char *szComment,
                              unsigned long commentBufferSize);
/*
  Get Info about the current file
  if pfile_info!=NULL, the *pfile_info structure will contain somes info about
  the current file
  if szFileName!=NULL, the filemane string will be copied in szFileName
  (fileNameBufferSize is the size of the buffer)
  if extraField!=NULL, the extra field information will be copied in extraField
  (extraFieldBufferSize is the size of the buffer).
  This is the Central-header version of the extra field
  if szComment!=NULL, the comment string of the file will be copied in szComment
  (commentBufferSize is the size of the buffer)
*/

/***************************************************************************/
/* for reading the content of the current zipfile, you can open it, read data
   from it, and close it (you can close it before reading all the file)
*/

niExportFunc(int) niUnzip_OpenCurrentFile(unzFile file, const char* password);
/*
  Open for reading data the current file in the zipfile.
  If there is no error, the return value is UNZ_OK.
*/

niExportFunc(int) niUnzip_CloseCurrentFile(unzFile file);
/*
  Close the file in zip opened with unzOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/


niExportFunc(int) niUnzip_ReadCurrentFile(unzFile file,
                           voidp buf,
                           unsigned len);
/*
  Read bytes from the current file (opened by unzOpenCurrentFile)
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
  (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/

niExportFunc(z_off_t) niUnzip_tell(unzFile file);
/*
  Give the current position in uncompressed data
*/

niExportFunc(int) niUnzip_eof(unzFile file);
/*
  return 1 if the end of file was reached, 0 elsewhere
*/

niExportFunc(int) niUnzip_GetLocalExtrafield(unzFile file,
                              voidp buf,
                              unsigned len);
/*
  Read extra field from the current file (opened by unzOpenCurrentFile)
  This is the local-header version of the extra field (sometimes, there is
  more info in the local-header version than in the central-header)

  if buf==NULL, it return the size of the local extra field

  if buf!=NULL, len is the size of the buffer, the extra header is copied in
  buf.
  the return value is the number of bytes copied in buf, or (if <0)
  the error code
*/

#endif /* _unz_H */
