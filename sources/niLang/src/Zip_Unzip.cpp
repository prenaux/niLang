/* unzip.c -- IO on .zip files using zlib
   Version 0.15 beta, Mar 19th, 1998,

   Read unzip.h for more info
*/

#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/StringLib.h"
#include "zlib/zlib.h"
#include "Zip_Unzip.h"

// Table of CRC-32's of all single-byte values (made by make_crc_table)
const uLong crc_table[256] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

// const uLong * get_crc_table() {
//     return (const uLong *)crc_table;
// }

#define CRC_DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define CRC_DO2(buf)  CRC_DO1(buf); CRC_DO1(buf);
#define CRC_DO4(buf)  CRC_DO2(buf); CRC_DO2(buf);
#define CRC_DO8(buf)  CRC_DO4(buf); CRC_DO4(buf);

// uLong ucrc32(uLong crc, const Byte *buf, uInt len)
// { if (buf == Z_NULL) return 0L;
//   crc = crc ^ 0xffffffffL;
//   while (len >= 8)  {CRC_DO8(buf); len -= 8;}
//   if (len) do {CRC_DO1(buf);} while (--len);
//   return crc ^ 0xffffffffL;
// }



// =============================================================
// some decryption routines
#define CRC32(c, b) (crc_table[((int)(c)^(b))&0xff]^((c)>>8))
void Uupdate_keys(unsigned long *keys, char c)
{ keys[0] = CRC32(keys[0],c);
  keys[1] += keys[0] & 0xFF;
  keys[1] = keys[1]*134775813L +1;
  keys[2] = CRC32(keys[2], keys[1] >> 24);
}
char Udecrypt_byte(unsigned long *keys)
{ unsigned temp = ((unsigned)keys[2] & 0xffff) | 2;
  return (char)(((temp * (temp ^ 1)) >> 8) & 0xff);
}
char zdecode(unsigned long *keys, char c)
{ c^=Udecrypt_byte(keys);
  Uupdate_keys(keys,c);
  return c;
}

#ifndef local
#  define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */



#if !defined(unix) && !defined(CASESENSITIVITYDEFAULT_YES) && \
  !defined(CASESENSITIVITYDEFAULT_NO)
#define CASESENSITIVITYDEFAULT_NO
#endif


#ifndef UNZ_BUFSIZE
#define UNZ_BUFSIZE (16384)
#endif

#ifndef UNZ_MAXFILENAMEINZIP
#define UNZ_MAXFILENAMEINZIP (256)
#endif

#ifndef ALLOC
# define ALLOC(size) (niMalloc(size))
#endif
#ifndef TRYFREE
# define TRYFREE(p) {if (p) niFree(p);}
#endif

#define SIZECENTRALDIRITEM (0x2e)
#define SIZEZIPLOCALHEADER (0x1e)


/* I've found an old Unix (a SunOS 4.1.3_U1) without all SEEK_* defined.... */

#ifndef SEEK_CUR
#define SEEK_CUR    1
#endif

#ifndef SEEK_END
#define SEEK_END    2
#endif

#ifndef SEEK_SET
#define SEEK_SET    0
#endif

/* unz_file_info_interntal contain internal info about a file in zipfile*/
typedef struct unz_file_info_internal_s {
  unsigned long offset_curfile;/* relative offset of local header 4 bytes */
} unz_file_info_internal;


/* file_in_zip_read_info_s contain internal information about a file in zipfile,
   when reading and decompress it */
typedef struct {
  char  *read_buffer;         /* internal buffer for compressed data */
  z_stream stream;            /* zLib stream structure for inflate */

  unsigned long pos_in_zipfile;       /* position in byte on the zipfile, for fseek*/
  unsigned long stream_initialised;   /* flag set if stream structure is initialised*/

  unsigned long offset_local_extrafield;/* offset of the local extra field */
  uInt  size_local_extrafield;/* size of the local extra field */
  unsigned long pos_local_extrafield;   /* position in the local extra field in read*/

  unsigned long crc32;                /* crc32 of all data uncompressed */
  unsigned long crc32_wait;           /* crc32 we must obtain after decompress all */
  unsigned long rest_read_compressed; /* number of byte to be decompressed */
  unsigned long rest_read_uncompressed;/*number of byte to be obtained after decomp*/
  ni::iFile* file;                 /* io structore of the zipfile */
  unsigned long compression_method;   /* compression method (0==store) */
  unsigned long byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/

  bool encrypted;               // is it encrypted?
  unsigned long keys[3];        // decryption keys, initialized by unzOpenCurrentFile
  int encheadleft;              // the first call(s) to unzReadCurrentFile will read this many encryption-header bytes first
  char crcenctest;              // if encrypted, we'll check the encryption buffer against this
} file_in_zip_read_info_s;


/* unz_s contain internal information about the zipfile
 */
