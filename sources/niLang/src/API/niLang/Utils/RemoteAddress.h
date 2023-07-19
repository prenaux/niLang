#ifndef __REMOTEADDRESS_H_50E72F89_E80D_44DA_BC18_F074366242BD__
#define __REMOTEADDRESS_H_50E72F89_E80D_44DA_BC18_F074366242BD__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "../ISocket.h"

#if !defined niNoSocket

#ifdef niWindows
#include "../Platforms/Win32/Win32_Redef.h"
#include <winsock2.h>
#endif

#ifdef niPosix
#include <netinet/in.h>
#endif

namespace ni {
/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_Utils
 * @{
 */

///////////////////////////////////////////////
static inline tBool RemoteAddrToSocketAddr(const iRemoteAddress* apAddr, sockaddr_in* dest) {
  if (!dest) return eFalse;
  QPtr<iRemoteAddressIPv4> ra = apAddr;
  if (!ra.IsOK()) return eFalse;
  memset(&dest,0,sizeof(dest));
  dest->sin_family = AF_INET;
  dest->sin_addr.s_addr = ra->GetHost();
  dest->sin_port = niSwapBE16(ra->GetPort());
  return eTrue;
}

///////////////////////////////////////////////
static inline tBool SocketAddrToRemoteAddr(const sockaddr_in* src, iRemoteAddress* apAddr) {
  if (!src) return eFalse;
  QPtr<iRemoteAddressIPv4> ra = apAddr;
  if (!ra.IsOK()) return eFalse;
  ra->SetHost(src->sin_addr.s_addr);
  ra->SetPort(niSwapBE16(src->sin_port));
  return eTrue;
}

niExportFunc(iRemoteAddressIPv4*) CreateRemoteAddressIPv4FromSockAddr(sockaddr_in* apSockAddrIn);

/**@}*/
/**@}*/
}

#endif // #if !defined niNoSocket

#endif // __REMOTEADDRESS_H_50E72F89_E80D_44DA_BC18_F074366242BD__
