#ifndef __GEOMETRYQUADRIC_47289368_H__
#define __GEOMETRYQUADRIC_47289368_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

iGeometry*
QuadricCylinder(iGraphics* apGraphics, tGeometryCreateFlags aFlags, tFVF aFVF,
                tF32 baseRadius, tF32 topRadius,
                tF32 height, tI32 slices, tI32 stacks,
                tBool abCW, tU32 aulColor, const sMatrixf& amtxUV, tBool abCap, tBool abCentered);

iGeometry*
QuadricSphere(iGraphics* apGraphics, tGeometryCreateFlags aFlags, tFVF aFVF,
              tF32 radius, tI32 slices, tI32 stacks,
              tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);

iGeometry*
QuadricDiskSweep(iGraphics* apGraphics, tGeometryCreateFlags aFlags, tFVF aFVF,
                 tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 loops, tF32 startAngle, tF32 sweepAngle,
                 tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);

tBool
QuadricDiskSweepEx(iGeometry* apGeom,
                   tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 loops, tF32 startAngle, tF32 sweepAngle,
                   tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);

iGeometry*
QuadricDisk(iGraphics* apGraphics, tGeometryCreateFlags aFlags, tFVF aFVF,
            tF32 innerRadius, tF32 outerRadius, tI32 slices, tI32 loops,
            tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);

iGeometry* GeoSphere(iGraphics* apGraphics, tGeometryCreateFlags aFlags, tFVF aFVF,
                              int type, tBool abHemi, tF32 radius, tI32 slices,
                              tBool abCW, tU32 aulColor, const sMatrixf& amtxUV);

/// EOF //////////////////////////////////////////////////////////////////////////////////////
#endif // __GEOMETRYQUADRIC_47289368_H__
