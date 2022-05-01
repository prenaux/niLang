
/* pngmem.c - stub functions for memory allocation
 *
 * Last changed in libpng 1.2.13 November 13, 2006
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1998-2006 Glenn Randers-Pehrson
 * (Version 0.96 Copyright (c) 1996, 1997 Andreas Dilger)
 * (Version 0.88 Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.)
 *
 * This file provides a location for all memory allocation.  Users who
 * need special memory handling are expected to supply replacement
 * functions for ni_png_malloc() and ni_png_free(), and to use
 * ni_png_create_read_struct_2() and ni_png_create_write_struct_2() to
 * identify the replacement functions.
 */

#define PNG_INTERNAL
#include "png.h"

#if defined(PNG_READ_SUPPORTED) || defined(PNG_WRITE_SUPPORTED)

/* Borland DOS special memory handler */
#if defined(__TURBOC__) && !defined(_Windows) && !defined(__FLAT__)
/* if you change this, be sure to change the one in png.h also */

/* Allocate memory for a ni_png_struct.  The malloc and memset can be replaced
   by a single call to calloc() if this is thought to improve performance. */
ni_png_voidp /* PRIVATE */
ni_png_create_struct(int type)
{
#ifdef PNG_USER_MEM_SUPPORTED
   return (ni_png_create_struct_2(type, ni_png_malloc_ptr_NULL, ni_png_voidp_NULL));
}

/* Alternate version of ni_png_create_struct, for use with user-defined malloc. */
ni_png_voidp /* PRIVATE */
ni_png_create_struct_2(int type, ni_png_malloc_ptr malloc_fn, ni_png_voidp mem_ptr)
{
#endif /* PNG_USER_MEM_SUPPORTED */
   ni_png_size_t size;
   ni_png_voidp struct_ptr;

   if (type == PNG_STRUCT_INFO)
     size = ni_png_sizeof(ni_png_info);
   else if (type == PNG_STRUCT_PNG)
     size = ni_png_sizeof(ni_png_struct);
   else
     return (ni_png_get_copyright(NULL));

#ifdef PNG_USER_MEM_SUPPORTED
   if(malloc_fn != NULL)
   {
      ni_png_struct dummy_struct;
      ni_png_structp ni_png_ptr = &dummy_struct;
      ni_png_ptr->mem_ptr=mem_ptr;
      struct_ptr = (*(malloc_fn))(ni_png_ptr, (ni_png_uint_32)size);
   }
   else
#endif /* PNG_USER_MEM_SUPPORTED */
      struct_ptr = (ni_png_voidp)farmalloc(size);
   if (struct_ptr != NULL)
      ni_png_memset(struct_ptr, 0, size);
   return (struct_ptr);
}

/* Free memory allocated by a ni_png_create_struct() call */
void /* PRIVATE */
ni_png_destroy_struct(ni_png_voidp struct_ptr)
{
#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_destroy_struct_2(struct_ptr, ni_png_free_ptr_NULL, ni_png_voidp_NULL);
}

/* Free memory allocated by a ni_png_create_struct() call */
void /* PRIVATE */
ni_png_destroy_struct_2(ni_png_voidp struct_ptr, ni_png_free_ptr free_fn,
    ni_png_voidp mem_ptr)
{
#endif
   if (struct_ptr != NULL)
   {
#ifdef PNG_USER_MEM_SUPPORTED
      if(free_fn != NULL)
      {
         ni_png_struct dummy_struct;
         ni_png_structp ni_png_ptr = &dummy_struct;
         ni_png_ptr->mem_ptr=mem_ptr;
         (*(free_fn))(ni_png_ptr, struct_ptr);
         return;
      }
#endif /* PNG_USER_MEM_SUPPORTED */
      farfree (struct_ptr);
   }
}

/* Allocate memory.  For reasonable files, size should never exceed
 * 64K.  However, zlib may allocate more then 64K if you don't tell
 * it not to.  See zconf.h and png.h for more information. zlib does
 * need to allocate exactly 64K, so whatever you call here must
 * have the ability to do that.
 *
 * Borland seems to have a problem in DOS mode for exactly 64K.
 * It gives you a segment with an offset of 8 (perhaps to store its
 * memory stuff).  zlib doesn't like this at all, so we have to
 * detect and deal with it.  This code should not be needed in
 * Windows or OS/2 modes, and only in 16 bit mode.  This code has
 * been updated by Alexander Lehmann for version 0.89 to waste less
 * memory.
 *
 * Note that we can't use ni_png_size_t for the "size" declaration,
 * since on some systems a ni_png_size_t is a 16-bit quantity, and as a
 * result, we would be truncating potentially larger memory requests
 * (which should cause a fatal error) and introducing major problems.
 */