typedef struct {
  ni::iFile* file;                 /* io structore of the zipfile */
  unz_global_info gi;       /* public global information */
  unsigned long byte_before_the_zipfile;/* byte before the zipfile, (>0 for sfx)*/
  unsigned long num_file;             /* number of the current file in the zipfile*/
  unsigned long pos_in_central_dir;   /* pos of the current file in the central dir*/
  unsigned long current_file_ok;      /* flag about the usability of the current file*/
  unsigned long central_pos;          /* position of the beginning of the central dir*/

  unsigned long size_central_dir;     /* size of the central directory  */
  unsigned long offset_central_dir;   /* offset of start of central directory with
                                         respect to the starting disk number */

  unz_file_info cur_file_info; /* public info about the current file in zip*/
  unz_file_info_internal cur_file_info_internal; /* private info about it*/
  file_in_zip_read_info_s* pfile_in_zip_read; /* structure about the current
                                                 file if we are decompressing it */
} unz_s;


/* ===========================================================================
   Read a byte from a gz_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been sucessfully opened for reading.
*/


local int unzlocal_getShort(iFile* fin,unsigned long *pi) {
  *pi = fin->ReadLE16();
  return (fin->GetFileFlags()&eFileFlags_PartialRead)?UNZ_EOF:UNZ_OK;
}

local int unzlocal_getLong(iFile* fin,unsigned long *pi) {
  *pi = fin->ReadLE32();
  return (fin->GetFileFlags()&eFileFlags_PartialRead)?UNZ_EOF:UNZ_OK;
}


#define BUFREADCOMMENT (0x400)

/*
  Locate the Central directory of a zipfile (at the end, just before
  the global comment)
*/
local unsigned long unzlocal_SearchCentralDir(iFile *fin) {
  unsigned char* buf;
  unsigned long uSizeFile;
  unsigned long uBackRead;
  unsigned long uMaxBack=0xffff; /* maximum size of global comment */
  unsigned long uPosFound=0;

  if (!fin->SeekEnd(0))
    return 0;

  fin->SeekEnd(4);
  unsigned long id;
  fin->ReadRaw(&id,4);
  if (id == niFourCC('A','E','X','E')) {
    fin->SeekEnd(12); // skip 12 bytes, the back header, if it's a AEXE file
  }
  uSizeFile = (ni::tU32)fin->Tell();

  if (uMaxBack>uSizeFile) uMaxBack = uSizeFile;

  buf = (unsigned char*)ALLOC(BUFREADCOMMENT+4);
  if (!buf) return 0;

  uBackRead = 4;
  while (uBackRead<uMaxBack) {
    unsigned long uReadSize,uReadPos ;
    int i;
    if (uBackRead+BUFREADCOMMENT>uMaxBack) uBackRead = uMaxBack;
    else uBackRead+=BUFREADCOMMENT;
    uReadPos = uSizeFile-uBackRead ;

    uReadSize = ((BUFREADCOMMENT+4) < (uSizeFile-uReadPos)) ?
        (BUFREADCOMMENT+4) : (uSizeFile-uReadPos);
    if (!fin->SeekSet(uReadPos)) break;
    if (fin->ReadRaw(buf,(uInt)uReadSize)!=uReadSize)
      break;

    for (i=(int)uReadSize-3; (i--)>0;)
      if (((*(buf+i))==0x50) && ((*(buf+i+1))==0x4b) &&
          ((*(buf+i+2))==0x05) && ((*(buf+i+3))==0x06)) {
        uPosFound = uReadPos+i;
        break;
      }

    if (uPosFound!=0) break;
  }
  TRYFREE(buf);
  return uPosFound;
}

