/* gzio.c -- IO on .gz files
 * Copyright (C) 1995-1998 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 *
 * Compile this file with -DNO_DEFLATE to avoid the compression code.
 */

/* @(#) $Id: gzio.cpp,v 1.1.1.1 2003/10/31 14:45:30 Pierre Renaux Exp $ */

#include <stdio.h>
#include "API/niLang/Types.h"
#include "API/niLang/IFile.h"
#include "API/niLang/IFileSystem.h"
#include "API/niLang/StringDef.h"
#include "zlib/zlib.h"
#include "zlib/zutil.h"
#include "Zip_Gzio.h"

using namespace ni;

//struct zlib_internal_state {int dummy;}; /* for buggy compilers */

#ifndef Z_BUFSIZE
#  ifdef MAXSEG_64K
#    define Z_BUFSIZE 4096 /* minimize memory usage for 16-bit DOS */
#  else
#    define Z_BUFSIZE 16384
#  endif
#endif
#ifndef Z_PRINTF_BUFSIZE
#  define Z_PRINTF_BUFSIZE 4096
#endif

#define ALLOC(size) niMalloc(size)
#define TRYFREE(p) {if (p) niFree((tPtr)p);}

#define A25ZMODE  niBit(31)

static constexpr unsigned char gz_magic_std[2] = {0x1f, 0x8b}; /* gzip magic header */
static constexpr unsigned char gz_magic_a25[4] = {'A', '2', '5', 'Z'}; /* gzip magic header */

/* gzip flag byte */
// #define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

typedef struct gz_stream {
  z_stream stream;
  int      z_err;   /* error code for last stream operation */
  int      z_eof;   /* set if end of input file */
  iFileBase* file;   /* .gz file */
  Byte     *inbuf;  /* input buffer */
  Byte     *outbuf; /* output buffer */
  uLong    crc;     /* crc32 of uncompressed data */
  char     *msg;    /* error message */
  int      transparent; /* 1 if input file is not a .gz file */
  int      mode;    /* 'w' or 'r' */
  long     startpos; /* start of compressed data in file (header skipped) */
} gz_stream;


local gzFile gz_open      OF((const char *mode, iFileBase*  apFile, bool abA25Z));
local int do_flush        OF((gzFile file, int flush));
local int    get_byte     OF((gz_stream *s));
local void   check_header OF((gz_stream *s));
local int    destroy      OF((gz_stream *s));
local void   putLong      OF((iFileBase* file, uLong x));
local uLong  getLong      OF((gz_stream *s));

