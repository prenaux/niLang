#ifndef __TRIANGLE_8474326_H__
#define __TRIANGLE_8474326_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

template <class T>
tBool TriangleIntersectTriangle(const T *V0, const T *V1, const T *V2,
                                const T *U0, const T *U1, const T *U2,
                                T epsilon = niEpsilon5);

template <class T>
tBool TriangleIntersectRay(const T *orig, const T *dir,
                           const T *vert0, const T *vert1, const T *vert2,
                           T *outt = NULL, T *outu = NULL, T *outv = NULL, T epsilon = niEpsilon5);

template <typename T>
sVec3<T>& TriangleBaryCentric(sVec3<T>& vOut,
                                 const sVec3<T>& A,
                                 const sVec3<T>& B,
                                 const sVec3<T>& C,
                                 const sVec3<T>& P);

template <class T>
tBool TriangleIntersectRayCull(const T *orig, const T *dir,
                               const T *vert0, const T *vert1, const T *vert2,
                               T *t, T *u, T *v, T epsilon = niEpsilon5);

template <typename T>
inline tF32 TriangleAreaAndNormal(sVec3<T>& avN, const sVec3<T>& avA, const sVec3<T>& avB, const sVec3<T>& avC);

template <typename T>
inline tBool TriangleIsDegenerate(const sVec3<T>& avA, const sVec3<T>& avB, const sVec3<T>& avC);

template <typename T>
inline tBool TriangleIntersectAABB(const sVec3<T>& vAABBMin, const sVec3<T>& vAABBMax,
                                   const sVec3<T>& A, const sVec3<T>& B, const sVec3<T>& C);

//////////////////////////////////////////////////////////////////////////////////////////////
// Use code of:
//-------------
// Triangle/triangle intersection test routine,
// by Tomas Moller, 1997.
// See article "A Fast Triangle-Triangle Intersection Test",
// Journal of Graphics Tools, 2(2), 1997
//////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////
// some macros
#define CROSS(dest,v1,v2)                       \
  dest[0]=v1[1]*v2[2]-v1[2]*v2[1];              \
  dest[1]=v1[2]*v2[0]-v1[0]*v2[2];              \
  dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) {                       \
    dest[0]=v1[0]-v2[0];                        \
    dest[1]=v1[1]-v2[1];                        \
    dest[2]=v1[2]-v2[2];                        \
  }

// sort so that a<=b
#define SORT(a,b) if(a>b) { T c_; c_=a; a=b;  b=c_; }

#define ISECT(VV0,VV1,VV2,D0,D1,D2,isect0,isect1) \
  isect0=VV0+(VV1-VV0)*D0/(D0-D1);                \
  isect1=VV0+(VV2-VV0)*D0/(D0-D2);