/*
  Open a Zip file. path contain the full pathname (by example,
  on a Windows NT computer "c:\\test\\zlib109.zip" or on an Unix computer
  "zlib/zlib109.zip".
  If the zipfile cannot be opened (file don't exist or in not valid), the
  return value is NULL.
  Else, the return value is a unzFile Handle, usable with other function
  of this unzip package.
*/
niExportFunc(unzFile) niUnzip_Open(ni::iFile* fin) {
  unz_s us;
  memset(&us,0,sizeof(us));
  unz_s *s = NULL;
  unsigned long central_pos = 0,uL = 0;
  unsigned long number_disk = 0;          /* number of the current dist, used for
                                             spaning ZIP, unsupported, always 0*/
  unsigned long number_disk_with_CD = 0;  /* number the the disk with central dir, used
                                             for spaning ZIP, unsupported, always 0*/
  unsigned long number_entry_CD = 0;      /* total number of entries in
                                             the central dir
                                             (same than number_entry on nospan) */

  int err=UNZ_OK;
  fin->AddRef();

  central_pos = unzlocal_SearchCentralDir(fin);
  if (!central_pos) {
    niError("Couldn't find central directory.");
    err=UNZ_ERRNO;
  }

  if (!fin->SeekSet(central_pos)) {
    niError("Couldn't seek to central directory.");
    err=UNZ_ERRNO;
  }

  /* the signature, already checked */
  if (unzlocal_getLong(fin,&uL)!=UNZ_OK) {
    niError("Couldn't get the signature.");
    err=UNZ_ERRNO;
  }

  /* number of this disk */
  if (unzlocal_getShort(fin,&number_disk)!=UNZ_OK) {
    niError("Couldn't get disk number.");
    err=UNZ_ERRNO;
  }

  /* number of the disk with the start of the central directory */
  if (unzlocal_getShort(fin,&number_disk_with_CD)!=UNZ_OK) {
    niError("Couldn't get disk number with central directory.");
    err=UNZ_ERRNO;
  }

  /* total number of entries in the central dir on this disk */
  if (unzlocal_getShort(fin,&us.gi.number_entry)!=UNZ_OK) {
    niError("Couldn't get total number of entries in central directory on disk.");
    err=UNZ_ERRNO;
  }

  /* total number of entries in the central dir */
  if (unzlocal_getShort(fin,&number_entry_CD)!=UNZ_OK) {
    niError("Couldn't get total number of entries in central directory.");
    err=UNZ_ERRNO;
  }

  if ((number_entry_CD!=us.gi.number_entry) || (number_disk_with_CD!=0) || (number_disk!=0)) {
    niError("Invalid zip file.");
    err=UNZ_BADZIPFILE;
  }

  /* size of the central directory */
  if (unzlocal_getLong(fin,&us.size_central_dir)!=UNZ_OK) {
    niError("Couldn't get the central directory size.");
    err=UNZ_ERRNO;
  }

  /* offset of start of central directory with respect to the
     starting disk number */
  if (unzlocal_getLong(fin,&us.offset_central_dir)!=UNZ_OK) {
    niError("Couldn't get offset of central directory.");
    err=UNZ_ERRNO;
  }

  /* zipfile comment length */
  if (unzlocal_getShort(fin,&us.gi.size_comment)!=UNZ_OK) {
    niError("Couldn't get commend length.");
    err=UNZ_ERRNO;
  }

  if ((central_pos<us.offset_central_dir+us.size_central_dir) && (err==UNZ_OK)) {
    niError("Invalid zip file central directory.");
    err=UNZ_BADZIPFILE;
  }

  if (err!=UNZ_OK) {
    fin->Release();
    return NULL;
  }

  us.file=fin;
  us.byte_before_the_zipfile = central_pos - (us.offset_central_dir+us.size_central_dir);
  us.central_pos = central_pos;
  us.pfile_in_zip_read = NULL;

  s=(unz_s*)ALLOC(sizeof(unz_s));
  *s=us;
  niUnzip_GoToFirstFile((unzFile)s);
  return (unzFile)s;
}


/*
  Close a ZipFile opened with unzipOpen.
  If there is files inside the .Zip opened with unzipOpenCurrentFile (see later),
  these files MUST be closed with unzipCloseCurrentFile before call unzipClose.
  return UNZ_OK if there is no problem. */
niExportFunc(int) niUnzip_Close (unzFile file) {
  unz_s* s;
  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;

  if (s->pfile_in_zip_read) {
    niUnzip_CloseCurrentFile(file);
  }

  niSafeRelease(s->file);
  TRYFREE(s);
  return UNZ_OK;
}


/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem. */
niExportFunc(int) niUnzip_GetGlobalInfo (unzFile file, unz_global_info *pglobal_info) {
  unz_s* s;
  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  *pglobal_info=s->gi;
  return UNZ_OK;
}


/*
  Get Info about the current file in the zipfile, with internal only info
*/
local int unzlocal_GetCurrentFileInfoInternal OF((unzFile file,
                                                  unz_file_info *pfile_info,
                                                  unz_file_info_internal
                                                  *pfile_info_internal,
                                                  char *szFileName,
                                                  unsigned long fileNameBufferSize,
                                                  void *extraField,
                                                  unsigned long extraFieldBufferSize,
                                                  char *szComment,
                                                  unsigned long commentBufferSize));

