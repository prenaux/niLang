#pragma once
#ifndef __IMAGE_H_7B6244CE_71AD_4801_A7B9_F6B06B4A7458__
#define __IMAGE_H_7B6244CE_71AD_4801_A7B9_F6B06B4A7458__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#define IMAGE_PIXELFORMAT "R8G8B8A8"

#ifdef _DEBUG
void __Image_SetDebugName(ni::iImage* apImg, const ni::achar* aaszDebugName);
#define Image_SetDebugName(I,DBG) __Image_SetDebugName(I,DBG)
#else
#define Image_SetDebugName(I,DBG)
#endif

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __IMAGE_H_7B6244CE_71AD_4801_A7B9_F6B06B4A7458__
