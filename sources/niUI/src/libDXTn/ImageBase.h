/*

  Image.h - Generic 24 bit image class

*/

#ifndef IMAGE_H_
#define IMAGE_H_

#include <niLang/Math/MathFloat.h>

inline long FtoL(float f)
{
  return ni::FloatToIntNearest(f);
}

typedef enum
{
  AT_None,      // No alpha used - All 0xff
  AT_Binary,      // 0x00 / 0xff
  AT_Constant,    // Single, constant non-0xff value used
  AT_ConstantBinary,  // 0x00 / 0x??
  AT_DualConstant,  // 0x?? / 0x?? - Two constants
  AT_Modulated,   // Multiple values used
} AlphaType;

typedef enum
{
  QM_Lloyd = 1,
  QM_MedianCut = 2,
} QuantMethodType;


class Color
{
 public:
  Color() {;}
  Color(ni::tU32 c) : Col(c) {;}
  Color(ni::tU8 R, ni::tU8 G, ni::tU8 B, ni::tU8 A) : a(A), r(R), g(G), b(B) {;}

  union {
    struct { ni::tU8 a, r, g, b; };
    ni::tU32  Col;
  };
};

// ----------------------------------------------------------------------------
// Base image class
// ----------------------------------------------------------------------------
class Image : public ni::cMemImpl
{
 private:
  long  XSize, YSize;
  Color *pPixels;

 public:
  Image();
  ~Image();

  long  GetXSize(void) {return XSize;}
  long  GetYSize(void) {return YSize;}

  double  Diff(Image *pComp);
  double  MSE(Image *pComp);

  AlphaType AlphaUsage(unsigned char *pAlpha1 = 0, unsigned char *pAlpha0 = 0); // Returns how the alpha channel is used
  void    AlphaToBinary(unsigned char Threshold = 128);           // Force alpha to be 0x00 / 0xff

  static  QuantMethodType QuantMethod;
  static  bool QuantDiffusion;

  void ReleaseAll(void);

  void  SetSize(long x, long y);
  Color *GetPixels(void) {return pPixels;}

  long    UniqueColors(void);                         // Unique color count for the image

  float AverageSlope(void);       // Compute the average slope between pixel neighbors

  void  DiffuseError(long aBits, long rBits, long gBits, long bBits); // # of bits per gun

  bool  Crop(long x1, long y1, long x2, long y2);
  bool  SizeCanvas(long NewX, long NewY);
  bool  Quarter(Image &Dest);     // Generate a quarter size image in Dest
  bool  HalfX(Image &Dest);
  bool  HalfY(Image &Dest);

  void  ResizeX(Image &Dest, long NewX);
  void  ScaleUpX(Image &Dest, long NewX);
  void  ScaleDownX(Image &Dest, long NewX);

  void  ResizeY(Image &Dest, long NewY);
  void  ScaleUpY(Image &Dest, long NewY);
  void  ScaleDownY(Image &Dest, long NewY);
};

#endif