#define COMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,isect0,isect1) \
  if(D0D1>T(0)) {                                                       \
    /* here we know that D0D2<=0.0 */                                   \
    /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
    ISECT(VV2,VV0,VV1,D2,D0,D1,isect0,isect1);                          \
  }                                                                     \
  else if(D0D2>T(0))  {                                                 \
    /* here we know that d0d1<=0.0 */                                   \
    ISECT(VV1,VV0,VV2,D1,D0,D2,isect0,isect1);                          \
  }                                                                     \
  else if(D1*D2>T(0) || D0!=T(0)) {                                     \
    /* here we know that d0d1<=0.0 or that D0!=0.0 */                   \
    ISECT(VV0,VV1,VV2,D0,D1,D2,isect0,isect1);                          \
  }                                                                     \
  else if(D1!=T(0)) {                                                   \
    ISECT(VV1,VV0,VV2,D1,D0,D2,isect0,isect1);                          \
  }                                                                     \
  else if(D2!=T(0)) {                                                   \
    ISECT(VV2,VV0,VV1,D2,D0,D1,isect0,isect1);                          \
  }                                                                     \
  else  {                                                               \
    /* triangles are coplanar */                                        \
    /*return CoplanarTriTri(N1,V0,V1,V2,U0,U1,U2);*/                    \
    T A[3];                                                             \
    short i0,i1;                                                        \
    /* first project onto an axis-aligned plane, that maximizes the area */ \
    /* of the triangles, compute indices: i0,i1. */                     \
    A[0]=fabsf(N1[0]);                                                  \
    A[1]=fabsf(N1[1]);                                                  \
    A[2]=fabsf(N1[2]);                                                  \
    if(A[0]>A[1]) {                                                     \
      if(A[0]>A[2]) {                                                   \
        i0=1;      /* A[0] is greatest */                               \
        i1=2;                                                           \
      }                                                                 \
      else  {                                                           \
        i0=0;      /* A[2] is greatest */                               \
        i1=1;                                                           \
      }                                                                 \
    }                                                                   \
    else  { /* A[0]<=A[1] */                                            \
      if(A[2]>A[1]) {                                                   \
        i0=0;      /* A[2] is greatest */                               \
        i1=1;                                                           \
      }                                                                 \
      else  {                                                           \
        i0=0;      /* A[1] is greatest */                               \
        i1=2;                                                           \
      }                                                                 \
    }                                                                   \
    /* test all edges of triangle 1 against the edges of triangle 2 */  \
    EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2);                             \
    EDGE_AGAINST_TRI_EDGES(V1,V2,U0,U1,U2);                             \
    EDGE_AGAINST_TRI_EDGES(V2,V0,U0,U1,U2);                             \
    /* finally, test if tri1 is totally contained in tri2 or vice versa */ \
    POINT_IN_TRI(V0,U0,U1,U2);                                          \
    POINT_IN_TRI(U0,V0,V1,V2);                                          \
    return eFalse;                                                      \
  }


// this edge to edge test is based on Franlin Antonio's gem:
// "Faster Line Segment Intersection", in Graphics Gems III,
// pp. 199-202
#define EDGE_EDGE_TEST(V0,U0,U1)                      \
  Bx=U0[i0]-U1[i0];                                   \
  By=U0[i1]-U1[i1];                                   \
  Cx=V0[i0]-U0[i0];                                   \
  Cy=V0[i1]-U0[i1];                                   \
  f=Ay*Bx-Ax*By;                                      \
  d=By*Cx-Bx*Cy;                                      \
  if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
  {                                                   \
    e=Ax*Cy-Ay*Cx;                                    \
    if(f>0) {                                         \
      if(e>=0 && e<=f) return eTrue;                  \
    }                                                 \
    else {                                            \
      if(e<=0 && e>=f) return eTrue;                  \
    }                                                 \
  }

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2)  \
  {                                             \
    T Ax,Ay,Bx,By,Cx,Cy,e,d,f;                  \
    Ax=V1[i0]-V0[i0];                           \
    Ay=V1[i1]-V0[i1];                           \
    /* test edge U0,U1 against V0,V1 */         \
    EDGE_EDGE_TEST(V0,U0,U1);                   \
    /* test edge U1,U2 against V0,V1 */         \
    EDGE_EDGE_TEST(V0,U1,U2);                   \
    /* test edge U2,U1 against V0,V1 */         \
    EDGE_EDGE_TEST(V0,U2,U0);                   \
  }