ni_png_voidp PNGAPI
ni_png_malloc(ni_png_structp ni_png_ptr, ni_png_uint_32 size)
{
   ni_png_voidp ret;

   if (ni_png_ptr == NULL || size == 0)
      return (NULL);

#ifdef PNG_USER_MEM_SUPPORTED
   if(ni_png_ptr->malloc_fn != NULL)
       ret = ((ni_png_voidp)(*(ni_png_ptr->malloc_fn))(ni_png_ptr, (ni_png_size_t)size));
   else
       ret = (ni_png_malloc_default(ni_png_ptr, size));
   if (ret == NULL && (ni_png_ptr->flags&PNG_FLAG_MALLOC_NULL_MEM_OK) == 0)
       ni_png_error(ni_png_ptr, "Out of memory!");
   return (ret);
}

ni_png_voidp PNGAPI
ni_png_malloc_default(ni_png_structp ni_png_ptr, ni_png_uint_32 size)
{
   ni_png_voidp ret;
#endif /* PNG_USER_MEM_SUPPORTED */

   if (ni_png_ptr == NULL || size == 0)
      return (NULL);

#ifdef PNG_MAX_MALLOC_64K
   if (size > (ni_png_uint_32)65536L)
   {
      ni_png_warning(ni_png_ptr, "Cannot Allocate > 64K");
      ret = NULL;
   }
   else
#endif

   if (size != (size_t)size)
     ret = NULL;
   else if (size == (ni_png_uint_32)65536L)
   {
      if (ni_png_ptr->offset_table == NULL)
      {
         /* try to see if we need to do any of this fancy stuff */
         ret = farmalloc(size);
         if (ret == NULL || ((ni_png_size_t)ret & 0xffff))
         {
            int num_blocks;
            ni_png_uint_32 total_size;
            ni_png_bytep table;
            int i;
            ni_png_byte huge * hptr;

            if (ret != NULL)
            {
               farfree(ret);
               ret = NULL;
            }

            if(ni_png_ptr->zlib_window_bits > 14)
               num_blocks = (int)(1 << (ni_png_ptr->zlib_window_bits - 14));
            else
               num_blocks = 1;
            if (ni_png_ptr->zlib_mem_level >= 7)
               num_blocks += (int)(1 << (ni_png_ptr->zlib_mem_level - 7));
            else
               num_blocks++;

            total_size = ((ni_png_uint_32)65536L) * (ni_png_uint_32)num_blocks+16;

            table = farmalloc(total_size);

            if (table == NULL)
            {
#ifndef PNG_USER_MEM_SUPPORTED
               if ((ni_png_ptr->flags&PNG_FLAG_MALLOC_NULL_MEM_OK) == 0)
                  ni_png_error(ni_png_ptr, "Out Of Memory."); /* Note "O" and "M" */
               else
                  ni_png_warning(ni_png_ptr, "Out Of Memory.");
#endif
               return (NULL);
            }

            if ((ni_png_size_t)table & 0xfff0)
            {
#ifndef PNG_USER_MEM_SUPPORTED
               if ((ni_png_ptr->flags&PNG_FLAG_MALLOC_NULL_MEM_OK) == 0)
                  ni_png_error(ni_png_ptr,
                    "Farmalloc didn't return normalized pointer");
               else
                  ni_png_warning(ni_png_ptr,
                    "Farmalloc didn't return normalized pointer");
#endif
               return (NULL);
            }

            ni_png_ptr->offset_table = table;
            ni_png_ptr->offset_table_ptr = farmalloc(num_blocks *
               ni_png_sizeof (ni_png_bytep));

            if (ni_png_ptr->offset_table_ptr == NULL)
            {
#ifndef PNG_USER_MEM_SUPPORTED
               if ((ni_png_ptr->flags&PNG_FLAG_MALLOC_NULL_MEM_OK) == 0)
                  ni_png_error(ni_png_ptr, "Out Of memory."); /* Note "O" and "M" */
               else
                  ni_png_warning(ni_png_ptr, "Out Of memory.");
#endif
               return (NULL);
            }

            hptr = (ni_png_byte huge *)table;
            if ((ni_png_size_t)hptr & 0xf)
            {
               hptr = (ni_png_byte huge *)((long)(hptr) & 0xfffffff0L);
               hptr = hptr + 16L;  /* "hptr += 16L" fails on Turbo C++ 3.0 */
            }
            for (i = 0; i < num_blocks; i++)
            {
               ni_png_ptr->offset_table_ptr[i] = (ni_png_bytep)hptr;
               hptr = hptr + (ni_png_uint_32)65536L;  /* "+=" fails on TC++3.0 */
            }

            ni_png_ptr->offset_table_number = num_blocks;
            ni_png_ptr->offset_table_count = 0;
            ni_png_ptr->offset_table_count_free = 0;
         }
      }

      if (ni_png_ptr->offset_table_count >= ni_png_ptr->offset_table_number)
      {
#ifndef PNG_USER_MEM_SUPPORTED
         if ((ni_png_ptr->flags&PNG_FLAG_MALLOC_NULL_MEM_OK) == 0)
            ni_png_error(ni_png_ptr, "Out of Memory."); /* Note "o" and "M" */
         else
            ni_png_warning(ni_png_ptr, "Out of Memory.");
#endif
         return (NULL);
      }

      ret = ni_png_ptr->offset_table_ptr[ni_png_ptr->offset_table_count++];
   }
   else
      ret = farmalloc(size);

#ifndef PNG_USER_MEM_SUPPORTED
   if (ret == NULL)
   {
      if ((ni_png_ptr->flags&PNG_FLAG_MALLOC_NULL_MEM_OK) == 0)
         ni_png_error(ni_png_ptr, "Out of memory."); /* Note "o" and "m" */
      else
         ni_png_warning(ni_png_ptr, "Out of memory."); /* Note "o" and "m" */
   }
#endif

   return (ret);
}

