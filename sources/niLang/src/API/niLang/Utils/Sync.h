#ifndef __SYNC_31981078_H__
#define __SYNC_31981078_H__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "ThreadImpl.h"
#include "BoxedTypes.h"
#include "SyncPtr.h"
#include "Nonnull.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

//! Synchronisation mutex.
#define __sync_mutex_(M)    mutable ni::ThreadMutex mmutex##M
//! Synchronisation mutex, using default mutex name.
#define __sync_mutex()      __sync_mutex_(Mutex)
//! Synchronised type.
#define __sync_type(T)      niSync  T
//! Synchronised pointer type.
#define __sync_ptr(T)       niSync ni::BoxPtrAtomic<T>
//! Synchronised smart pointer type.
#define __sync_sptr(T)      niSync ni::Ptr<T>
//! Synchronised query pointer type.
#define __sync_qptr(T)      niSync ni::QPtr<T>
//! Synchronised base type.
#define __sync_base(T)      niSync ni::BoxAtomic<T>
//! Synchronised local access.
#define __sync_local_type_(T,V,M) ni::SyncPtr<T>  V(m##V,mmutex##M)
//! Synchronised local access, using default mutex name.
#define __sync_local_type(T,V)    __sync_local_type_(T,V,Mutex)
//! Synchronised local access to base type.
#define __sync_local_base_(T,V,M) ni::SyncPtr<T>  V(*m##V.Ptr(),mmutex##M)
//! Synchronised local access to base type, using default mutex name.
#define __sync_local_base(T,V)    __sync_local_base_(T,V,Mutex)
//! Synchronised local access to pointers.
#define __sync_local_ptr_(T,V,M)  ni::SyncPtr<T>  V(*m##V.Ptr(),mmutex##M)
//! Synchronised local access to pointers, using default mutex name.
#define __sync_local_ptr(T,V)   __sync_local_ptr_(T,V,Mutex)
//! Synchronised local access to smart pointers.
#define __sync_local_sptr_(T,V,M) ni::SyncPtr<T>  V(*const_cast<ni::Ptr<T>&>(m##V).ptr(),mmutex##M)
//! Synchronised local access to smart pointers, using default mutex name.
#define __sync_local_sptr(T,V)    __sync_local_sptr_(T,V,Mutex)
//! Synchronised mutex lock.
#define __sync_lock_(M)   ni::AutoThreadLock  lock##M(mmutex##M);
//! Synchronised mutex lock, using default mutex name.
#define __sync_lock()   __sync_lock_(Mutex)
//! Synchronised locked local access, aka when you know the mutex is already properly locked.
#define __sync_llocal_type(T,V) T* V = &const_cast<T&>(m##V)
//! Synchronised locked local access, aka when you know the mutex is already properly locked.
#define __sync_llocal_base(T,V) T* V = m##V.Ptr()
//! Synchronised locked local access, aka when you know the mutex is already properly locked.
#define __sync_llocal_ptr(T,V)  T*& V = *m##V.PtrAddr()
//! Synchronised locked local access, aka when you know the mutex is already properly locked.
#define __sync_llocal_sptr(T,V) ni::Ptr<T>& V = const_cast<ni::Ptr<T>&>(m##V)

//! Lock Synchronisation mutex.
#define __sync_mutex_lock_(M)   mmutex##M.ThreadLock()
//! Lock Synchronisation mutex, using default mutex name.
#define __sync_mutex_lock()     __sync_mutex_lock_(Mutex)
//! Unlock Synchronisation mutex.
#define __sync_mutex_unlock_(M)   mmutex##M.ThreadUnlock()
//! Unlock Synchronisation mutex, using default mutex name.
#define __sync_mutex_unlock()   __sync_mutex_unlock_(Mutex)

//! Synchronised locked get access, aka when you know the mutex is already properly locked.
#define __sync_lget_type(T,V) (&const_cast<T&>(V))
//! Synchronised locked get access, aka when you know the mutex is already properly locked.
#define __sync_lget_base(V) (V.Ptr())
//! Synchronised locked get access, aka when you know the mutex is already properly locked.
#define __sync_lget_ptr(V)  (*V.PtrAddr())
//! Synchronised locked get access, aka when you know the mutex is already properly locked.
#define __sync_lget_sptr(T,V) const_cast<ni::Ptr<T>&>(V)

//! Synchronised set.
#define __sync_set_(D,S,M)  ni::__SyncSet(D,S,mmutex##M)
//! Synchronised set, using default mutex name.
#define __sync_set(D,S)   __sync_set_(D,S,Mutex)
//! Synchronised locked set, aka when you know the mutex is already properly locked.
#define __sync_lset(D,S)  __SyncLockedSet(D,S)

namespace ni {
template <typename T, typename MT>
inline void __SyncSet(niSync T& aToSet, const T& aVal, MT& mutex) {
  ni::SyncPtr<T> ptr(aToSet,mutex);
  *ptr = aVal;
}
template <typename T, typename MT>
inline void __SyncSet(niSync Ptr<T>& aToSet, const T* aVal, MT& mutex) {
  ni::SyncPtr<Ptr<T> > ptr(aToSet,mutex);
  *ptr = (T*)aVal;
}
template <typename T, typename MT>
inline void __SyncSet(niSync BoxAtomic<T>& aToSet, const T& aVal, MT& mutex) {
  ni::SyncPtr<BoxAtomic<T> > ptr(aToSet,mutex);
  *ptr = aVal;
}
template <typename T, typename MT>
inline void __SyncSet(niSync BoxPtrAtomic<T>& aToSet, const T* aVal, MT& mutex) {
  ni::SyncPtr<BoxPtrAtomic<T> > ptr(aToSet,mutex);
  *ptr = (T*)aVal;
}
template <typename T>
inline void __SyncLockedSet(niSync T& aToSet, const T& aVal) {
  const_cast<T&>(aToSet) = aVal;
}
template <typename T>
inline void __SyncLockedSet(niSync Ptr<T>& aToSet, T* aVal) {
  const_cast<Ptr<T>&>(aToSet) = aVal;
}
template <typename T>
inline void __SyncLockedSet(niSync BoxAtomic<T>& aToSet, const T& aVal) {
  const_cast<BoxAtomic<T>&>(aToSet) = aVal;
}
template <typename T>
inline void __SyncLockedSet(niSync BoxPtrAtomic<T>& aToSet, T* aVal) {
  const_cast<BoxPtrAtomic<T>&>(aToSet) = aVal;
}
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
/**@}*/
#endif // __SYNC_31981078_H__
