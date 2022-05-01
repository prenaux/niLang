// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
namespace ni {

///////////////////////////////////////////////
template <typename T, typename BASE>
cFrustumImpl<T,BASE>::cFrustumImpl()
{
	ZeroMembers();
}

///////////////////////////////////////////////
template <typename T, typename BASE>
cFrustumImpl<T,BASE>::~cFrustumImpl()
{
}

///////////////////////////////////////////////
//! Create a copy of this object.
template <typename T, typename BASE>
iFrustum* __stdcall cFrustumImpl<T,BASE>::Clone() const
{
	cFrustumImpl* pNew = niNew cFrustumImpl();
	pNew->mvPlanes = mvPlanes;
	return pNew;
}

///////////////////////////////////////////////
//! Copy the given frustum.
template <typename T, typename BASE>
void __stdcall cFrustumImpl<T,BASE>::Copy(const iFrustum* apSrc)
{
	SetPlanes(apSrc->GetNumPlanes(), apSrc->GetPlanes());
}

///////////////////////////////////////////////
template <typename T, typename BASE>
void cFrustumImpl<T,BASE>::ZeroMembers()
{
}

///////////////////////////////////////////////
template <typename T, typename BASE>
tBool cFrustumImpl<T,BASE>::IsOK() const
{
	return eTrue;
}

///////////////////////////////////////////////
template <typename T, typename BASE>
sVec3<T> __stdcall cFrustumImpl<T,BASE>::GetForward() const
{
	niCheckSilent(GetNumPlanes() >= eFrustumPlane_Near,kvec3fZero);
	return -mvPlanes[eFrustumPlane_Near].GetNormal();
}

///////////////////////////////////////////////
template <typename T, typename BASE>
sVec3<T> __stdcall cFrustumImpl<T,BASE>::GetUp() const
{
	niCheckSilent(GetNumPlanes() >= eFrustumPlane_Right,kvec3fZero);
	sVec3<T> r;
	VecCross(r,
             mvPlanes[eFrustumPlane_Right].GetNormal(),
             mvPlanes[eFrustumPlane_Left].GetNormal());
	VecNormalize(r);
	return r;
}

///////////////////////////////////////////////
template <typename T, typename BASE>
sVec3<T> __stdcall cFrustumImpl<T,BASE>::GetRight() const
{
	niCheckSilent(GetNumPlanes() >= eFrustumPlane_Bottom,kvec3fZero);
	sVec3<T> r;
	VecCross(r,
             mvPlanes[eFrustumPlane_Bottom].GetNormal(),
             mvPlanes[eFrustumPlane_Top].GetNormal());
	VecNormalize(r);
	return r;
}

///////////////////////////////////////////////
//! Extract the planes from a view projection matrix.
template <typename T, typename BASE>
void cFrustumImpl<T,BASE>::ExtractPlanes(const sMatrix<T>& mtxVP)
{
	if (GetNumPlanes() < 6) {
		SetNumPlanes(6);
	}

	// Near clipping plane
    mvPlanes[eFrustumPlane_Near].x = -mtxVP._13;
    mvPlanes[eFrustumPlane_Near].y = -mtxVP._23;
    mvPlanes[eFrustumPlane_Near].z = -mtxVP._33;
    mvPlanes[eFrustumPlane_Near].w = -mtxVP._43;
    PlaneNormalize(mvPlanes[eFrustumPlane_Near]);

	// Far clipping plane
	mvPlanes[eFrustumPlane_Far].x = -(mtxVP._14 - mtxVP._13);
	mvPlanes[eFrustumPlane_Far].y = -(mtxVP._24 - mtxVP._23);
	mvPlanes[eFrustumPlane_Far].z = -(mtxVP._34 - mtxVP._33);
	mvPlanes[eFrustumPlane_Far].w = -(mtxVP._44 - mtxVP._43);
	PlaneNormalize(mvPlanes[eFrustumPlane_Far]);

	// Left clipping plane
	mvPlanes[eFrustumPlane_Left].x = -(mtxVP._14 + mtxVP._11);
	mvPlanes[eFrustumPlane_Left].y = -(mtxVP._24 + mtxVP._21);
	mvPlanes[eFrustumPlane_Left].z = -(mtxVP._34 + mtxVP._31);
	mvPlanes[eFrustumPlane_Left].w = -(mtxVP._44 + mtxVP._41);
	PlaneNormalize(mvPlanes[eFrustumPlane_Left]);

	// Right clipping plane
	mvPlanes[eFrustumPlane_Right].x = -(mtxVP._14 - mtxVP._11);
	mvPlanes[eFrustumPlane_Right].y = -(mtxVP._24 - mtxVP._21);
	mvPlanes[eFrustumPlane_Right].z = -(mtxVP._34 - mtxVP._31);
	mvPlanes[eFrustumPlane_Right].w = -(mtxVP._44 - mtxVP._41);
	PlaneNormalize(mvPlanes[eFrustumPlane_Right]);

	// Top clipping plane
	mvPlanes[eFrustumPlane_Top].x = -(mtxVP._14 - mtxVP._12);
	mvPlanes[eFrustumPlane_Top].y = -(mtxVP._24 - mtxVP._22);
	mvPlanes[eFrustumPlane_Top].z = -(mtxVP._34 - mtxVP._32);
	mvPlanes[eFrustumPlane_Top].w = -(mtxVP._44 - mtxVP._42);
	PlaneNormalize(mvPlanes[eFrustumPlane_Top]);

	// Bottom clipping plane
	mvPlanes[eFrustumPlane_Bottom].x = -(mtxVP._14 + mtxVP._12);
	mvPlanes[eFrustumPlane_Bottom].y = -(mtxVP._24 + mtxVP._22);
	mvPlanes[eFrustumPlane_Bottom].z = -(mtxVP._34 + mtxVP._32);
	mvPlanes[eFrustumPlane_Bottom].w = -(mtxVP._44 + mtxVP._42);
	PlaneNormalize(mvPlanes[eFrustumPlane_Bottom]);

	UpdateBoundingVolume();
}

///////////////////////////////////////////////
//! Set the number of plane in the frustum.
template <typename T, typename BASE>
void cFrustumImpl<T,BASE>::SetNumPlanes(tU32 ulNumPlane)
{
	if (mvPlanes.size() == ulNumPlane) return;
	if (ulNumPlane == 0)	mvPlanes.clear();
	else					mvPlanes.resize(ulNumPlane);
}

///////////////////////////////////////////////
//! Get the number of planes in the frustum.
template <typename T, typename BASE>
tU32 cFrustumImpl<T,BASE>::GetNumPlanes() const
{
	return (tU32)mvPlanes.size();
}

///////////////////////////////////////////////
//! Add the given number of planes to the frustum.
template <typename T, typename BASE>
void cFrustumImpl<T,BASE>::AddPlanes(tU32 aulNumPlane, const sVec4<T>* apPlanes)
{
	if (!aulNumPlane)
		return;

	tU32 aulOldSize = (tU32)mvPlanes.size();
	mvPlanes.resize(aulOldSize+aulNumPlane);
	if (apPlanes)
	{
		for (tU32 i = 0; i < aulNumPlane; ++i)
			mvPlanes[aulOldSize+i] = apPlanes[i];
	}
}

///////////////////////////////////////////////
//! Add one plane to the frustum.
template <typename T, typename BASE>
void cFrustumImpl<T,BASE>::AddPlane(const sVec4<T>& aPlane)
{
	AddPlanes(1,&aPlane);
}

///////////////////////////////////////////////
//! Set all planes of the frustum.
template <typename T, typename BASE>
void cFrustumImpl<T,BASE>::SetPlanes(tU32 aulNumPlane, const sVec4<T>* apPlanes)
{
	SetNumPlanes(aulNumPlane);
	if (apPlanes)
	{
		for (tU32 i = 0; i < aulNumPlane; ++i)
			mvPlanes[i] = apPlanes[i];
		UpdateBoundingVolume();
	}
}

///////////////////////////////////////////////
//! Set the plane of the given index.
template <typename T, typename BASE>
void cFrustumImpl<T,BASE>::SetPlane(tU32 ulIdx, const sVec4<T>& Plane)
{
	niAssert(ulIdx < GetNumPlanes());
	mvPlanes[ulIdx] = Plane;
	UpdateBoundingVolume();
}

///////////////////////////////////////////////
//! Get the plane of the given index.
template <typename T, typename BASE>
sVec4<T> cFrustumImpl<T,BASE>::GetPlane(tU32 ulIdx) const
{
	niAssert(ulIdx < GetNumPlanes());
	return mvPlanes[ulIdx];
}

///////////////////////////////////////////////
//! Get all planes.
template <typename T, typename BASE>
sVec4<T>* cFrustumImpl<T,BASE>::GetPlanes() const
{
	return const_cast<sVec4<T>*>(&mvPlanes[0]);
}

///////////////////////////////////////////////
//! Cull an AABB.
template <typename T, typename BASE>
eCullCode cFrustumImpl<T,BASE>::CullAABB(const sVec3f& vMin, const sVec3f& vMax) const
{
	if (mvPlanes.empty())
		return eCullCode_In;	// No planes in the frustum means that it's an infinie (360 degres frustum)

    sVec3<T>	minExtreme, maxExtreme;
    tBool		intersect;

    // Initialize the intersection indicator.
    intersect = eFalse;

    // For all planes
    for (tPlaneVecCIt it = mvPlanes.begin(); it != mvPlanes.end(); ++it)
    {
        // Find the minimum and maximum extreme points along the plane's
        // normal vector. Just understand this normal as a line of all
        // real numbers. 0 then lies on the plane, negative numbers are
        // in the halfspace opposing the normal, and positive numbers
        // are in the other halfspace. The terms "minimum" and "maximum"
        // should now make sense.
		const sVec3<T> vNormal = it->GetNormal();

		// for x
		if(vNormal.x >= 0)
		{
			minExtreme.x = vMin.x;
			maxExtreme.x = vMax.x;
		}
		else
		{
			minExtreme.x = vMax.x;
			maxExtreme.x = vMin.x;
		}

		// for y
		if(vNormal.y >= 0)
		{
			minExtreme.y = vMin.y;
			maxExtreme.y = vMax.y;
		}
		else
		{
			minExtreme.y = vMax.y;
			maxExtreme.y = vMin.y;
		}

		// for z
		if(vNormal.z >= 0)
		{
			minExtreme.z = vMin.z;
			maxExtreme.z = vMax.z;
		}
		else
		{
			minExtreme.z = vMax.z;
			maxExtreme.z = vMin.z;
		}

        // If minimum extreme point is outside, then the whole AABB
        // must be outside.
        if(PlaneDotCoord(*it, minExtreme) > 0.0f)
			return eCullCode_Out;

        // The minimum extreme point is inside. Hence, if the maximum
        // extreme point is outside, then the AABB must intersect with
        // the plane. However, the AABB may still be outside another
        // plane.
        if(PlaneDotCoord(*it, maxExtreme) >= 0.0f)
			intersect = eTrue;
    }

    // The AABB is either partially or fully visible.
    if(intersect)
		return eCullCode_Intersect;

    return eCullCode_In;
}

///////////////////////////////////////////////
//! Check if the given AABB is in the frustum.
template <typename T, typename BASE>
tBool cFrustumImpl<T,BASE>::IntersectAABB(const sVec3f& vMin, const sVec3f& vMax) const
{
	if (mvPlanes.empty())
		return eTrue;	// No planes in the frustum means that it's an infinie (360 degres frustum)

	sVec3<T> nearpoint;

    for (tPlaneVecCIt it = mvPlanes.begin(); it != mvPlanes.end(); ++it)
	{
		if(it->x > 0.0f)
		{
			if(it->y > 0.0f)
			{
				if(it->z > 0.0f)	nearpoint.Set(vMin.x, vMin.y, vMin.z);
				else				nearpoint.Set(vMin.x, vMin.y, vMax.z);
			}
			else
			{
				if(it->z > 0.0f)	nearpoint.Set(vMin.x, vMax.y, vMin.z);
				else				nearpoint.Set(vMin.x, vMax.y, vMax.z);
			}
		}
		else
		{
			if(it->y > 0.0f)
			{
				if(it->z > 0.0f)	nearpoint.Set(vMax.x, vMin.y, vMin.z);
				else				nearpoint.Set(vMax.x, vMin.y, vMax.z);
			}
			else
			{
				if(it->z > 0.0f)	nearpoint.Set(vMax.x, vMax.y, vMin.z);
				else				nearpoint.Set(vMax.x, vMax.y, vMax.z);
			}
		}

		if(PlaneDotCoord(*it,nearpoint) > 0.0f)
			// near extreme point is outside and thus the AABB is totally
			// outside the polyhedron
			return eFalse;
	}

	return eTrue;
}


///////////////////////////////////////////////
//! Cull a sphere.
template <typename T, typename BASE>
eCullCode cFrustumImpl<T,BASE>::CullSphere(const sVec3f& avCenter, tF32 afRadius) const
{
	if (mvPlanes.empty())
		return eCullCode_In;	// No planes in the frustum means that it's an infinie (360 degres frustum)

	tU32 c = 0;
	tF32 d;

    for (tPlaneVecCIt it = mvPlanes.begin(); it != mvPlanes.end(); ++it)
	{
		d = PlaneDotCoord(*it, avCenter);

		if (d < -afRadius)
			return eCullCode_Out;

		if (d > afRadius)
			++c;
	}

	return (c == mvPlanes.size()) ? eCullCode_In : eCullCode_Intersect;
}

///////////////////////////////////////////////
//! Check if the given sphere is in the frustum.
template <typename T, typename BASE>
tBool cFrustumImpl<T,BASE>::IntersectSphere(const sVec3f& avCenter, tF32 afRadius) const
{
	return CullSphere(avCenter,afRadius) != eCullCode_Out;
}

///////////////////////////////////////////////
//! Transform the frustum by the given matrix.
template <typename T, typename BASE>
tBool cFrustumImpl<T,BASE>::Transform(const sMatrix<T>& M)
{
	for (tPlaneVecIt it = mvPlanes.begin(); it != mvPlanes.end(); ++it)
	{
		PlaneTransform(*it, *it, M);
	}

	UpdateBoundingVolume();
	return eTrue;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
} // End of namespace agl