local int unzlocal_GetCurrentFileInfoInternal (unzFile file,
                                               unz_file_info *pfile_info,
                                               unz_file_info_internal *pfile_info_internal,
                                               char *szFileName, unsigned long fileNameBufferSize,
                                               void *extraField, unsigned long extraFieldBufferSize,
                                               char *szComment,  unsigned long commentBufferSize) {
  unz_s* s;
  unz_file_info file_info;
  unz_file_info_internal file_info_internal;
  int err=UNZ_OK;
  unsigned long uMagic = 0;
  long lSeek=0;

  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  if (!s->file->SeekSet(s->pos_in_central_dir+s->byte_before_the_zipfile))
    err=UNZ_ERRNO;

  /* we check the magic */
  if (err == UNZ_OK) {
    if (unzlocal_getLong(s->file,&uMagic) != UNZ_OK)
      err=UNZ_ERRNO;
    else if (uMagic!=0x02014b50)
      err=UNZ_BADZIPFILE;
  }

  s->file->ReadRaw(&file_info, sizeof(file_info)-2*4); // 2*4 is the size of 2 my vars

  file_info_internal.offset_curfile = 0;
  if (unzlocal_getLong(s->file,&file_info_internal.offset_curfile) != UNZ_OK)
    err=UNZ_ERRNO;

  file_info.offset = file_info_internal.offset_curfile;
  file_info.c_offset = s->pos_in_central_dir;

  lSeek+=file_info.size_filename;
  if ((err==UNZ_OK) && (szFileName)) {
    unsigned long uSizeRead ;
    if (file_info.size_filename<fileNameBufferSize) {
      *(szFileName+file_info.size_filename)='\0';
      uSizeRead = file_info.size_filename;
    } else uSizeRead = fileNameBufferSize;

    if ((file_info.size_filename>0) && (fileNameBufferSize>0))
      if (s->file->ReadRaw(szFileName,(uInt)uSizeRead)!=uSizeRead)
        err=UNZ_ERRNO;
    lSeek -= uSizeRead;
  }


  if ((err==UNZ_OK) && (extraField)) {
    unsigned long uSizeRead ;
    if (file_info.size_file_extra<extraFieldBufferSize)
      uSizeRead = file_info.size_file_extra;
    else
      uSizeRead = extraFieldBufferSize;

    if (lSeek!=0) {
      if (s->file->Seek(lSeek))
        lSeek=0;
      else
        err=UNZ_ERRNO;
    }
    if ((file_info.size_file_extra>0) && (extraFieldBufferSize>0))
      if (s->file->ReadRaw(extraField,(uInt)uSizeRead)!=uSizeRead)
        err=UNZ_ERRNO;
    lSeek += file_info.size_file_extra - uSizeRead;
  }
  else {
    lSeek+=file_info.size_file_extra;
  }

  if ((err==UNZ_OK) && (szComment)) {
    unsigned long uSizeRead ;
    if (file_info.size_file_comment<commentBufferSize) {
      *(szComment+file_info.size_file_comment)='\0';
      uSizeRead = file_info.size_file_comment;
    } else uSizeRead = commentBufferSize;

    if (lSeek!=0) {
      if (s->file->Seek(lSeek)) lSeek=0;
      else err=UNZ_ERRNO;
    }
    if ((file_info.size_file_comment>0) && (commentBufferSize>0)) {
      if (s->file->ReadRaw(szComment,(uInt)uSizeRead) != uSizeRead)
        err=UNZ_ERRNO;
    }
    lSeek+=file_info.size_file_comment - uSizeRead;
  }
  else {
    lSeek+=file_info.size_file_comment;
  }
  niUnused(lSeek);

  if ((err==UNZ_OK) && (pfile_info))
    *pfile_info=file_info;

  if ((err==UNZ_OK) && (pfile_info_internal))
    *pfile_info_internal=file_info_internal;

  return err;
}



/*
  Write info about the ZipFile in the *pglobal_info structure.
  No preparation of the structure is needed
  return UNZ_OK if there is no problem.
*/
niExportFunc(int) niUnzip_GetCurrentFileInfo (unzFile file,
                                  unz_file_info *pfile_info,
                                  char *szFileName, unsigned long fileNameBufferSize,
                                  void *extraField, unsigned long extraFieldBufferSize,
                                  char *szComment,  unsigned long commentBufferSize) {
  return unzlocal_GetCurrentFileInfoInternal(file,pfile_info,NULL,
                                             szFileName,fileNameBufferSize,
                                             extraField,extraFieldBufferSize,
                                             szComment,commentBufferSize);
}

/*
  Set the current file of the zipfile to the first file.
  return UNZ_OK if there is no problem
*/
niExportFunc(int) niUnzip_GoToFirstFile (unzFile file) {
  int err=UNZ_OK;
  unz_s* s;
  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  s->pos_in_central_dir=s->offset_central_dir;
  s->num_file=0;
  err=unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                          &s->cur_file_info_internal,
                                          NULL,0,NULL,0,NULL,0);
  s->current_file_ok = (err == UNZ_OK);
  return err;
}


/*
  Set the current file of the zipfile to the next file.
  return UNZ_OK if there is no problem
  return UNZ_END_OF_LIST_OF_FILE if the actual file was the latest.
*/
niExportFunc(int) niUnzip_GoToNextFile (unzFile file) {
  unz_s* s;
  int err;

  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  if (!s->current_file_ok) return UNZ_END_OF_LIST_OF_FILE;
  if (s->num_file+1==s->gi.number_entry) return UNZ_END_OF_LIST_OF_FILE;

  s->pos_in_central_dir += SIZECENTRALDIRITEM + s->cur_file_info.size_filename +
      s->cur_file_info.size_file_extra + s->cur_file_info.size_file_comment ;
  s->num_file++;
  err = unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,
                                            &s->cur_file_info_internal,
                                            NULL,0,NULL,0,NULL,0);
  s->current_file_ok = (err == UNZ_OK);
  return err;
}

/*
  Compare two filename (fileName1,fileName2).
  If iCaseSenisivity = 1, comparision is case sensitivity (like strcmp)
  If iCaseSenisivity = 2, comparision is not case sensitivity (like strcmpi
  or strcasecmp)
  If iCaseSenisivity = 0, case sensitivity is defaut of your operating system
  (like 1 on Unix, 2 on Windows)

*/
niExportFunc(int) niUnzip_StringFileNameCompare (const char* fileName1,const char* fileName2,int iCaseSensitivity)
{
  if (iCaseSensitivity==1)
    return StrCmp(fileName1,fileName2);
  return StrICmp(fileName1,fileName2);
}