/* ===========================================================================
   Opens a gzip (.gz) file for reading or writing. The mode parameter
   is as in fopen ("rb" or "wb"). The file is given either by file descriptor
   or path name (if fd == -1).
   gz_open return NULL if the file could not be opened or if there was
   insufficient memory to allocate the (de)compression state; errno
   can be checked to distinguish the two cases (if errno is zero, the
   zlib error is Z_MEM_ERROR).
*/
local gzFile gz_open (const char *mode, iFileBase*  apFile, bool abA25Z)
{
  int err;
  int level = Z_DEFAULT_COMPRESSION; /* compression level */
  int strategy = Z_DEFAULT_STRATEGY; /* compression strategy */
  char *p = (char*)mode;
  gz_stream *s;
  char fmode[80]; /* copy of mode, without the compression level */
  char *m = fmode;

  if (!apFile || !mode) return Z_NULL;


  s = (gz_stream *)ALLOC(sizeof(gz_stream));
  if (!s) return Z_NULL;

  s->stream.zalloc = (zlib_alloc_func)0;
  s->stream.zfree = (zlib_free_func)0;
  s->stream.opaque = (voidpf)0;
  s->stream.next_in = s->inbuf = Z_NULL;
  s->stream.next_out = s->outbuf = Z_NULL;
  s->stream.avail_in = s->stream.avail_out = 0;
  s->file = NULL;
  s->z_err = Z_OK;
  s->z_eof = 0;
  s->crc = crc32(0L, Z_NULL, 0);
  s->msg = NULL;
  s->transparent = 0;

  s->mode = (eFileOpenMode)eInvalidHandle;
  do {
    if (*p == 'r') s->mode = eFileOpenMode_Read;
    if (*p == 'w' || *p == 'a') s->mode = eFileOpenMode_Write;
    if (*p >= '0' && *p <= '9') {
      level = *p - '0';
    } else if (*p == 'f') {
      strategy = Z_FILTERED;
    } else if (*p == 'h') {
      strategy = Z_HUFFMAN_ONLY;
    } else {
      *m++ = *p; /* copy the mode */
    }
  } while (*p++ && m != fmode + sizeof(fmode));
  if (s->mode == (eFileOpenMode)eInvalidHandle)
    return destroy(s), (gzFile)Z_NULL;

  if (niFlagIs(s->mode,eFileOpenMode_Write)) {
#ifdef NO_DEFLATE
    err = Z_STREAM_ERROR;
#else
    err = deflateInit2(&(s->stream), level,
                       Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, strategy);
    /* windowBits is passed < 0 to suppress zlib header */

    s->stream.next_out = s->outbuf = (Byte*)ALLOC(Z_BUFSIZE);
#endif
    if (err != Z_OK || s->outbuf == Z_NULL) {
      return destroy(s), (gzFile)Z_NULL;
    }
  } else {
    s->stream.next_in  = s->inbuf = (Byte*)ALLOC(Z_BUFSIZE);

    err = inflateInit2(&(s->stream), -MAX_WBITS);
    /* windowBits is passed < 0 to tell that there is no zlib header.
     * Note that in this case inflate *requires* an extra "dummy" byte
     * after the compressed stream in order to complete decompression and
     * return Z_STREAM_END. Here the gzip CRC32 ensures that 4 bytes are
     * present after the compressed stream.
     */
    if (err != Z_OK || s->inbuf == Z_NULL) {
      return destroy(s), (gzFile)Z_NULL;
    }
  }
  s->stream.avail_out = Z_BUFSIZE;

  s->file = apFile;
  if (s->file == NULL) {
    return destroy(s), (gzFile)Z_NULL;
  }
  s->file->AddRef();

  if (abA25Z)
    s->mode |= A25ZMODE;

  if (niFlagIs(s->mode,eFileOpenMode_Write)) {
    if (abA25Z) {
      char header[12] = {
        gz_magic_a25[0], gz_magic_a25[1], gz_magic_a25[2], gz_magic_a25[3],
        Z_DEFLATED, 0 /*flags*/, 0,0,0,0 /*time*/, 0 /*xflags*/, OS_CODE
      };
      s->file->WriteRaw(header,12);
      s->startpos = 12L;
    }
    else {
      /* Write a very simple .gz header:
       */
      unsigned char header[10] = {
        gz_magic_std[0], gz_magic_std[1],
        Z_DEFLATED, 0 /*flags*/, 0,0,0,0 /*time*/, 0 /*xflags*/, OS_CODE
      };
      s->file->WriteRaw(header,10);
      s->startpos = 10L;
      /* We use 10L instead of ftell(s->file) to because ftell causes an
       * fflush on some systems. This version of the library doesn't use
       * startpos anyway in write mode, so this initialization is not
       * necessary.
       */
    }
  } else {
    check_header(s); /* skip the .a25z header */
    s->startpos = (s->file->Tell() - s->stream.avail_in);
  }

  return (gzFile)s;
}

/* ===========================================================================
   Associate a gzFile with the file descriptor fd. fd is not dup'ed here
   to mimic the behavio(u)r of fdopen.
*/
gzFile gzdopen (iFileBase* fd, const char *mode, bool abA25Z)
{
  if (!niIsOK(fd)) return (gzFile)Z_NULL;
  return gz_open(mode, fd, abA25Z);
}

/* ===========================================================================
 * Update the compression level and strategy
 */
int gzsetparams (gzFile file, int level, int strategy)
{
  gz_stream *s = (gz_stream*)file;

  if (s == NULL || niFlagIsNot(s->mode,eFileOpenMode_Write)) return Z_STREAM_ERROR;

  /* Make room to allow flushing */
  if (s->stream.avail_out == 0) {

    s->stream.next_out = s->outbuf;
    if (s->file->WriteRaw(s->outbuf, Z_BUFSIZE) != Z_BUFSIZE) {
      s->z_err = Z_ERRNO;
    }
    s->stream.avail_out = Z_BUFSIZE;
  }

  return deflateParams (&(s->stream), level, strategy);
}

/* ===========================================================================
   Read a byte from a gz_stream; update next_in and avail_in. Return EOF
   for end of file.
   IN assertion: the stream s has been sucessfully opened for reading.
*/
local int get_byte(gz_stream *s)
{
  if (s->z_eof) return EOF;
  if (s->stream.avail_in == 0) {
    s->stream.avail_in = s->file->ReadRaw(s->inbuf, Z_BUFSIZE);
    if (s->stream.avail_in == 0) {
      s->z_eof = 1;
      return EOF;
    }
    s->stream.next_in = s->inbuf;
  }
  s->stream.avail_in--;
  return *(s->stream.next_in)++;
}

