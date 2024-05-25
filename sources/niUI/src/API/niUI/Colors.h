#ifndef __COLORS_H_FC927A9B_0890_4951_8AE7_EAF1B89479B3__
#define __COLORS_H_FC927A9B_0890_4951_8AE7_EAF1B89479B3__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include <niLang/Types.h>

namespace ni {
/** \addtogroup niUI
 * @{
 */

//! Color spaces
enum eColorSpace {
  //! RGB color space
  eColorSpace_RGB = 0,
  //! CMY color space
  eColorSpace_CMY = 1,
  //! CMYK color space
  eColorSpace_CMYK = 2,
  //! HSV color space
  eColorSpace_HSV = 3,
  //! HSL color space
  eColorSpace_HSL = 4,
  //! XYZ color space
  eColorSpace_XYZ = 5,
  //! Yxy color space
  eColorSpace_Yxy = 6,
  //! Hunter lab color space
  eColorSpace_HunterLab = 7,
  //! CIE Lab color space
  eColorSpace_CIELab = 8,
  //! CIE LCH color space
  eColorSpace_CIELCH = 9,
  //! CIE Luv color space
  eColorSpace_CIELuv = 10,
  //! YIQ color space (NTSC/USA TV).
  eColorSpace_YIQ = 11,
  //! YUV color space (PAL/Europe TV).
  eColorSpace_YUV = 12,
  //! YCbCr color space.
  eColorSpace_YCbCr = 13,
  //! YPbPr color space (HDTV).
  eColorSpace_YPbPr = 14,
  //! YCoCg color space (H264 & AVT).
  eColorSpace_YCoCg = 15,
  //! \internal
  eColorSpace_Last = 16,
  //! \internal
  eColorSpace_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

enum eColor {
  eColor_aliceblue = niBuildColor(240,248,255, 255),
  eColor_antiquewhite = niBuildColor(250,235,215, 255),
  eColor_aqua = niBuildColor(0,255,255, 255),
  eColor_aquamarine = niBuildColor(127,255,212, 255),
  eColor_azure = niBuildColor(240,255,255, 255),
  eColor_beige = niBuildColor(245,245,220, 255),
  eColor_bisque = niBuildColor(255,228,196, 255),
  eColor_black = niBuildColor(0,0,0, 255),
  eColor_blanchedalmond = niBuildColor(255,235,205, 255),
  eColor_blue = niBuildColor(0,0,255, 255),
  eColor_blueviolet = niBuildColor(138,43,226, 255),
  eColor_brown = niBuildColor(165,42,42, 255),
  eColor_burlywood = niBuildColor(222,184,135, 255),
  eColor_cadetblue = niBuildColor(95,158,160, 255),
  eColor_chartreuse = niBuildColor(127,255,0, 255),
  eColor_chocolate = niBuildColor(210,105,30, 255),
  eColor_coral = niBuildColor(255,127,80, 255),
  eColor_cornflowerblue = niBuildColor(100,149,237, 255),
  eColor_cornsilk = niBuildColor(255,248,220, 255),
  eColor_crimson = niBuildColor(220,20,60, 255),
  eColor_cyan = niBuildColor(0,255,255, 255),
  eColor_darkblue = niBuildColor(0,0,139, 255),
  eColor_darkcyan = niBuildColor(0,139,139, 255),
  eColor_darkgoldenrod = niBuildColor(184,134,11, 255),
  eColor_darkgray = niBuildColor(169,169,169, 255),
  eColor_darkgreen = niBuildColor(0,100,0, 255),
  eColor_darkgrey = niBuildColor(169,169,169, 255),
  eColor_darkkhaki = niBuildColor(189,183,107, 255),
  eColor_darkmagenta = niBuildColor(139,0,139, 255),
  eColor_darkolivegreen = niBuildColor(85,107,47, 255),
  eColor_darkorange = niBuildColor(255,140,0, 255),
  eColor_darkorchid = niBuildColor(153,50,204, 255),
  eColor_darkred = niBuildColor(139,0,0, 255),
  eColor_darksalmon = niBuildColor(233,150,122, 255),
  eColor_darkseagreen = niBuildColor(143,188,143, 255),
  eColor_darkslateblue = niBuildColor(72,61,139, 255),
  eColor_darkslategray = niBuildColor(47,79,79, 255),
  eColor_darkslategrey = niBuildColor(47,79,79, 255),
  eColor_darkturquoise = niBuildColor(0,206,209, 255),
  eColor_darkviolet = niBuildColor(148,0,211, 255),
  eColor_deeppink = niBuildColor(255,20,147, 255),
  eColor_deepskyblue = niBuildColor(0,191,255, 255),
  eColor_dimgray = niBuildColor(105,105,105, 255),
  eColor_dimgrey = niBuildColor(105,105,105, 255),
  eColor_dodgerblue = niBuildColor(30,144,255, 255),
  eColor_firebrick = niBuildColor(178,34,34, 255),
  eColor_floralwhite = niBuildColor(255,250,240, 255),
  eColor_forestgreen = niBuildColor(34,139,34, 255),
  eColor_fuchsia = niBuildColor(255,0,255, 255),
  eColor_gainsboro = niBuildColor(220,220,220, 255),
  eColor_ghostwhite = niBuildColor(248,248,255, 255),
  eColor_gold = niBuildColor(255,215,0, 255),
  eColor_goldenrod = niBuildColor(218,165,32, 255),
  eColor_gray = niBuildColor(128,128,128, 255),
  eColor_green = niBuildColor(0,128,0, 255),
  eColor_greenyellow = niBuildColor(173,255,47, 255),
  eColor_grey = niBuildColor(128,128,128, 255),
  eColor_honeydew = niBuildColor(240,255,240, 255),
  eColor_hotpink = niBuildColor(255,105,180, 255),
  eColor_indianred = niBuildColor(205,92,92, 255),
  eColor_indigo = niBuildColor(75,0,130, 255),
  eColor_ivory = niBuildColor(255,255,240, 255),
  eColor_khaki = niBuildColor(240,230,140, 255),
  eColor_lavender = niBuildColor(230,230,250, 255),
  eColor_lavenderblush = niBuildColor(255,240,245, 255),
  eColor_lawngreen = niBuildColor(124,252,0, 255),
  eColor_lemonchiffon = niBuildColor(255,250,205, 255),
  eColor_lightblue = niBuildColor(173,216,230, 255),
  eColor_lightcoral = niBuildColor(240,128,128, 255),
  eColor_lightcyan = niBuildColor(224,255,255, 255),
  eColor_lightgoldenrodyellow = niBuildColor(250,250,210, 255),
  eColor_lightgray = niBuildColor(211,211,211, 255),
  eColor_lightgreen = niBuildColor(144,238,144, 255),
  eColor_lightgrey = niBuildColor(211,211,211, 255),
  eColor_lightpink = niBuildColor(255,182,193, 255),
  eColor_lightsalmon = niBuildColor(255,160,122, 255),
  eColor_lightseagreen = niBuildColor(32,178,170, 255),
  eColor_lightskyblue = niBuildColor(135,206,250, 255),
  eColor_lightslategray = niBuildColor(119,136,153, 255),
  eColor_lightslategrey = niBuildColor(119,136,153, 255),
  eColor_lightsteelblue = niBuildColor(176,196,222, 255),
  eColor_lightyellow = niBuildColor(255,255,224, 255),
  eColor_lime = niBuildColor(0,255,0, 255),
  eColor_limegreen = niBuildColor(50,205,50, 255),
  eColor_linen = niBuildColor(250,240,230, 255),
  eColor_magenta = niBuildColor(255,0,255, 255),
  eColor_maroon = niBuildColor(128,0,0, 255),
  eColor_mediumaquamarine = niBuildColor(102,205,170, 255),
  eColor_mediumblue = niBuildColor(0,0,205, 255),
  eColor_mediumorchid = niBuildColor(186,85,211, 255),
  eColor_mediumpurple = niBuildColor(147,112,219, 255),
  eColor_mediumseagreen = niBuildColor(60,179,113, 255),
  eColor_mediumslateblue = niBuildColor(123,104,238, 255),
  eColor_mediumspringgreen = niBuildColor(0,250,154, 255),
  eColor_mediumturquoise = niBuildColor(72,209,204, 255),
  eColor_mediumvioletred = niBuildColor(199,21,133, 255),
  eColor_midnightblue = niBuildColor(25,25,112, 255),
  eColor_mintcream = niBuildColor(245,255,250, 255),
  eColor_mistyrose = niBuildColor(255,228,225, 255),
  eColor_moccasin = niBuildColor(255,228,181, 255),
  eColor_navajowhite = niBuildColor(255,222,173, 255),
  eColor_navy = niBuildColor(0,0,128, 255),
  eColor_oldlace = niBuildColor(253,245,230, 255),
  eColor_olive = niBuildColor(128,128,0, 255),
  eColor_olivedrab = niBuildColor(107,142,35, 255),
  eColor_orange = niBuildColor(255,165,0, 255),
  eColor_orangered = niBuildColor(255,69,0, 255),
  eColor_orchid = niBuildColor(218,112,214, 255),
  eColor_palegoldenrod = niBuildColor(238,232,170, 255),
  eColor_palegreen = niBuildColor(152,251,152, 255),
  eColor_paleturquoise = niBuildColor(175,238,238, 255),
  eColor_palevioletred = niBuildColor(219,112,147, 255),
  eColor_papayawhip = niBuildColor(255,239,213, 255),
  eColor_peachpuff = niBuildColor(255,218,185, 255),
  eColor_peru = niBuildColor(205,133,63, 255),
  eColor_pink = niBuildColor(255,192,203, 255),
  eColor_plum = niBuildColor(221,160,221, 255),
  eColor_powderblue = niBuildColor(176,224,230, 255),
  eColor_purple = niBuildColor(128,0,128, 255),
  eColor_red = niBuildColor(255,0,0, 255),
  eColor_rosybrown = niBuildColor(188,143,143, 255),
  eColor_royalblue = niBuildColor(65,105,225, 255),
  eColor_saddlebrown = niBuildColor(139,69,19, 255),
  eColor_salmon = niBuildColor(250,128,114, 255),
  eColor_sandybrown = niBuildColor(244,164,96, 255),
  eColor_seagreen = niBuildColor(46,139,87, 255),
  eColor_seashell = niBuildColor(255,245,238, 255),
  eColor_sienna = niBuildColor(160,82,45, 255),
  eColor_silver = niBuildColor(192,192,192, 255),
  eColor_skyblue = niBuildColor(135,206,235, 255),
  eColor_slateblue = niBuildColor(106,90,205, 255),
  eColor_slategray = niBuildColor(112,128,144, 255),
  eColor_slategrey = niBuildColor(112,128,144, 255),
  eColor_snow = niBuildColor(255,250,250, 255),
  eColor_springgreen = niBuildColor(0,255,127, 255),
  eColor_steelblue = niBuildColor(70,130,180, 255),
  eColor_tan = niBuildColor(210,180,140, 255),
  eColor_teal = niBuildColor(0,128,128, 255),
  eColor_thistle = niBuildColor(216,191,216, 255),
  eColor_tomato = niBuildColor(255,99,71, 255),
  eColor_turquoise = niBuildColor(64,224,208, 255),
  eColor_violet = niBuildColor(238,130,238, 255),
  eColor_wheat = niBuildColor(245,222,179, 255),
  eColor_white = niBuildColor(255,255,255, 255),
  eColor_whitesmoke = niBuildColor(245,245,245, 255),
  eColor_yellow = niBuildColor(255,255,0, 255),
  eColor_yellowgreen = niBuildColor(154,205,50, 255),
  eColor_zzzzzzzzzzz = niBuildColor(0,0,0,0),
  eColor_zzzz = niBuildColor(0,0,0,0),
  //! \internal
  eColor_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};


// Observer= 2, Illuminant= D65
const sVec4f kcolXYZRef2D65 = { 95.047f,100.0f,108.883f,0.0f };
const sVec3f kvColorLuminanceR21G71B07 = { 0.2125f,0.7154f,0.0721f };
const sVec3f kvColorLuminanceR27G67B06 = { 0.27f,0.67f,0.06f };

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __COLORS_H_FC927A9B_0890_4951_8AE7_EAF1B89479B3__