/*
  Try locate the file szFileName in the zipfile.
  For the iCaseSensitivity signification, see unzipStringFileNameCompare

  return value :
  UNZ_OK if the file is found. It becomes the current file.
  UNZ_END_OF_LIST_OF_FILE if the file is not found
*/
niExportFunc(int) niUnzip_LocateFile (unzFile file, const char *szFileName, int iCaseSensitivity)
{
  unz_s* s;
  int err;


  unsigned long num_fileSaved;
  unsigned long pos_in_central_dirSaved;


  if (file==NULL)
    return UNZ_PARAMERROR;

  if (strlen(szFileName)>=UNZ_MAXFILENAMEINZIP)
    return UNZ_PARAMERROR;

  s=(unz_s*)file;
  if (!s->current_file_ok)
    return UNZ_END_OF_LIST_OF_FILE;

  num_fileSaved = s->num_file;
  pos_in_central_dirSaved = s->pos_in_central_dir;

  err = niUnzip_GoToFirstFile(file);

  while (err == UNZ_OK)
  {
    char szCurrentFileName[UNZ_MAXFILENAMEINZIP+1];
    niUnzip_GetCurrentFileInfo(file,NULL,
                               szCurrentFileName,sizeof(szCurrentFileName)-1,
                               NULL,0,NULL,0);
    if (niUnzip_StringFileNameCompare(szCurrentFileName,
                                      szFileName,iCaseSensitivity)==0)
      return UNZ_OK;
    err = niUnzip_GoToNextFile(file);
  }

  s->num_file = num_fileSaved ;
  s->pos_in_central_dir = pos_in_central_dirSaved ;
  return err;
}


niExportFunc(int) niUnzip_LocateFileMy (unzFile file, unsigned long num, unsigned long pos) {
  unz_s* s;
  s = (unz_s *)file;
  s->pos_in_central_dir = pos;
  s->num_file = num;
  unzlocal_GetCurrentFileInfoInternal(file,&s->cur_file_info,&s->cur_file_info_internal,NULL,0,NULL,0,NULL,0);
  return 1;
}


/*
  Read the local header of the current zipfile
  Check the coherency of the local header and info in the end of central
  directory about this file
  store in *piSizeVar the size of extra info in local header
  (filename and size of extra field data)
*/
local int unzlocal_CheckCurrentFileCoherencyHeader (unz_s *s, uInt *piSizeVar,
                                                    unsigned long *poffset_local_extrafield,
                                                    uInt *psize_local_extrafield) {
  unsigned long uMagic = 0,uData = 0,uFlags = 0;
  unsigned long size_filename = 0;
  unsigned long size_extra_field = 0;
  int err=UNZ_OK;

  *piSizeVar = 0;
  *poffset_local_extrafield = 0;
  *psize_local_extrafield = 0;

  if (!s->file->SeekSet(s->cur_file_info_internal.offset_curfile + s->byte_before_the_zipfile))
    return UNZ_ERRNO;

  if (err==UNZ_OK) {
    if (unzlocal_getLong(s->file,&uMagic) != UNZ_OK) err=UNZ_ERRNO;
    else if (uMagic!=0x04034b50) err=UNZ_BADZIPFILE;
  }

  if (unzlocal_getShort(s->file,&uData) != UNZ_OK) err=UNZ_ERRNO;

  if (unzlocal_getShort(s->file,&uFlags) != UNZ_OK)
    err=UNZ_ERRNO;

  if (unzlocal_getShort(s->file,&uData) != UNZ_OK) err=UNZ_ERRNO;
  else if ((err==UNZ_OK) && (uData!=s->cur_file_info.compression_method)) err=UNZ_BADZIPFILE;

  if ((err==UNZ_OK) && (s->cur_file_info.compression_method!=0) && (s->cur_file_info.compression_method!=Z_DEFLATED)) err=UNZ_BADZIPFILE;

  /* date/time */
  if (unzlocal_getLong(s->file,&uData) != UNZ_OK) err=UNZ_ERRNO;

  /* crc */
  if (unzlocal_getLong(s->file,&uData) != UNZ_OK) err=UNZ_ERRNO;
  else if ((err==UNZ_OK) && (uData!=s->cur_file_info.crc) && ((uFlags & 8)==0)) err=UNZ_BADZIPFILE;

  /* size compr */
  if (unzlocal_getLong(s->file,&uData) != UNZ_OK) err=UNZ_ERRNO;
  else if ((err==UNZ_OK) && (uData!=s->cur_file_info.compressed_size) && ((uFlags & 8)==0)) err=UNZ_BADZIPFILE;

  /* size uncompr */
  if (unzlocal_getLong(s->file,&uData) != UNZ_OK) err=UNZ_ERRNO;
  else if ((err==UNZ_OK) && (uData!=s->cur_file_info.uncompressed_size) && ((uFlags & 8)==0)) err=UNZ_BADZIPFILE;


  if (unzlocal_getShort(s->file,&size_filename) != UNZ_OK) err=UNZ_ERRNO;
  else if ((err==UNZ_OK) && (size_filename!=s->cur_file_info.size_filename)) err=UNZ_BADZIPFILE;

  *piSizeVar += (uInt)size_filename;

  if (unzlocal_getShort(s->file,&size_extra_field) != UNZ_OK) err=UNZ_ERRNO;
  *poffset_local_extrafield= s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + size_filename;
  *psize_local_extrafield = (uInt)size_extra_field;

  *piSizeVar += (uInt)size_extra_field;

  return err;
}