/* ===========================================================================
   Check the gzip header of a gz_stream opened for reading. Set the stream
   mode to transparent if the gzip magic header is not present; set s->err
   to Z_DATA_ERROR if the magic header is present but the rest of the header
   is incorrect.
   IN assertion: the stream s has already been created sucessfully;
   s->stream.avail_in is zero for the first time, but may be non-zero
   for concatenated .gz files.
*/
local void check_header(gz_stream *s)
{
  int method; /* method byte */
  int flags;  /* flags byte */
  uInt len;
  int c;

  const unsigned char* gz_magic;
  int gz_magic_len;
  if (s->mode&A25ZMODE) {
    gz_magic = gz_magic_a25;
    gz_magic_len = 4;
  }
  else {
    gz_magic = gz_magic_std;
    gz_magic_len = 2;
  }

  /* Check the gzip magic header */
  for (len = 0; len < (uInt)gz_magic_len; len++) {
    c = get_byte(s);
    if (c != gz_magic[len]) {
      if (len != 0) s->stream.avail_in++, s->stream.next_in--;
      if (c != EOF) {
        s->stream.avail_in++, s->stream.next_in--;
        s->transparent = 1;
      }
      s->z_err = s->stream.avail_in != 0 ? Z_OK : Z_STREAM_END;
      return;
    }
  }
  method = get_byte(s);
  flags = get_byte(s);
  if (method != Z_DEFLATED || (flags & RESERVED) != 0) {
    s->z_err = Z_DATA_ERROR;
    return;
  }

  /* Discard time, xflags and OS code: */
  for (len = 0; len < 6; len++) (void)get_byte(s);

  if ((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
    len  =  (uInt)get_byte(s);
    len += ((uInt)get_byte(s))<<8;
    /* len is garbage if EOF but the loop below will quit anyway */
    while (len-- != 0 && get_byte(s) != EOF) ;
  }
  if ((flags & ORIG_NAME) != 0) { /* skip the original file name */
    while ((c = get_byte(s)) != 0 && c != EOF) ;
  }
  if ((flags & COMMENT) != 0) {   /* skip the .gz file comment */
    while ((c = get_byte(s)) != 0 && c != EOF) ;
  }
  if ((flags & HEAD_CRC) != 0) {  /* skip the header crc */
    for (len = 0; len < 2; len++) (void)get_byte(s);
  }
  s->z_err = s->z_eof ? Z_DATA_ERROR : Z_OK;
}

/* ===========================================================================
 * Cleanup then free the given gz_stream. Return a zlib error code.
 Try freeing in the reverse order of allocations.
*/
local int destroy (gz_stream *s)
{
  int err = Z_OK;

  if (!s) return Z_STREAM_ERROR;

  TRYFREE(s->msg);

  if (s->stream.state != NULL) {
    if (niFlagIs(s->mode,eFileOpenMode_Write)) {
#ifdef NO_DEFLATE
      err = Z_STREAM_ERROR;
#else
      err = deflateEnd(&(s->stream));
#endif
    } else if (niFlagIs(s->mode,eFileOpenMode_Read)) {
      err = inflateEnd(&(s->stream));
    }
  }
  if (s->file != NULL) {
    s->file->Release();
    err = Z_ERRNO;
  }
  if (s->z_err < 0) err = s->z_err;

  TRYFREE(s->inbuf);
  TRYFREE(s->outbuf);
  TRYFREE(s);
  return err;
}

/* ===========================================================================
   Reads the given number of uncompressed bytes from the compressed file.
   gzread returns the number of bytes actually read (0 for end of file).
*/
int gzread (gzFile file, voidp buf, unsigned len)
{
  gz_stream *s = (gz_stream*)file;
  Bytef *start = (Bytef*)buf; /* starting point for crc computation */
  Byte  *next_out; /* == stream.next_out but not forced far (for MSDOS) */

  if (s == NULL || niFlagIsNot(s->mode,eFileOpenMode_Read)) return Z_STREAM_ERROR;

  if (s->z_err == Z_DATA_ERROR || s->z_err == Z_ERRNO) return -1;
  if (s->z_err == Z_STREAM_END) return 0;  /* EOF */

  next_out = (Byte*)buf;
  s->stream.next_out = (Bytef*)buf;
  s->stream.avail_out = len;

  while (s->stream.avail_out != 0) {

    if (s->transparent) {
      /* Copy first the lookahead bytes: */
      uInt n = s->stream.avail_in;
      if (n > (uInt)s->stream.avail_out) n = s->stream.avail_out;
      if (n > 0) {
        zmemcpy(s->stream.next_out, s->stream.next_in, n);
        next_out += n;
        s->stream.next_out = next_out;
        s->stream.next_in   += n;
        s->stream.avail_out -= n;
        s->stream.avail_in  -= n;
      }
      if (s->stream.avail_out > 0) {
        s->stream.avail_out -= s->file->ReadRaw(next_out, s->stream.avail_out);
      }
      len -= s->stream.avail_out;
      s->stream.total_in  += (uLong)len;
      s->stream.total_out += (uLong)len;
      if (len == 0) s->z_eof = 1;
      return (int)len;
    }
    if (s->stream.avail_in == 0 && !s->z_eof) {
      s->stream.avail_in = s->file->ReadRaw(s->inbuf, Z_BUFSIZE);
      if (s->stream.avail_in == 0) {
        s->z_eof = 1;
      }
      s->stream.next_in = s->inbuf;
    }
    s->z_err = inflate(&(s->stream), Z_NO_FLUSH);

    if (s->z_err == Z_STREAM_END) {
      /* Check CRC and original size */
      s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));
      start = s->stream.next_out;

      if (getLong(s) != s->crc) {
        s->z_err = Z_DATA_ERROR;
      } else {
        (void)getLong(s);
        /* The uncompressed length returned by above getlong() may
         * be different from s->stream.total_out) in case of
         * concatenated .gz files. Check for such files:
         */
        check_header(s);
        if (s->z_err == Z_OK) {
          uLong total_in = s->stream.total_in;
          uLong total_out = s->stream.total_out;

          inflateReset(&(s->stream));
          s->stream.total_in = total_in;
          s->stream.total_out = total_out;
          s->crc = crc32(0L, Z_NULL, 0);
        }
      }
    }
    if (s->z_err != Z_OK || s->z_eof) break;
  }
  s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));

  return (int)(len - s->stream.avail_out);
}