#define POINT_IN_TRI(V0,U0,U1,U2)               \
  {                                             \
    T a_,b_,c_,d0_,d1_,d2_;                     \
    /* is T1 completly inside T2? */            \
    /* check if V0 is inside tri(U0,U1,U2) */   \
    a_=U1[i1]-U0[i1];                           \
    b_=-(U1[i0]-U0[i0]);                        \
    c_=-a_*U0[i0]-b_*U0[i1];                    \
    d0_=a_*V0[i0]+b_*V0[i1]+c_;                 \
                                                \
    a_=U2[i1]-U1[i1];                           \
    b_=-(U2[i0]-U1[i0]);                        \
    c_=-a_*U1[i0]-b_*U1[i1];                    \
    d1_=a_*V0[i0]+b_*V0[i1]+c_;                 \
                                                \
    a_=U0[i1]-U2[i1];                           \
    b_=-(U0[i0]-U2[i0]);                        \
    c_=-a_*U2[i0]-b_*U2[i1];                    \
    d2_=a_*V0[i0]+b_*V0[i1]+c_;                 \
    if(d0_*d1_>0.0) {                           \
      if(d0_*d2_>0.0) return eTrue;             \
    }                                           \
  }

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////
template <class T>
tBool TriangleIntersectTriangle(const T *V0, const T *V1, const T *V2,
                                const T *U0, const T *U1, const T *U2,
                                T epsilon)
{
  T E1[3],E2[3];
  T N1[3],N2[3],d1,d2;
  T du0,du1,du2,dv0,dv1,dv2;
  T D[3];
  T isect1[2], isect2[2];
  T du0du1,du0du2,dv0dv1,dv0dv2;
  short index;
  T vp0,vp1,vp2;
  T up0,up1,up2;
  T b,c,max;

  // compute plane equation of triangle(V0,V1,V2)
  SUB(E1,V1,V0);
  SUB(E2,V2,V0);
  CROSS(N1,E1,E2);
  d1=-DOT(N1,V0);

  // plane equation 1: N1.X+d1=0

  // put U0,U1,U2 into plane equation 1 to compute signed distances to the plane
  du0=DOT(N1,U0)+d1;
  du1=DOT(N1,U1)+d1;
  du2=DOT(N1,U2)+d1;

  // coplanarity robustness check
  if(fabsf(du0) < epsilon) du0=0.0;
  if(fabsf(du1) < epsilon) du1=0.0;
  if(fabsf(du2) < epsilon) du2=0.0;

  du0du1=du0*du1;
  du0du2=du0*du2;

  if(du0du1>T(0) && du0du2>T(0)) // same sign on all of them + not equal 0 ?
    return eFalse;              // no intersection occurs

  // compute plane of triangle (U0,U1,U2)
  SUB(E1,U1,U0);
  SUB(E2,U2,U0);
  CROSS(N2,E1,E2);
  d2=-DOT(N2,U0);

  // plane equation 2: N2.X+d2=0

  // put V0,V1,V2 into plane equation 2
  dv0=DOT(N2,V0)+d2;
  dv1=DOT(N2,V1)+d2;
  dv2=DOT(N2,V2)+d2;

  if(fabsf(dv0) < epsilon) dv0 = 0.0;
  if(fabsf(dv1) < epsilon) dv1 = 0.0;
  if(fabsf(dv2) < epsilon) dv2 = 0.0;

  dv0dv1=dv0*dv1;
  dv0dv2=dv0*dv2;

  if(dv0dv1>T(0) && dv0dv2>T(0)) // same sign on all of them + not equal 0 ?
    return eFalse;              // no intersection occurs

  // compute direction of intersection line
  CROSS(D,N1,N2);

  // compute and index to the largest component of D
  max=fabsf(D[0]);
  index=0;
  b=fabsf(D[1]);
  c=fabsf(D[2]);
  if(b>max) max=b,index=1;
  if(c>max) max=c,index=2;

  // this is the simplified projection onto L
  vp0=V0[index];
  vp1=V1[index];
  vp2=V2[index];

  up0=U0[index];
  up1=U1[index];
  up2=U2[index];

  // compute interval for triangle 1
  COMPUTE_INTERVALS(vp0,vp1,vp2,dv0,dv1,dv2,dv0dv1,dv0dv2,isect1[0],isect1[1]);

  // compute interval for triangle 2
  COMPUTE_INTERVALS(up0,up1,up2,du0,du1,du2,du0du1,du0du2,isect2[0],isect2[1]);

  SORT(isect1[0],isect1[1]);
  SORT(isect2[0],isect2[1]);

  if(isect1[1]<isect2[0] || isect2[1]<isect1[0])
    return eFalse;

  return eTrue;
}