/* free a pointer allocated by ni_png_malloc().  In the default
   configuration, ni_png_ptr is not used, but is passed in case it
   is needed.  If ptr is NULL, return without taking any action. */
void PNGAPI
ni_png_free(ni_png_structp ni_png_ptr, ni_png_voidp ptr)
{
   if (ni_png_ptr == NULL || ptr == NULL)
      return;

#ifdef PNG_USER_MEM_SUPPORTED
   if (ni_png_ptr->free_fn != NULL)
   {
      (*(ni_png_ptr->free_fn))(ni_png_ptr, ptr);
      return;
   }
   else ni_png_free_default(ni_png_ptr, ptr);
}

void PNGAPI
ni_png_free_default(ni_png_structp ni_png_ptr, ni_png_voidp ptr)
{
#endif /* PNG_USER_MEM_SUPPORTED */

   if(ni_png_ptr == NULL) return;

   if (ni_png_ptr->offset_table != NULL)
   {
      int i;

      for (i = 0; i < ni_png_ptr->offset_table_count; i++)
      {
         if (ptr == ni_png_ptr->offset_table_ptr[i])
         {
            ptr = NULL;
            ni_png_ptr->offset_table_count_free++;
            break;
         }
      }
      if (ni_png_ptr->offset_table_count_free == ni_png_ptr->offset_table_count)
      {
         farfree(ni_png_ptr->offset_table);
         farfree(ni_png_ptr->offset_table_ptr);
         ni_png_ptr->offset_table = NULL;
         ni_png_ptr->offset_table_ptr = NULL;
      }
   }

   if (ptr != NULL)
   {
      farfree(ptr);
   }
}

#else /* Not the Borland DOS special memory handler */

/* Allocate memory for a ni_png_struct or a ni_png_info.  The malloc and
   memset can be replaced by a single call to calloc() if this is thought
   to improve performance noticably. */
ni_png_voidp /* PRIVATE */
ni_png_create_struct(int type)
{
#ifdef PNG_USER_MEM_SUPPORTED
   return (ni_png_create_struct_2(type, ni_png_malloc_ptr_NULL, ni_png_voidp_NULL));
}

/* Allocate memory for a ni_png_struct or a ni_png_info.  The malloc and
   memset can be replaced by a single call to calloc() if this is thought
   to improve performance noticably. */