/* ===========================================================================
   Reads one byte from the compressed file. gzgetc returns this byte
   or -1 in case of end of file or error.
*/
int gzgetc(gzFile file)
{
  unsigned char c;

  return gzread(file, &c, 1) == 1 ? c : -1;
}


/* ===========================================================================
   Reads bytes from the compressed file until len-1 characters are
   read, or a newline character is read and transferred to buf, or an
   end-of-file condition is encountered.  The string is then terminated
   with a null character.
   gzgets returns buf, or Z_NULL in case of error.

   The current implementation is not optimized at all.
*/
char * gzgets(gzFile file, char *buf, int len)
{
  char *b = buf;
  if (buf == Z_NULL || len <= 0) return Z_NULL;

  while (--len > 0 && gzread(file, buf, 1) == 1 && *buf++ != '\n') ;
  *buf = '\0';
  return b == buf && len > 0 ? Z_NULL : b;
}


#ifndef NO_DEFLATE
/* ===========================================================================
   Writes the given number of uncompressed bytes into the compressed file.
   gzwrite returns the number of bytes actually written (0 in case of error).
*/
int gzwrite (gzFile file, const voidp buf, unsigned len)
{
  gz_stream *s = (gz_stream*)file;

  if (s == NULL || niFlagIsNot(s->mode,eFileOpenMode_Write)) return Z_STREAM_ERROR;

  s->stream.next_in = (Bytef*)buf;
  s->stream.avail_in = len;

  while (s->stream.avail_in != 0) {

    if (s->stream.avail_out == 0) {

      s->stream.next_out = s->outbuf;
      if (s->file->WriteRaw(s->outbuf, Z_BUFSIZE) != Z_BUFSIZE) {
        s->z_err = Z_ERRNO;
        break;
      }
      s->stream.avail_out = Z_BUFSIZE;
    }
    s->z_err = deflate(&(s->stream), Z_NO_FLUSH);
    if (s->z_err != Z_OK) break;
  }
  s->crc = crc32(s->crc, (const Bytef *)buf, len);

  return (int)(len - s->stream.avail_in);
}