/*
  Open for reading data the current file in the zipfile.
  If there is no error and the file is opened, the return value is UNZ_OK.
*/
niExportFunc(int) niUnzip_OpenCurrentFile (unzFile file, const char* password) {
  int err=UNZ_OK;
  int Store;
  uInt iSizeVar;
  unz_s* s;
  file_in_zip_read_info_s* pfile_in_zip_read_info;
  unsigned long offset_local_extrafield;  /* offset of the local extra field */
  uInt  size_local_extrafield;    /* size of the local extra field */

  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  if (!s->current_file_ok) return UNZ_PARAMERROR;

  if (s->pfile_in_zip_read) niUnzip_CloseCurrentFile(file);

  if (unzlocal_CheckCurrentFileCoherencyHeader(s,&iSizeVar, &offset_local_extrafield,&size_local_extrafield)!=UNZ_OK) return UNZ_BADZIPFILE;

  pfile_in_zip_read_info = (file_in_zip_read_info_s*)ALLOC(sizeof(file_in_zip_read_info_s));
  if (!pfile_in_zip_read_info) return UNZ_INTERNALERROR;

  pfile_in_zip_read_info->read_buffer=(char*)ALLOC(UNZ_BUFSIZE);
  pfile_in_zip_read_info->offset_local_extrafield = offset_local_extrafield;
  pfile_in_zip_read_info->size_local_extrafield = size_local_extrafield;
  pfile_in_zip_read_info->pos_local_extrafield=0;

  if (!pfile_in_zip_read_info->read_buffer) {
    TRYFREE(pfile_in_zip_read_info);
    return UNZ_INTERNALERROR;
  }

  pfile_in_zip_read_info->stream_initialised=0;

  if ((s->cur_file_info.compression_method!=0) && (s->cur_file_info.compression_method!=Z_DEFLATED)) err=UNZ_BADZIPFILE;
  Store = s->cur_file_info.compression_method==0;

  pfile_in_zip_read_info->crc32_wait=s->cur_file_info.crc;
  pfile_in_zip_read_info->crc32=0;
  pfile_in_zip_read_info->compression_method = s->cur_file_info.compression_method;
  pfile_in_zip_read_info->file=s->file;
  pfile_in_zip_read_info->byte_before_the_zipfile=s->byte_before_the_zipfile;

  pfile_in_zip_read_info->stream.total_out = 0;

  if (!Store) {
    pfile_in_zip_read_info->stream.zalloc = (zlib_alloc_func)0;
    pfile_in_zip_read_info->stream.zfree = (zlib_free_func)0;
    pfile_in_zip_read_info->stream.opaque = (voidpf)0;

    err=inflateInit2(&pfile_in_zip_read_info->stream, -MAX_WBITS);
    if (err == Z_OK) pfile_in_zip_read_info->stream_initialised=1;
    /* windowBits is passed < 0 to tell that there is no zlib header.
     * Note that in this case inflate *requires* an extra "dummy" byte
     * after the compressed stream in order to complete decompression and
     * return Z_STREAM_END.
     * In unzip, i don't wait absolutely Z_STREAM_END because I known the
     * size of both compressed and uncompressed data
     */
  }
  pfile_in_zip_read_info->rest_read_compressed = s->cur_file_info.compressed_size ;
  pfile_in_zip_read_info->rest_read_uncompressed = s->cur_file_info.uncompressed_size ;

  pfile_in_zip_read_info->encrypted = (s->cur_file_info.flag&1)!=0;
  bool extlochead = (s->cur_file_info.flag&8)!=0;
  if (extlochead) pfile_in_zip_read_info->crcenctest = (char)((s->cur_file_info.dosDate>>8)&0xff);
  else pfile_in_zip_read_info->crcenctest = (char)(s->cur_file_info.crc >> 24);
  pfile_in_zip_read_info->encheadleft = (pfile_in_zip_read_info->encrypted?12:0);
  pfile_in_zip_read_info->keys[0] = 305419896L;
  pfile_in_zip_read_info->keys[1] = 591751049L;
  pfile_in_zip_read_info->keys[2] = 878082192L;
  for (const char *cp=password; cp!=0 && *cp!=0; cp++)
    Uupdate_keys(pfile_in_zip_read_info->keys,*cp);

  pfile_in_zip_read_info->pos_in_zipfile = s->cur_file_info_internal.offset_curfile + SIZEZIPLOCALHEADER + iSizeVar;

  pfile_in_zip_read_info->stream.avail_in = (uInt)0;


  s->pfile_in_zip_read = pfile_in_zip_read_info;
  return UNZ_OK;
}