ni_png_voidp /* PRIVATE */
ni_png_create_struct_2(int type, ni_png_malloc_ptr malloc_fn, ni_png_voidp mem_ptr)
{
#endif /* PNG_USER_MEM_SUPPORTED */
   ni_png_size_t size;
   ni_png_voidp struct_ptr;

   if (type == PNG_STRUCT_INFO)
      size = ni_png_sizeof(ni_png_info);
   else if (type == PNG_STRUCT_PNG)
      size = ni_png_sizeof(ni_png_struct);
   else
      return (NULL);

#ifdef PNG_USER_MEM_SUPPORTED
   if(malloc_fn != NULL)
   {
      ni_png_struct dummy_struct;
      ni_png_structp ni_png_ptr = &dummy_struct;
      ni_png_ptr->mem_ptr=mem_ptr;
      struct_ptr = (*(malloc_fn))(ni_png_ptr, size);
      if (struct_ptr != NULL)
         ni_png_memset(struct_ptr, 0, size);
      return (struct_ptr);
   }
#endif /* PNG_USER_MEM_SUPPORTED */

#if defined(__TURBOC__) && !defined(__FLAT__)
   struct_ptr = (ni_png_voidp)farmalloc(size);
#else
# if defined(_MSC_VER) && defined(MAXSEG_64K)
   struct_ptr = (ni_png_voidp)halloc(size,1);
# else
   struct_ptr = (ni_png_voidp)malloc(size);
# endif
#endif
   if (struct_ptr != NULL)
      ni_png_memset(struct_ptr, 0, size);

   return (struct_ptr);
}


/* Free memory allocated by a ni_png_create_struct() call */
void /* PRIVATE */
ni_png_destroy_struct(ni_png_voidp struct_ptr)
{
#ifdef PNG_USER_MEM_SUPPORTED
   ni_png_destroy_struct_2(struct_ptr, ni_png_free_ptr_NULL, ni_png_voidp_NULL);
}

/* Free memory allocated by a ni_png_create_struct() call */
void /* PRIVATE */
ni_png_destroy_struct_2(ni_png_voidp struct_ptr, ni_png_free_ptr free_fn,
    ni_png_voidp mem_ptr)
{
#endif /* PNG_USER_MEM_SUPPORTED */
   if (struct_ptr != NULL)
   {
#ifdef PNG_USER_MEM_SUPPORTED
      if(free_fn != NULL)
      {
         ni_png_struct dummy_struct;
         ni_png_structp ni_png_ptr = &dummy_struct;
         ni_png_ptr->mem_ptr=mem_ptr;
         (*(free_fn))(ni_png_ptr, struct_ptr);
         return;
      }
#endif /* PNG_USER_MEM_SUPPORTED */
#if defined(__TURBOC__) && !defined(__FLAT__)
      farfree(struct_ptr);
#else
# if defined(_MSC_VER) && defined(MAXSEG_64K)
      hfree(struct_ptr);
# else
      free(struct_ptr);
# endif
#endif
   }
}

/* Allocate memory.  For reasonable files, size should never exceed
   64K.  However, zlib may allocate more then 64K if you don't tell
   it not to.  See zconf.h and png.h for more information.  zlib does
   need to allocate exactly 64K, so whatever you call here must
   have the ability to do that. */

ni_png_voidp PNGAPI
ni_png_malloc(ni_png_structp ni_png_ptr, ni_png_uint_32 size)
{
   ni_png_voidp ret;

#ifdef PNG_USER_MEM_SUPPORTED
   if (ni_png_ptr == NULL || size == 0)
      return (NULL);

   if(ni_png_ptr->malloc_fn != NULL)
       ret = ((ni_png_voidp)(*(ni_png_ptr->malloc_fn))(ni_png_ptr, (ni_png_size_t)size));
   else
       ret = (ni_png_malloc_default(ni_png_ptr, size));
   if (ret == NULL && (ni_png_ptr->flags&PNG_FLAG_MALLOC_NULL_MEM_OK) == 0)
       ni_png_error(ni_png_ptr, "Out of Memory!");
   return (ret);
}

ni_png_voidp PNGAPI
ni_png_malloc_default(ni_png_structp ni_png_ptr, ni_png_uint_32 size)
{
   ni_png_voidp ret;
#endif /* PNG_USER_MEM_SUPPORTED */

   if (ni_png_ptr == NULL || size == 0)
      return (NULL);

#ifdef PNG_MAX_MALLOC_64K
   if (size > (ni_png_uint_32)65536L)
   {
#ifndef PNG_USER_MEM_SUPPORTED
      if(ni_png_ptr->flags&PNG_FLAG_MALLOC_NULL_MEM_OK) == 0)
         ni_png_error(ni_png_ptr, "Cannot Allocate > 64K");
      else
#endif
         return NULL;
   }
#endif

 /* Check for overflow */
#if defined(__TURBOC__) && !defined(__FLAT__)
 if (size != (unsigned long)size)
   ret = NULL;
 else
   ret = farmalloc(size);
#else
# if defined(_MSC_VER) && defined(MAXSEG_64K)
 if (size != (unsigned long)size)
   ret = NULL;
 else
   ret = halloc(size, 1);
# else
 if (size != (size_t)size)
   ret = NULL;
 else
   ret = malloc((size_t)size);
