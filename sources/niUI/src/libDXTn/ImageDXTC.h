/*

  ImageDXTC.h

*/

#ifndef IMAGEDXTC_H_
#define IMAGEDXTC_H_

#include "ImageBase.h"

class CodeBook;

typedef enum
{
  DC_None,
  DC_DXT1,
  DC_DXT3,
  DC_DXT5,
} DXTCMethod;


class ImageDXTC
{
 private:
  long    XSize, YSize;
  ni::tU16    *pBlocks;
  DXTCMethod  Method;
  ni::tU8   AlphaValue;

  void    DXT1to32(Image *pDest);
  void    DXT3to32(Image *pDest);

  void    EmitTransparentBlock(ni::tU16 *pDest);
  void    Emit1ColorBlock(ni::tU16 *pDest, Color c);
  void    Emit1ColorBlockTrans(ni::tU16 *pDest, Color c, Color *pSrc);
  void    Emit2ColorBlock(ni::tU16 *pDest, Color c1, Color c2, Color *pSrc);
  void    Emit2ColorBlockTrans(ni::tU16 *pDest, Color c1, Color c2, Color *pSrc);
  void    EmitMultiColorBlock3(ni::tU16 *pDest, CodeBook &cb, Color *pSrc);
  void    EmitMultiColorBlock4(ni::tU16 *pDest, CodeBook &cb, Color *pSrc);
  void    EmitMultiColorBlockTrans(ni::tU16 *pDest, CodeBook &cb, Color *pSrc);
  void    Emit4BitAlphaBlock(ni::tU16 *pDest, Color *pSrc);


 public:
  ImageDXTC();
  ~ImageDXTC();

  void  ReleaseAll(void);

  void  SetMethod(DXTCMethod NewMethod);  // MUST be called before setsize
  void  SetSize(long x, long y);

  long    GetXSize(void) {return XSize;}
  long    GetYSize(void) {return YSize;}
  DXTCMethod  GetMethod(void) {return Method;}
  ni::tU16    *GetBlocks(void) {return pBlocks;}

  void  FromImage32(Image *pSrc, DXTCMethod = DC_None);
  void  ToImage32(Image *pDest);

  void  CompressDXT1(Image *pSrcImg);   // Potentially called by FromImage32
  void  CompressDXT3(Image *pSrcImg);   // Potentially called by FromImage32

  void PlotDXT1(ni::tU16 *pSrc, Color *pDest, long Pitch);
  void PlotDXT3Alpha(ni::tU16 *pSrc, Color *pDest, long Pitch);
  void PlotDXT5Alpha(ni::tU16 *pSrc, Color *pDest, long Pitch);
};

#endif