///////////////////////////////////////////////
template <class T>
tBool TriangleIntersectRay(const T *orig, const T *dir,
                           const T *vert0, const T *vert1, const T *vert2,
                           T *outt, T *outu, T *outv, T epsilon)
{
  T edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
  T det,inv_det;
  T u, v;

  // find vectors for two edges sharing vert0
  SUB(edge1, vert1, vert0);
  SUB(edge2, vert2, vert0);

  // begin calculating determinant - also used to calculate U parameter
  CROSS(pvec, dir, edge2);

  // if determinant is near zero, ray lies in plane of triangle
  det = DOT(edge1, pvec);

  if(det > -epsilon && det < epsilon)
    return eFalse;
  inv_det = T(1) / det;

  // calculate distance from vert0 to ray origin
  SUB(tvec, orig, vert0);

  // calculate U parameter and test bounds
  u = DOT(tvec, pvec) * inv_det;
  if(outu) *outu = u;
  if(u < 0.0 || u > T(1))
    return eFalse;

  // prepare to test V parameter
  CROSS(qvec, tvec, edge1);

  // calculate V parameter and test bounds
  v = DOT(dir, qvec) * inv_det;
  if(outv) *outv = v;
  if(v < 0.0 || u + v > T(1))
    return eFalse;

  if(outt)
  {
    // calculate t, ray intersects triangle
    *outt = DOT(edge2, qvec) * inv_det;
  }

  return eTrue;
}

///////////////////////////////////////////////
template <class T>
tBool TriangleIntersectRayCull(const T *orig, const T *dir,
                               const T *vert0, const T *vert1, const T *vert2,
                               T *t, T *u, T *v, T epsilon)
{
  T edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
  T det,inv_det;

  // find vectors for two edges sharing vert0
  SUB(edge1, vert1, vert0);
  SUB(edge2, vert2, vert0);

  // begin calculating determinant - also used to calculate U parameter
  CROSS(pvec, dir, edge2);

  // if determinant is near zero, ray lies in plane of triangle
  det = DOT(edge1, pvec);

  if(det < epsilon)
    return eFalse;

  // calculate distance from vert0 to ray origin
  SUB(tvec, orig, vert0);

  // calculate U parameter and test bounds
  *u = DOT(tvec, pvec);
  if(*u < 0.0 || *u > det)
    return eFalse;

  // prepare to test V parameter
  CROSS(qvec, tvec, edge1);

  // calculate V parameter and test bounds
  *v = DOT(dir, qvec);
  if(*v < 0.0 || *u + *v > det)
    return eFalse;

  // calculate t, scale parameters, ray intersects triangle
  *t = DOT(edge2, qvec);
  inv_det = T(1) / det;
  *t *= inv_det;
  *u *= inv_det;
  *v *= inv_det;

  return eTrue;
}

///////////////////////////////////////////////
// V2-V3 must be the base of the triangle
template <class T>
T TriangleArea(const sVec3<T>& A,
               const sVec3<T>& B,
               const sVec3<T>& C)
{
  sVec3<T> t;
  return VecLength(VecCross(t, B-A, C-A))*T(0.5);
}

///////////////////////////////////////////////
template <typename T>
sVec3<T>& TriangleBaryCentric(sVec3<T>& vOut,
                                 const sVec3<T>& A,
                                 const sVec3<T>& B,
                                 const sVec3<T>& C,
                                 const sVec3<T>& P)
{
  tF32 fBCP = TriangleArea(B,C,P);
  tF32 fCAP = TriangleArea(C,A,P);
  tF32 fABC = TriangleArea(A,B,C);
  vOut.x = ni::FDiv(fBCP,fABC);
  vOut.y = ni::FDiv(fCAP,fABC);
  vOut.z = T(1) - vOut.x - vOut.y;
  return vOut;
}