/* ===========================================================================
   Writes c, converted to an unsigned char, into the compressed file.
   gzputc returns the value that was written, or -1 in case of error.
*/
int gzputc(gzFile file, int c)
{
  unsigned char cc = (unsigned char) c; /* required for big endian systems */

  return gzwrite(file, &cc, 1) == 1 ? (int)cc : -1;
}


/* ===========================================================================
   Writes the given null-terminated string to the compressed file, excluding
   the terminating null character.
   gzputs returns the number of characters written, or -1 in case of error.
*/
int gzputs(gzFile file, const char *s)
{
  return gzwrite(file, (char*)s, (unsigned)strlen(s));
}


/* ===========================================================================
   Flushes all pending output into the compressed file. The parameter
   flush is as in the deflate() function.
*/
local int do_flush (gzFile file, int flush)
{
  uInt len;
  int done = 0;
  gz_stream *s = (gz_stream*)file;

  if (s == NULL || niFlagIsNot(s->mode,eFileOpenMode_Write)) return Z_STREAM_ERROR;

  s->stream.avail_in = 0; /* should be zero already anyway */

  for (;;) {
    len = Z_BUFSIZE - s->stream.avail_out;

    if (len != 0) {
      if ((uInt)s->file->WriteRaw(s->outbuf, len) != len) {
        s->z_err = Z_ERRNO;
        return Z_ERRNO;
      }
      s->stream.next_out = s->outbuf;
      s->stream.avail_out = Z_BUFSIZE;
    }
    if (done) break;
    s->z_err = deflate(&(s->stream), flush);

    /* Ignore the second of two consecutive flushes: */
    if (len == 0 && s->z_err == Z_BUF_ERROR) s->z_err = Z_OK;

    /* deflate has finished flushing only when it hasn't used up
     * all the available space in the output buffer:
     */
    done = (s->stream.avail_out != 0 || s->z_err == Z_STREAM_END);

    if (s->z_err != Z_OK && s->z_err != Z_STREAM_END) break;
  }
  return  s->z_err == Z_STREAM_END ? Z_OK : s->z_err;
}

int gzflush (gzFile file, int flush)
{
  gz_stream *s = (gz_stream*)file;
  int err = do_flush (file, flush);

  if (err) return err;
  s->file->Flush();
  return  s->z_err == Z_STREAM_END ? Z_OK : s->z_err;
}
#endif /* NO_DEFLATE */

/* ===========================================================================
   Sets the starting position for the next gzread or gzwrite on the given
   compressed file. The offset represents a number of bytes in the
   gzseek returns the resulting offset location as measured in bytes from
   the beginning of the uncompressed stream, or -1 in case of error.
   SEEK_END is not implemented, returns error.
   In this version of the library, gzseek can be extremely slow.
*/
z_off_t gzseek (gzFile file, z_off_t offset, int whence)
{
  gz_stream *s = (gz_stream*)file;

  if (s == NULL || whence == SEEK_END ||
      s->z_err == Z_ERRNO || s->z_err == Z_DATA_ERROR) {
    return -1L;
  }

  if (niFlagIs(s->mode,eFileOpenMode_Write)) {
#ifdef NO_DEFLATE
    return -1L;
#else
    if (whence == SEEK_SET) {
      offset -= s->stream.total_in;
    }
    if (offset < 0) return -1L;

    /* At this point, offset is the number of zero bytes to write. */
    if (s->inbuf == Z_NULL) {
      s->inbuf = (Byte*)ALLOC(Z_BUFSIZE); /* for seeking */
      zmemzero(s->inbuf, Z_BUFSIZE);
    }
    while (offset > 0)  {
      uInt size = Z_BUFSIZE;
      if (offset < Z_BUFSIZE) size = (uInt)offset;

      size = gzwrite(file, s->inbuf, size);
      if (size == 0) return -1L;

      offset -= size;
    }
    return (z_off_t)s->stream.total_in;
#endif
  }
  /* Rest of function is for reading only */

  /* compute absolute position */
  if (whence == SEEK_CUR) {
    offset += s->stream.total_out;
  }
  if (offset < 0) return -1L;

  if (s->transparent) {
    /* map to fseek */
    s->stream.avail_in = 0;
    s->stream.next_in = s->inbuf;
    if (s->file->SeekSet(offset) < 0) return -1L;

    s->stream.total_in = s->stream.total_out = (uLong)offset;
    return offset;
  }

  /* For a negative seek, rewind and use positive seek */
  if ((uLong)offset >= s->stream.total_out) {
    offset -= s->stream.total_out;
  } else if (gzrewind(file) < 0) {
    return -1L;
  }
  /* offset is now the number of bytes to skip. */

  if (offset != 0 && s->outbuf == Z_NULL) {
    s->outbuf = (Byte*)ALLOC(Z_BUFSIZE);
  }
  while (offset > 0)  {
    int size = Z_BUFSIZE;
    if (offset < Z_BUFSIZE) size = (int)offset;

    size = gzread(file, s->outbuf, (uInt)size);
    if (size <= 0) return -1L;
    offset -= size;
  }
  return (z_off_t)s->stream.total_out;
}

