#ifndef __GZIO_44925116_H__
#define __GZIO_44925116_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include "zlib/zlib.h"

typedef voidp gzFile;

/*
  Opens a gzip (.gz) file for reading or writing. The mode parameter
  is as in fopen ("rb" or "wb") but can also include a compression level
  ("wb9") or a strategy: 'f' for filtered data as in "wb6f", 'h' for
  Huffman only compression as in "wb1h". (See the description
  of deflateInit2 for more information about the strategy parameter.)

  gzopen can be used to read a file which is not in gzip format; in this
  case gzread will directly read from the file without decompression.

  gzopen returns NULL if the file could not be opened or if there was
  insufficient memory to allocate the (de)compression state; errno
  can be checked to distinguish the two cases (if errno is zero, the
  zlib error is Z_MEM_ERROR).  */

gzFile gzdopen  OF((ni::iFileBase* fd, const char *mode, bool abA25Z));

/*
  gzdopen() associates a gzFile with the file descriptor fd.  File
  descriptors are obtained from calls like open, dup, creat, pipe or
  fileno (in the file has been previously opened with fopen).
  The mode parameter is as in gzopen.
  The next call of gzclose on the returned gzFile will also close the
  file descriptor fd, just like fclose(fdopen(fd), mode) closes the file
  descriptor fd. If you want to keep fd open, use gzdopen(dup(fd), mode).
  gzdopen returns NULL if there was insufficient memory to allocate
  the (de)compression state.
*/

int gzsetparams OF((gzFile file, int level, int strategy));
/*
  Dynamically update the compression level or strategy. See the description
  of deflateInit2 for the meaning of these parameters.
  gzsetparams returns Z_OK if success, or Z_STREAM_ERROR if the file was not
  opened for writing.
*/

int    gzread  OF((gzFile file, voidp buf, unsigned len));
/*
  Reads the given number of uncompressed bytes from the compressed file.
  If the input file was not in gzip format, gzread copies the given number
  of bytes into the buffer.
  gzread returns the number of uncompressed bytes actually read (0 for
  end of file, -1 for error). */

int    gzwrite OF((gzFile file,
                   const voidp buf, unsigned len));
/*
  Writes the given number of uncompressed bytes into the compressed file.
  gzwrite returns the number of uncompressed bytes actually written
  (0 in case of error).
*/

int gzputs OF((gzFile file, const char *s));
/*
  Writes the given null-terminated string to the compressed file, excluding
  the terminating null character.
  gzputs returns the number of characters written, or -1 in case of error.
*/

char * gzgets OF((gzFile file, char *buf, int len));
/*
  Reads bytes from the compressed file until len-1 characters are read, or
  a newline character is read and transferred to buf, or an end-of-file
  condition is encountered.  The string is then terminated with a null
  character.
  gzgets returns buf, or Z_NULL in case of error.
*/

int    gzputc OF((gzFile file, int c));
/*
  Writes c, converted to an unsigned char, into the compressed file.
  gzputc returns the value that was written, or -1 in case of error.
*/

int    gzgetc OF((gzFile file));
/*
  Reads one byte from the compressed file. gzgetc returns this byte
  or -1 in case of end of file or error.
*/

int    gzflush OF((gzFile file, int flush));
/*
  Flushes all pending output into the compressed file. The parameter
  flush is as in the deflate() function. The return value is the zlib
  error number (see function gzerror below). gzflush returns Z_OK if
  the flush parameter is Z_FINISH and all output could be flushed.
  gzflush should be called only when strictly necessary because it can
  degrade compression.
*/

z_off_t    gzseek OF((gzFile file,
                      z_off_t offset, int whence));
/*
  Sets the starting position for the next gzread or gzwrite on the
  given compressed file. The offset represents a number of bytes in the
  uncompressed data stream. The whence parameter is defined as in lseek(2);
  the value SEEK_END is not supported.
  If the file is opened for reading, this function is emulated but can be
  extremely slow. If the file is opened for writing, only forward seeks are
  supported; gzseek then compresses a sequence of zeroes up to the
  starting position.

  gzseek returns the resulting offset location as measured in bytes from
  the beginning of the uncompressed stream, or -1 in case of error, in
  particular if the file is opened for writing and the starting position
  would be before the current position.
*/

int    gzrewind OF((gzFile file));
/*
  Rewinds the given file. This function is supported only for reading.

  gzrewind(file) is equivalent to (int)gzseek(file, 0L, SEEK_SET)
*/

z_off_t    gztell OF((gzFile file));
/*
  Returns the starting position for the next gzread or gzwrite on the
  given compressed file. This position represents a number of bytes in the
  uncompressed data stream.

  gztell(file) is equivalent to gzseek(file, 0L, SEEK_CUR)
*/

int gzeof OF((gzFile file));
/*
  Returns 1 when EOF has previously been detected reading the given
  input stream, otherwise zero.
*/

int    gzclose OF((gzFile file));
/*
  Flushes all pending output if necessary, closes the compressed file
  and deallocates all the (de)compression state. The return value is the zlib
  error number (see function gzerror below).
*/

const char * gzerror OF((gzFile file, int *errnum));
/*
  Returns the error message for the last error which occurred on the
  given compressed file. errnum is set to zlib error number. If an
  error occurred in the file system and not in the compression library,
  errnum is set to Z_ERRNO and the application may consult errno
  to get the exact error code.
*/


/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __GZIO_44925116_H__