# endif
#endif

#ifndef PNG_USER_MEM_SUPPORTED
   if (ret == NULL && (ni_png_ptr->flags&PNG_FLAG_MALLOC_NULL_MEM_OK) == 0)
      ni_png_error(ni_png_ptr, "Out of Memory");
#endif

   return (ret);
}

/* Free a pointer allocated by ni_png_malloc().  If ptr is NULL, return
   without taking any action. */
void PNGAPI
ni_png_free(ni_png_structp ni_png_ptr, ni_png_voidp ptr)
{
   if (ni_png_ptr == NULL || ptr == NULL)
      return;

#ifdef PNG_USER_MEM_SUPPORTED
   if (ni_png_ptr->free_fn != NULL)
   {
      (*(ni_png_ptr->free_fn))(ni_png_ptr, ptr);
      return;
   }
   else ni_png_free_default(ni_png_ptr, ptr);
}
void PNGAPI
ni_png_free_default(ni_png_structp ni_png_ptr, ni_png_voidp ptr)
{
   if (ni_png_ptr == NULL || ptr == NULL)
      return;

#endif /* PNG_USER_MEM_SUPPORTED */

#if defined(__TURBOC__) && !defined(__FLAT__)
   farfree(ptr);
#else
# if defined(_MSC_VER) && defined(MAXSEG_64K)
   hfree(ptr);
# else
   free(ptr);
# endif
#endif
}

#endif /* Not Borland DOS special memory handler */

#if defined(PNG_1_0_X)
#  define ni_png_malloc_warn ni_png_malloc
#else
/* This function was added at libpng version 1.2.3.  The ni_png_malloc_warn()
 * function will set up ni_png_malloc() to issue a ni_png_warning and return NULL
 * instead of issuing a ni_png_error, if it fails to allocate the requested
 * memory.
 */
ni_png_voidp PNGAPI
ni_png_malloc_warn(ni_png_structp ni_png_ptr, ni_png_uint_32 size)
{
   ni_png_voidp ptr;
   ni_png_uint_32 save_flags;
   if(ni_png_ptr == NULL) return (NULL);

   save_flags=ni_png_ptr->flags;
   ni_png_ptr->flags|=PNG_FLAG_MALLOC_NULL_MEM_OK;
   ptr = (ni_png_voidp)ni_png_malloc((ni_png_structp)ni_png_ptr, size);
   ni_png_ptr->flags=save_flags;
   return(ptr);
}
#endif

ni_png_voidp PNGAPI
ni_png_memcpy_check (ni_png_structp ni_png_ptr, ni_png_voidp s1, ni_png_voidp s2,
   ni_png_uint_32 length)
{
   ni_png_size_t size;

   size = (ni_png_size_t)length;
   if ((ni_png_uint_32)size != length)
      ni_png_error(ni_png_ptr,"Overflow in ni_png_memcpy_check.");

   return(ni_png_memcpy (s1, s2, size));
}

ni_png_voidp PNGAPI
ni_png_memset_check (ni_png_structp ni_png_ptr, ni_png_voidp s1, int value,
   ni_png_uint_32 length)
{
   ni_png_size_t size;

   size = (ni_png_size_t)length;
   if ((ni_png_uint_32)size != length)
      ni_png_error(ni_png_ptr,"Overflow in ni_png_memset_check.");

   return (ni_png_memset (s1, value, size));

}

#ifdef PNG_USER_MEM_SUPPORTED
/* This function is called when the application wants to use another method
 * of allocating and freeing memory.
 */
void PNGAPI
ni_png_set_mem_fn(ni_png_structp ni_png_ptr, ni_png_voidp mem_ptr, ni_png_malloc_ptr
  malloc_fn, ni_png_free_ptr free_fn)
{
   if(ni_png_ptr != NULL) {
   ni_png_ptr->mem_ptr = mem_ptr;
   ni_png_ptr->malloc_fn = malloc_fn;
   ni_png_ptr->free_fn = free_fn;
   }
}

/* This function returns a pointer to the mem_ptr associated with the user
 * functions.  The application should free any memory associated with this
 * pointer before ni_png_write_destroy and ni_png_read_destroy are called.
 */
ni_png_voidp PNGAPI
ni_png_get_mem_ptr(ni_png_structp ni_png_ptr)
{
   if(ni_png_ptr == NULL) return (NULL);
   return ((ni_png_voidp)ni_png_ptr->mem_ptr);
}
#endif /* PNG_USER_MEM_SUPPORTED */
#endif /* PNG_READ_SUPPORTED || PNG_WRITE_SUPPORTED */