/* ===========================================================================
   Rewinds input file.
*/
int gzrewind (gzFile file)
{
  gz_stream *s = (gz_stream*)file;

  if (s == NULL || niFlagIsNot(s->mode,eFileOpenMode_Read)) return -1;

  s->z_err = Z_OK;
  s->z_eof = 0;
  s->stream.avail_in = 0;
  s->stream.next_in = s->inbuf;
  s->crc = crc32(0L, Z_NULL, 0);

  if (s->startpos == 0) { /* not a compressed file */
    s->file->SeekSet(0);
    return 0;
  }

  (void) inflateReset(&s->stream);
  return s->file->SeekSet(s->startpos);
}

/* ===========================================================================
   Returns the starting position for the next gzread or gzwrite on the
   given compressed file. This position represents a number of bytes in the
   uncompressed data stream.
*/
z_off_t gztell (gzFile file)
{
  return gzseek(file, 0L, SEEK_CUR);
}

/* ===========================================================================
   Returns 1 when EOF has previously been detected reading the given
   input stream, otherwise zero.
*/
int gzeof (gzFile file)
{
  gz_stream *s = (gz_stream*)file;

  return (s == NULL || niFlagIsNot(s->mode,eFileOpenMode_Read)) ? 0 : s->z_eof;
}

/* ===========================================================================
   Outputs a long in LSB order to the given file
*/
local void putLong (iFileBase *file, uLong x)
{
  tU8 bytes[4];
  int n;
  for (n = 0; n < 4; n++) {
    bytes[n] = (tU8)(x & 0xff);
    x >>= 8;
  }
  file->WriteRaw(bytes,4);
}

/* ===========================================================================
   Reads a long in LSB order from the given gz_stream. Sets z_err in case
   of error.
*/
local uLong getLong (gz_stream *s)
{
  uLong x = (uLong)get_byte(s);
  int c;

  x += ((uLong)get_byte(s))<<8;
  x += ((uLong)get_byte(s))<<16;
  c = get_byte(s);
  if (c == EOF) s->z_err = Z_DATA_ERROR;
  x += ((uLong)c)<<24;
  return x;
}

/* ===========================================================================
   Flushes all pending output if necessary, closes the compressed file
   and deallocates all the (de)compression state.
*/
int gzclose (gzFile file)
{
  int err;
  gz_stream *s = (gz_stream*)file;

  if (s == NULL) return Z_STREAM_ERROR;

  if (niFlagIs(s->mode,eFileOpenMode_Write)) {
#ifdef NO_DEFLATE
    return Z_STREAM_ERROR;
#else
    err = do_flush (file, Z_FINISH);
    if (err != Z_OK) return destroy((gz_stream*)file);

    putLong (s->file, s->crc);
    putLong (s->file, s->stream.total_in);
#endif
  }
  return destroy((gz_stream*)file);
}

/* ===========================================================================
   Returns the error message for the last error which occured on the
   given compressed file. errnum is set to zlib error number. If an
   error occured in the file system and not in the compression library,
   errnum is set to Z_ERRNO and the application may consult errno
   to get the exact error code.
*/
const char*  gzerror (gzFile file, int *errnum)
{
  char *m;
  gz_stream *s = (gz_stream*)file;

  if (s == NULL) {
    *errnum = Z_STREAM_ERROR;
    return (const char*)ERR_MSG(Z_STREAM_ERROR);
  }
  *errnum = s->z_err;
  if (*errnum == Z_OK) return (const char*)"";

  m =  (char*)(*errnum == Z_ERRNO ? "" : s->stream.msg);

  if (m == NULL || *m == '\0') m = (char*)ERR_MSG(s->z_err);

  TRYFREE(s->msg);
  s->msg = (char*)ALLOC(8 + strlen(m)+1);
  strcat(s->msg, "gzfile: ");
  strcat(s->msg, m);
  return (const char*)s->msg;
}
