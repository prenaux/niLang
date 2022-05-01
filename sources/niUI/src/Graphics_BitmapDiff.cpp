#include "stdafx.h"
#include "Graphics.h"
#include "Image.h"

// blend semi-transparent color with white
static __forceinline uint8_t blend(uint8_t c, double a) {
  return 255 + (c - 255) * a;
}
static __forceinline double rgb2y(uint8_t r, uint8_t g, uint8_t b) {
  return r * 0.29889531 + g * 0.58662247 + b * 0.11448223;
}
static __forceinline double rgb2i(uint8_t r, uint8_t g, uint8_t b) {
  return r * 0.59597799 - g * 0.27417610 - b * 0.32180189;
}
static __forceinline double rgb2q(uint8_t r, uint8_t g, uint8_t b) {
  return r * 0.21147017 - g * 0.52261711 + b * 0.31114694;
}

// calculate color difference according to the paper "Measuring perceived color difference
// using YIQ NTSC transmission color space in mobile applications" by Y. Kotsarenko and F. Ramos
static double colorDelta(const uint8_t* img1, const uint8_t* img2, size_t k, size_t m, bool yOnly = false)
{
  double a1 = double(img1[k + 3]) / 255;
  double a2 = double(img2[m + 3]) / 255;

  uint8_t r1 = blend(img1[k + 0], a1);
  uint8_t g1 = blend(img1[k + 1], a1);
  uint8_t b1 = blend(img1[k + 2], a1);

  uint8_t r2 = blend(img2[m + 0], a2);
  uint8_t g2 = blend(img2[m + 1], a2);
  uint8_t b2 = blend(img2[m + 2], a2);

  double y = rgb2y(r1, g1, b1) - rgb2y(r2, g2, b2);

  if (yOnly) return y; // brightness difference only

  double i = rgb2i(r1, g1, b1) - rgb2i(r2, g2, b2);
  double q = rgb2q(r1, g1, b1) - rgb2q(r2, g2, b2);

  return 0.5053 * y * y + 0.299 * i * i + 0.1957 * q * q;
}

static __forceinline void drawPixel(uint8_t* output, size_t pos, uint8_t r, uint8_t g, uint8_t b) {
  output[pos + 0] = r;
  output[pos + 1] = g;
  output[pos + 2] = b;
  output[pos + 3] = 255;
}

static double grayPixel(const uint8_t* img, size_t i) {
  double a = double(img[i + 3]) / 255;
  uint8_t r = blend(img[i + 0], a);
  uint8_t g = blend(img[i + 1], a);
  uint8_t b = blend(img[i + 2], a);
  return rgb2y(r, g, b);
}

// check if a pixel is likely a part of anti-aliasing;
// based on "Anti-aliased Pixel and Intensity Slope Detector" paper by V. Vysniauskas, 2009
static bool antialiased(const uint8_t* img, size_t x1, size_t y1, size_t width, size_t height, const uint8_t* img2 = nullptr) {
  size_t x0 = x1 > 0 ? x1 - 1 : 0;
  size_t y0 = y1 > 0 ? y1 - 1 : 0;
  size_t x2 = astl::min(x1 + 1, width - 1);
  size_t y2 = astl::min(y1 + 1, height - 1);
  size_t pos = (y1 * width + x1) * 4;
  uint64_t zeroes = 0;
  uint64_t positives = 0;
  uint64_t negatives = 0;
  double min = 0;
  double max = 0;
  size_t minX = 0, minY = 0, maxX = 0, maxY = 0;

  // go through 8 adjacent pixels
  for (size_t x = x0; x <= x2; x++) {
    for (size_t y = y0; y <= y2; y++) {
      if (x == x1 && y == y1) continue;

      // brightness delta between the center pixel and adjacent one
      double delta = colorDelta(img, img, pos, (y * width + x) * 4, true);

      // count the number of equal, darker and brighter adjacent pixels
      if (delta == 0) zeroes++;
      else if (delta < 0) negatives++;
      else if (delta > 0) positives++;

      // if found more than 2 equal siblings, it's definitely not anti-aliasing
      if (zeroes > 2) return false;

      if (!img2) continue;

      // remember the darkest pixel
      if (delta < min) {
        min = delta;
        minX = x;
        minY = y;
      }
      // remember the brightest pixel
      if (delta > max) {
        max = delta;
        maxX = x;
        maxY = y;
      }
    }
  }

  if (!img2) return true;

  // if there are no both darker and brighter pixels among siblings, it's not anti-aliasing
  if (negatives == 0 || positives == 0) return false;

  // if either the darkest or the brightest pixel has more than 2 equal siblings in both images
  // (definitely not anti-aliased), this pixel is anti-aliased
  return (!antialiased(img, minX, minY, width, height) && !antialiased(img2, minX, minY, width, height)) ||
      (!antialiased(img, maxX, maxY, width, height) && !antialiased(img2, maxX, maxY, width, height));
}