/*
  Read bytes from the current file.
  buf contain buffer where data must be copied
  len the size of buf.

  return the number of byte copied if somes bytes are copied
  return 0 if the end of file was reached
  return <0 with error code if there is an error
  (UNZ_ERRNO for IO error, or zLib error for uncompress error)
*/
niExportFunc(int) niUnzip_ReadCurrentFile  (unzFile file, voidp buf, unsigned len) {
  int err=UNZ_OK;
  uInt iRead = 0;
  unz_s* s;
  file_in_zip_read_info_s* pfile_in_zip_read_info;
  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  pfile_in_zip_read_info=s->pfile_in_zip_read;

  if (!pfile_in_zip_read_info) return UNZ_PARAMERROR;


  if ((!pfile_in_zip_read_info->read_buffer)) return UNZ_END_OF_LIST_OF_FILE;
  if (!len) return 0;

  pfile_in_zip_read_info->stream.next_out = (Bytef*)buf;

  pfile_in_zip_read_info->stream.avail_out = (uInt)len;

  if (len>pfile_in_zip_read_info->rest_read_uncompressed)
    pfile_in_zip_read_info->stream.avail_out = (uInt)pfile_in_zip_read_info->rest_read_uncompressed;

  while (pfile_in_zip_read_info->stream.avail_out>0) {
    if ((pfile_in_zip_read_info->stream.avail_in==0) &&
        (pfile_in_zip_read_info->rest_read_compressed>0))
    {
      uInt uReadThis = UNZ_BUFSIZE;
      if (pfile_in_zip_read_info->rest_read_compressed<uReadThis) uReadThis = (uInt)pfile_in_zip_read_info->rest_read_compressed;
      if (!uReadThis) return UNZ_EOF;
      if (!pfile_in_zip_read_info->file->SeekSet(
              pfile_in_zip_read_info->pos_in_zipfile +
              pfile_in_zip_read_info->byte_before_the_zipfile))
        return UNZ_ERRNO;
      if (pfile_in_zip_read_info->file->ReadRaw(pfile_in_zip_read_info->read_buffer,uReadThis)!=uReadThis)
        return UNZ_ERRNO;
      pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

      pfile_in_zip_read_info->rest_read_compressed-=uReadThis;

      pfile_in_zip_read_info->stream.next_in =
          (Bytef*)pfile_in_zip_read_info->read_buffer;
      pfile_in_zip_read_info->stream.avail_in = (uInt)uReadThis;

      //
      if (pfile_in_zip_read_info->encrypted)
      {
        char *streamInBuf = (char*)pfile_in_zip_read_info->stream.next_in;
        for (unsigned int i = 0; i < uReadThis; ++i) {
          streamInBuf[i] = zdecode(pfile_in_zip_read_info->keys,streamInBuf[i]);
        }
      }
    }

    unsigned int uDoEncHead = pfile_in_zip_read_info->encheadleft;
    if ((int)uDoEncHead>pfile_in_zip_read_info->stream.avail_in)
      uDoEncHead=pfile_in_zip_read_info->stream.avail_in;
    if (uDoEncHead>0)
    {
      char bufcrc=pfile_in_zip_read_info->stream.next_in[uDoEncHead-1];
      pfile_in_zip_read_info->rest_read_uncompressed-=uDoEncHead;
      pfile_in_zip_read_info->stream.avail_in -= uDoEncHead;
      pfile_in_zip_read_info->stream.next_in += uDoEncHead;
      pfile_in_zip_read_info->encheadleft -= uDoEncHead;
      if (pfile_in_zip_read_info->encheadleft==0) {
        if (bufcrc!=pfile_in_zip_read_info->crcenctest)
          return UNZ_PASSWORD;
      }
    }

    if (pfile_in_zip_read_info->compression_method==0) {
      uInt uDoCopy,i ;
      if (pfile_in_zip_read_info->stream.avail_out < pfile_in_zip_read_info->stream.avail_in) uDoCopy = pfile_in_zip_read_info->stream.avail_out ;
      else uDoCopy = pfile_in_zip_read_info->stream.avail_in ;

      for (i=0;i<uDoCopy;i++)
        *(pfile_in_zip_read_info->stream.next_out+i) = *(pfile_in_zip_read_info->stream.next_in+i);

      pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32, pfile_in_zip_read_info->stream.next_out, uDoCopy);
      pfile_in_zip_read_info->rest_read_uncompressed-=uDoCopy;
      pfile_in_zip_read_info->stream.avail_in -= uDoCopy;
      pfile_in_zip_read_info->stream.avail_out -= uDoCopy;
      pfile_in_zip_read_info->stream.next_out += uDoCopy;
      pfile_in_zip_read_info->stream.next_in += uDoCopy;
      pfile_in_zip_read_info->stream.total_out += uDoCopy;
      iRead += uDoCopy;
    } else {
      unsigned long uTotalOutBefore,uTotalOutAfter;
      const Bytef *bufBefore;
      unsigned long uOutThis;
      int flush=Z_SYNC_FLUSH;

      uTotalOutBefore = pfile_in_zip_read_info->stream.total_out;
      bufBefore = pfile_in_zip_read_info->stream.next_out;

      err=inflate(&pfile_in_zip_read_info->stream,flush);

      uTotalOutAfter = pfile_in_zip_read_info->stream.total_out;
      uOutThis = uTotalOutAfter-uTotalOutBefore;

      pfile_in_zip_read_info->crc32 = crc32(pfile_in_zip_read_info->crc32,bufBefore, (uInt)(uOutThis));

      pfile_in_zip_read_info->rest_read_uncompressed -= uOutThis;

      iRead += (uInt)(uTotalOutAfter - uTotalOutBefore);

      if (err==Z_STREAM_END) return (iRead==0) ? UNZ_EOF : iRead;
      if (err!=Z_OK) break;
    }
  }

  if (err==Z_OK) return iRead;
  return err;
}