///////////////////////////////////////////////
template <typename T>
inline tF32 TriangleAreaAndNormal(sVec3<T>& avN, const sVec3<T>& avA, const sVec3<T>& avB, const sVec3<T>& avC)
{
  sVec3<T> A = avB-avA;
  sVec3<T> B = avC-avA;
  VecCross(avN,A,B);
  tF32 fLen = VecLength(avN);
  avN *= ni::FInvert(fLen);
  return fLen/2.0f;
}

///////////////////////////////////////////////
template <typename T>
inline tBool TriangleIsDegenerate(const sVec3<T>& avA, const sVec3<T>& avB, const sVec3<T>& avC)
{
  if (VecEqual(avA,avB)) return eTrue;
  if (VecEqual(avA,avC)) return eTrue;
  if (VecEqual(avB,avC)) return eTrue;
  return eFalse;
}

#define X 0
#define Y 1
#define Z 2

#define FINDMINMAX(x0,x1,x2,min,max)            \
  min = max = x0;                               \
  if(x1<min) min=x1;                            \
  if(x1>max) max=x1;                            \
  if(x2<min) min=x2;                            \
  if(x2>max) max=x2;

template <typename T>
inline int planeBoxOverlap(T normal[3], T d, T maxbox[3])
{
  int q;
  T vmin[3],vmax[3];
  for (q=X;q<=Z;q++) {
    if (normal[q]>0.0f) {
      vmin[q]=-maxbox[q];
      vmax[q]=maxbox[q];
    }
    else {
      vmin[q]=maxbox[q];
      vmax[q]=-maxbox[q];
    }
  }
  if(DOT(normal,vmin)+d>0.0f) return 0;
  if(DOT(normal,vmax)+d>=0.0f) return 1;
  return 0;
}

/*======================== X-tests ========================*/
#define AXISTEST_X01(a, b, fa, fb)                    \
  p0 = a*v0[Y] - b*v0[Z];                             \
  p2 = a*v2[Y] - b*v2[Z];                             \
  if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}  \
  rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];    \
  if(min>rad || max<-rad) return 0;

#define AXISTEST_X2(a, b, fa, fb)                     \
  p0 = a*v0[Y] - b*v0[Z];                             \
  p1 = a*v1[Y] - b*v1[Z];                             \
  if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}  \
  rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];    \
  if(min>rad || max<-rad) return 0;

/*======================== Y-tests ========================*/
#define AXISTEST_Y02(a, b, fa, fb)                    \
  p0 = -a*v0[X] + b*v0[Z];                            \
  p2 = -a*v2[X] + b*v2[Z];                            \
  if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}  \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];    \
  if(min>rad || max<-rad) return 0;

#define AXISTEST_Y1(a, b, fa, fb)                     \
  p0 = -a*v0[X] + b*v0[Z];                            \
  p1 = -a*v1[X] + b*v1[Z];                            \
  if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}  \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];    \
  if(min>rad || max<-rad) return 0;

/*======================== Z-tests ========================*/
#define AXISTEST_Z12(a, b, fa, fb)                    \
  p1 = a*v1[X] - b*v1[Y];                             \
  p2 = a*v2[X] - b*v2[Y];                             \
  if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}  \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];    \
  if(min>rad || max<-rad) return 0;

#define AXISTEST_Z0(a, b, fa, fb)                     \
  p0 = a*v0[X] - b*v0[Y];                             \
  p1 = a*v1[X] - b*v1[Y];                             \
  if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}  \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];    \
  if(min>rad || max<-rad) return 0;