// Based of pixelmatch: https://github.com/mapbox/pixelmatch
static int64_t _ComputeImageDiff(
  const uint8_t* img1,
  const uint8_t* img2,
  size_t width,
  size_t height,
  uint8_t* output,
  double threshold,
  bool ignoreAA)
{
  // maximum acceptable square distance between two colors;
  // 35215 is the maximum possible value for the YIQ difference metric
  const double maxDelta = 35215 * threshold * threshold;
  int64_t diff = 0;

  // compare each pixel of one image against the other one
  for (size_t y = 0; y < height; y++) {
    for (size_t x = 0; x < width; x++) {

      size_t pos = (y * width + x) * 4;

      // squared YUV distance between colors at this pixel position
      double delta = colorDelta(img1, img2, pos, pos);

      // the color difference is above the threshold
      if (delta > maxDelta) {
        // check it's a real rendering difference or just anti-aliasing
        if (ignoreAA && (antialiased(img1, x, y, width, height, img2) ||
                         antialiased(img2, x, y, width, height, img1))) {
          // one of the pixels is anti-aliasing; draw as yellow and do not count as difference
          if (output) drawPixel(output, pos, 255, 255, 0);

        } else {
          // found substantial difference not caused by anti-aliasing; draw it as red
          if (output) drawPixel(output, pos, 255, 0, 0);
          diff++;
        }

      } else if (output) {
        // pixels are similar; draw background as grayscale image blended with white
        uint8_t val = blend(grayPixel(img1, pos), 0.1);
        drawPixel(output, pos, val, val, val);
      }
    }
  }

  // return the number of different pixels
  return diff;
}

tI64  __stdcall cGraphics::ComputeBitmapDiff(const iBitmap2D* apImgA,
                                             const iBitmap2D* apImgB,
                                             iBitmap2D* apOutput,
                                             tF64 afThreshold,
                                             tBool abIgnoreAA)
{
  niCheckIsOK(apImgA,-1);
  niCheckIsOK(apImgB,-1);
  niCheck(apImgA->GetWidth() == apImgB->GetWidth(), -1);
  niCheck(apImgA->GetHeight() == apImgB->GetHeight(), -1);

  uint8_t* output = nullptr;
  if (apOutput) {
    niCheck(apImgA->GetWidth() == apOutput->GetWidth(), -1);
    niCheck(apImgA->GetHeight() == apOutput->GetHeight(), -1);
    niCheck(apOutput->GetPixelFormat()->GetBytesPerPixel() == 4, -1);
    output = apOutput->GetData();
  }

  Ptr<iPixelFormat> pxf = this->CreatePixelFormat(IMAGE_PIXELFORMAT);

  QPtr<iBitmap2D> convertedImgA = apImgA;
  if (!convertedImgA->GetPixelFormat()->IsSamePixelFormat(pxf)) {
    convertedImgA = convertedImgA->CreateConvertedFormat(pxf);
    niCheckIsOK(convertedImgA,-1);
  }

  QPtr<iBitmap2D> convertedImgB = apImgB;
  if (!convertedImgB->GetPixelFormat()->IsSamePixelFormat(pxf)) {
    convertedImgB = convertedImgB->CreateConvertedFormat(pxf);
    niCheckIsOK(convertedImgB,-1);
  }

  return _ComputeImageDiff(convertedImgA->GetData(),
                           convertedImgB->GetData(),
                           convertedImgA->GetWidth(),
                           convertedImgA->GetHeight(),
                           output,
                           afThreshold,
                           !!abIgnoreAA);
}
