#ifndef __UNIXTHREAD_H_EA367A1D_4ECD_4F76_A17F_FA53088D7043__
#define __UNIXTHREAD_H_EA367A1D_4ECD_4F76_A17F_FA53088D7043__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

namespace ni {
/** \addtogroup niLang
 * @{
 */
namespace Unix {
/** \addtogroup niLang_Unix
 * @{
 */

// #define TRACE_UNIX_THREAD

typedef unsigned int  DWORD;
typedef unsigned char BYTE;
typedef void*        HANDLE;
typedef void         VOID;
typedef int          BOOL;

enum WAIT_FOR_CONSTS {
  WAIT_TIMEOUT = 0,
  WAIT_FAILED = 1,
  WAIT_OBJECT_0 = 0xFFFF,
  INFINITE = ~0
};

//--------------------------------------------------------------------------------------------
//
//  Event
//
//--------------------------------------------------------------------------------------------
typedef struct _EVENT_OBJECT
{
  pthread_mutex_t lock;
  pthread_cond_t  cond;
  bool is_set;
  bool is_pulse;
  bool is_manual_reset;

  _EVENT_OBJECT( bool _is_set, bool _is_manual_reset )
      : is_set( _is_set ),
        is_pulse( false ), is_manual_reset( _is_manual_reset )
  {
    pthread_cond_init( &cond, NULL );
    pthread_mutex_init( &lock, NULL );
  }
  ~_EVENT_OBJECT()
  {
    pthread_cond_destroy( &cond );
    pthread_mutex_destroy( &lock );
  }

} EVENT_OBJECT, *LPEVENT_OBJECT;

inline HANDLE CreateEvent(void*, BOOL bManualReset, BOOL bInitialSet) {
  return (HANDLE)new EVENT_OBJECT(bInitialSet,bManualReset);
}

inline BOOL DeleteEvent(HANDLE hObject) {
  LPEVENT_OBJECT lpObject = (LPEVENT_OBJECT)hObject;
#ifdef TRACE_UNIX_THREAD
  printf("Unix::CloseHandle<EVENT> %p: %d, %d\n",
         hObject,lpObject->is_set,lpObject->is_pulse);
  fflush(stdout);
#endif
  delete lpObject;
  return 1;
}

inline BOOL SetEvent(HANDLE hObject) {
  LPEVENT_OBJECT lpObject = (LPEVENT_OBJECT)hObject;
  pthread_mutex_lock( &lpObject->lock );
  lpObject->is_set = true;
  lpObject->is_pulse = false;
  pthread_cond_broadcast( &lpObject->cond );
#ifdef TRACE_UNIX_THREAD
  printf("Unix::SetEvent %p: %d, %d\n",
         hObject,lpObject->is_set,lpObject->is_pulse);
  fflush(stdout);
#endif
  pthread_mutex_unlock( &lpObject->lock );
  return 1;
}

inline BOOL PulseEvent(HANDLE hObject) {
  LPEVENT_OBJECT lpObject = (LPEVENT_OBJECT)hObject;
  pthread_mutex_lock( &lpObject->lock );
  lpObject->is_set = true;
  lpObject->is_pulse = true;
  pthread_cond_signal( &lpObject->cond );
#ifdef TRACE_UNIX_THREAD
  printf("Unix::PulseEvent %p: %d, %d\n",
         hObject,lpObject->is_set,lpObject->is_pulse);
  fflush(stdout);
#endif
  pthread_mutex_unlock( &lpObject->lock );

  return 1;
}

inline BOOL ResetEvent(HANDLE hObject) {
  LPEVENT_OBJECT lpObject = (LPEVENT_OBJECT)hObject;
  pthread_mutex_lock( &lpObject->lock );
  lpObject->is_set = false;
#ifdef TRACE_UNIX_THREAD
  printf("Unix::ResetEvent %p: %d, %d\n",
         hObject,lpObject->is_set,lpObject->is_pulse);
  fflush(stdout);
#endif
  pthread_mutex_unlock( &lpObject->lock );

  return 1;
}

inline DWORD WaitForSingleObject(HANDLE hObject, DWORD dwTime) {
  DWORD retval = WAIT_OBJECT_0;
  LPEVENT_OBJECT lpObject = (LPEVENT_OBJECT)hObject;
  pthread_mutex_lock( &lpObject->lock );

#ifdef TRACE_UNIX_THREAD
  printf("Unix::WaitForSingleObject for %p: %d, %d\n",
         hObject,lpObject->is_set,lpObject->is_pulse);
  fflush(stdout);
#endif

  if ( true != lpObject->is_set ) {
    if (dwTime == INFINITE) {
      pthread_cond_wait( &lpObject->cond, &lpObject->lock );
    }
    else {
      struct timeval delta;
      struct timespec abstime;
      gettimeofday(&delta, NULL);

      abstime.tv_sec = delta.tv_sec + (dwTime/1000);
      abstime.tv_nsec = (delta.tv_usec + (dwTime%1000) * 1000) * 1000;
      if (abstime.tv_nsec > 1000000000) {
        abstime.tv_sec += 1;
        abstime.tv_nsec -= 1000000000;
      }

   tryagain:
      retval = pthread_cond_timedwait(&lpObject->cond, &lpObject->lock, &abstime);
      switch (retval) {
        case EINTR:
          goto tryagain;
          break;
        case ETIMEDOUT:
          // time out
          retval = WAIT_TIMEOUT;
          break;
        default:
          // failed
          retval = WAIT_FAILED;
          break;
        case 0:
          // done
          retval = WAIT_OBJECT_0;
          break;
      }
    }
  }

  if (false == lpObject->is_manual_reset)
    lpObject->is_set = false;

  pthread_mutex_unlock(&lpObject->lock);
  return retval;
}

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
/**@}*/
}
#endif // __UNIXTHREAD_H_EA367A1D_4ECD_4F76_A17F_FA53088D7043__