template <typename T>
inline int triBoxOverlap(T boxcenter[3], T boxhalfsize[3], T triverts[3][3])
{

  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
  T v0[3],v1[3],v2[3];
  T min,max,d,p0,p1,p2,rad,fex,fey,fez;
  T normal[3],e0[3],e1[3],e2[3];

  /* This is the fastest branch on Sun */
  /* move everything so that the boxcenter is in (0,0,0) */
  SUB(v0,triverts[0],boxcenter);
  SUB(v1,triverts[1],boxcenter);
  SUB(v2,triverts[2],boxcenter);

  /* compute triangle edges */
  SUB(e0,v1,v0);      /* tri edge 0 */
  SUB(e1,v2,v1);      /* tri edge 1 */
  SUB(e2,v0,v2);      /* tri edge 2 */

  /* Bullet 3:  */
  /*  test the 9 tests first (this was faster) */
  fex = fabsf(e0[X]);
  fey = fabsf(e0[Y]);
  fez = fabsf(e0[Z]);
  AXISTEST_X01(e0[Z], e0[Y], fez, fey);
  AXISTEST_Y02(e0[Z], e0[X], fez, fex);
  AXISTEST_Z12(e0[Y], e0[X], fey, fex);

  fex = fabsf(e1[X]);
  fey = fabsf(e1[Y]);
  fez = fabsf(e1[Z]);
  AXISTEST_X01(e1[Z], e1[Y], fez, fey);
  AXISTEST_Y02(e1[Z], e1[X], fez, fex);
  AXISTEST_Z0(e1[Y], e1[X], fey, fex);

  fex = fabsf(e2[X]);
  fey = fabsf(e2[Y]);
  fez = fabsf(e2[Z]);
  AXISTEST_X2(e2[Z], e2[Y], fez, fey);
  AXISTEST_Y1(e2[Z], e2[X], fez, fex);
  AXISTEST_Z12(e2[Y], e2[X], fey, fex);

  /* Bullet 1: */
  /*  first test overlap in the {x,y,z}-directions */
  /*  find min, max of the triangle each direction, and test for overlap in */
  /*  that direction -- this is equivalent to testing a minimal AABB around */
  /*  the triangle against the AABB */

  /* test in X-direction */
  FINDMINMAX(v0[X],v1[X],v2[X],min,max);
  if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;

  /* test in Y-direction */
  FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);
  if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;

  /* test in Z-direction */
  FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
  if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;

  /* Bullet 2: */
  /*  test if the box intersects the plane of the triangle */
  /*  compute plane equation of triangle: normal*x+d=0 */
  CROSS(normal,e0,e1);
  d=-DOT(normal,v0);  /* plane eq: normal.x+d=0 */
  if(!planeBoxOverlap(normal,d,boxhalfsize)) return 0;

  return 1;   /* box and triangle overlaps */
}

#undef X
#undef Y
#undef Z

template <typename T>
inline tBool TriangleIntersectAABB(const sVec3<T>& vAABBMin, const sVec3<T>& vAABBMax,
                                   const sVec3<T>& A, const sVec3<T>& B, const sVec3<T>& C)
{
  sVec3<T> vAABBCenter = (vAABBMin + vAABBMax) / 2.0f;
  sVec3<T> vHalfSize = (vAABBMax - vAABBMin) / 2.0f;
  T fTri3D[3][3] = {
    { A.x, A.y, A.z },
    { B.x, B.y, B.z },
    { C.x, C.y, C.z },
  };
  return triBoxOverlap(vAABBCenter.ptr(), vHalfSize.ptr(), fTri3D) == 1;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// undef the macros
#undef CROSS
#undef DOT
#undef SUB
#undef SORT
#undef ISECT
#undef COMPUTE_INTERVALS
#undef EDGE_EDGE_TEST
#undef EDGE_AGAINST_TRI_EDGES
#undef POINT_IN_TRI
#undef AXISTEST_X01
#undef AXISTEST_X2
#undef AXISTEST_Y02
#undef AXISTEST_Y1
#undef AXISTEST_Z12
#undef AXISTEST_Z0

/// EOF //////////////////////////////////////////////////////////////////////////////////////
}; // End of ni
#endif // __TRIANGLE_8474326_H__
