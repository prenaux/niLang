// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
::Import("lang.ni")

::math <- {
  function getAngle2D(aPos,aLookAt) {
    return (::gMath.GetAngleFromPoints2D(aPos,aLookAt))
  }

  function getDir2D(aPos,aLookAt) {
    local a = getAngle2D(aPos,aLookAt)
    return ::Vec2(::gMath.Cos(a),::gMath.Sin(a))
  }

  function getLookAtMatrix2D(aPos,aLookAt) {
    local a = getAngle2D(aPos,aLookAt)
    return  ::gMath.MatrixTranslation(::Vec3(-aPos.x,-aPos.y,0)) *
      ::gMath.MatrixRotationZ(a) *
      ::gMath.MatrixTranslation(::Vec3(aPos.x,aPos.y,0))
  }

  function getLookAtMatrixFromAngle2D(aPos,a) {
    return  ::gMath.MatrixTranslation(::Vec3(-aPos.x,-aPos.y,0)) *
      ::gMath.MatrixRotationZ(a) *
      ::gMath.MatrixTranslation(::Vec3(aPos.x,aPos.y,0))
  }

  function triangleNormalArea(a,b,c) {
    local A = b-a;
    local B = c-a;
    local N = A.cross(B)
    return N
  }
  function triangleNormal(a,b,c) {
    local A = b-a;
    local B = c-a;
    local N = A.cross(B)
    //         local area = N.length()/2
    N = N.normalize()
    return N
  }
}
