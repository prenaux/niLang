#ifndef __AVT_UTILS_H__
#define __AVT_UTILS_H__

typedef unsigned char uint8;

#if !defined inline && !defined __cplusplus
#  ifdef _MSC_VER
#    define inline __forceinline
#else
#    define inline __inline
#  endif
#endif

/*
  RGB <-> YCoCg

  Y  = [ 1/4  1/2  1/4] [R]
  Co = [ 1/2    0 -1/2] [G]
  CG = [-1/4  1/2 -1/4] [B]

  R  = [   1    1   -1] [Y]
  G  = [   1    0    1] [Co]
  B  = [   1   -1   -1] [Cg]

*/

static inline uint8 CLAMP_UINT8( int x ) {
  return ( (x) < 0 ? (0) : ( (x) > 255 ? 255 : (x) ) );
}

#define RGB_TO_YCOCG_Y( r, g, b )   ( ( (    r +   (g<<1) +  b     ) + 2 ) >> 2 )
#define RGB_TO_YCOCG_CO( r, g, b )  ( ( (   (r<<1)        - (b<<1) ) + 2 ) >> 2 )
#define RGB_TO_YCOCG_CG( r, g, b )  ( ( ( -  r +   (g<<1) -  b     ) + 2 ) >> 2 )

#define COCG_TO_R( co, cg )         ( co - cg )
#define COCG_TO_G( co, cg )         ( cg )
#define COCG_TO_B( co, cg )         ( - co - cg )

// 24-bits RGB to 24 bits YCoCg
static inline void Convert_RGB_YCoCg(uint8* d, const uint8* s, const int width, const int height) {
  int i;
  for ( i = 0; i < width * height; ++i ) {
    int r = s[i*3+0];
    int g = s[i*3+1];
    int b = s[i*3+2];
    d[i*3+0] = CLAMP_UINT8( RGB_TO_YCOCG_Y( r, g, b ) );
    d[i*3+1] = CLAMP_UINT8( RGB_TO_YCOCG_CO( r, g, b ) + 128 );
    d[i*3+2] = CLAMP_UINT8( RGB_TO_YCOCG_CG( r, g, b ) + 128 );
  }
}

// Inplace 24-bits YCoCg to 24 bits RGB
static inline void Convert_YCoCg_RGB(uint8* d, const uint8* s, const int width, const int height ) {
  int i;
  for ( i = 0; i < width * height; ++i ) {
    int y  = s[i*3+0];
    int co = s[i*3+1] - 128;
    int cg = s[i*3+2] - 128;
    d[i*3+0] = CLAMP_UINT8( y + COCG_TO_R( co, cg ) );
    d[i*3+1] = CLAMP_UINT8( y + COCG_TO_G( co, cg ) );
    d[i*3+2] = CLAMP_UINT8( y + COCG_TO_B( co, cg ) );
  }
}

// 32-bits RGBA to 32 bits YCoCgA
static inline void Convert_RGBA_YCoCg(uint8* d, const uint8* s, const int width, const int height) {
  int i;
  for ( i = 0; i < width * height; ++i ) {
    int r = s[i*3+0];
    int g = s[i*3+1];
    int b = s[i*3+2];
    int a = s[i*3+3];
    d[i*3+0] = CLAMP_UINT8( RGB_TO_YCOCG_Y( r, g, b ) );
    d[i*3+1] = CLAMP_UINT8( RGB_TO_YCOCG_CO( r, g, b ) + 128 );
    d[i*3+2] = CLAMP_UINT8( RGB_TO_YCOCG_CG( r, g, b ) + 128 );
    d[i*3+3] = a;
  }
}

// 32-bits YCoCgA to 32 bits RGBA
static inline void Convert_YCoCgA_RGBA(uint8* d, const uint8* s, const int width, const int height ) {
  int i;
  for ( i = 0; i < width * height; ++i ) {
    int   y  = s[i*3+0];
    int   co = s[i*3+1] - 128;
    int   cg = s[i*3+2] - 128;
    uint8 a  = s[i*3+3];
    d[i*3+0] = CLAMP_UINT8( y + COCG_TO_R( co, cg ) );
    d[i*3+1] = CLAMP_UINT8( y + COCG_TO_G( co, cg ) );
    d[i*3+2] = CLAMP_UINT8( y + COCG_TO_B( co, cg ) );
    d[i*3+3] = a;
  }
}

#endif