/*
  Give the current position in uncompressed data
*/
niExportFunc(z_off_t) niUnzip_tell (unzFile file) {
  unz_s* s;
  file_in_zip_read_info_s* pfile_in_zip_read_info;
  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  pfile_in_zip_read_info=s->pfile_in_zip_read;

  if (!pfile_in_zip_read_info) return UNZ_PARAMERROR;

  return (z_off_t)pfile_in_zip_read_info->stream.total_out;
}


/*
  return 1 if the end of file was reached, 0 elsewhere
*/
niExportFunc(int) niUnzip_eof (unzFile file) {
  unz_s* s;
  file_in_zip_read_info_s* pfile_in_zip_read_info;
  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  pfile_in_zip_read_info=s->pfile_in_zip_read;

  if (!pfile_in_zip_read_info) return UNZ_PARAMERROR;

  if (pfile_in_zip_read_info->rest_read_uncompressed == 0) return 1;
  else return 0;
}



/*
  Read extra field from the current file (opened by unzOpenCurrentFile)
  This is the local-header version of the extra field (sometimes, there is
  more info in the local-header version than in the central-header)

  if buf==NULL, it return the size of the local extra field that can be read

  if buf!=NULL, len is the size of the buffer, the extra header is copied in
  buf.
  the return value is the number of bytes copied in buf, or (if <0)
  the error code
*/
niExportFunc(int) niUnzip_GetLocalExtrafield (unzFile file,voidp buf,unsigned len) {
  unz_s* s;
  file_in_zip_read_info_s* pfile_in_zip_read_info;
  uInt read_now;
  unsigned long size_to_read;

  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  pfile_in_zip_read_info=s->pfile_in_zip_read;

  if (!pfile_in_zip_read_info) return UNZ_PARAMERROR;

  size_to_read = (pfile_in_zip_read_info->size_local_extrafield - pfile_in_zip_read_info->pos_local_extrafield);

  if (!buf) return (int)size_to_read;

  if (len>size_to_read) read_now = (uInt)size_to_read;
  else read_now = (uInt)len ;

  if (!read_now) return 0;

  if (pfile_in_zip_read_info->file->SeekSet(
          pfile_in_zip_read_info->offset_local_extrafield +
          pfile_in_zip_read_info->pos_local_extrafield)!=0)
    return UNZ_ERRNO;

  if (pfile_in_zip_read_info->file->ReadRaw(buf,(uInt)size_to_read)!=size_to_read)
    return UNZ_ERRNO;

  return (int)read_now;
}

/*
  Close the file in zip opened with unzipOpenCurrentFile
  Return UNZ_CRCERROR if all the file was read but the CRC is not good
*/
niExportFunc(int) niUnzip_CloseCurrentFile (unzFile file) {
  int err=UNZ_OK;

  unz_s* s;
  file_in_zip_read_info_s* pfile_in_zip_read_info;
  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;
  pfile_in_zip_read_info=s->pfile_in_zip_read;

  if (!pfile_in_zip_read_info) return UNZ_PARAMERROR;


  if (!pfile_in_zip_read_info->rest_read_uncompressed) {
    if (pfile_in_zip_read_info->crc32 != pfile_in_zip_read_info->crc32_wait) err=UNZ_CRCERROR;
  }


  TRYFREE(pfile_in_zip_read_info->read_buffer);
  pfile_in_zip_read_info->read_buffer = NULL;
  if (pfile_in_zip_read_info->stream_initialised) inflateEnd(&pfile_in_zip_read_info->stream);

  pfile_in_zip_read_info->stream_initialised = 0;
  TRYFREE(pfile_in_zip_read_info);

  s->pfile_in_zip_read=NULL;

  return err;
}


/*
  Get the global comment string of the ZipFile, in the szComment buffer.
  uSizeBuf is the size of the szComment buffer.
  return the number of byte copied or an error code <0
*/
niExportFunc(int) niUnzip_GetGlobalComment (unzFile file, char *szComment, unsigned long uSizeBuf) {
  //  int err=UNZ_OK;
  unz_s* s;
  unsigned long uReadThis ;
  if (!file) return UNZ_PARAMERROR;
  s=(unz_s*)file;

  uReadThis = uSizeBuf;
  if (uReadThis>s->gi.size_comment) uReadThis = s->gi.size_comment;

  if (!s->file->SeekSet(s->central_pos+22))
    return UNZ_ERRNO;

  if (uReadThis>0) {
    *szComment='\0';
    if (s->file->ReadRaw(szComment,(uInt)uReadThis)!=uReadThis)
      return UNZ_ERRNO;
  }

  if ((szComment != NULL) && (uSizeBuf > s->gi.size_comment)) *(szComment+s->gi.size_comment)='\0';
  return (int)uReadThis;
}
