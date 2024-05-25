#pragma once
#ifndef __ISOCKET_H_1BFFA368_8773_EF48_ACA6_B81C73C1A2D7__
#define __ISOCKET_H_1BFFA368_8773_EF48_ACA6_B81C73C1A2D7__
// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT
#include "Types.h"

namespace ni {
struct iFile;

/** \addtogroup niLang
 * @{
 */

//! Remote address types.
enum eRemoteAddressType
{
  //! Unknown remote address type.
  eRemoteAddressType_Unknown = 0,
  //! IP v4 address type.
  eRemoteAddressType_IPv4 = 1,
  //! IP v6 address type.
  eRemoteAddressType_IPv6 = 2,
  //! IPC (inter-process communication) address.
  eRemoteAddressType_IPC = 3,
  //! \internal
  eRemoteAddressType_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Remote address base interface.
struct iRemoteAddress : public iUnknown
{
  niDeclareInterfaceUUID(iRemoteAddress,0x3e076b8b,0x75a5,0x47c8,0x9e,0x7d,0x4a,0x4b,0xa4,0x53,0xdb,0x58)

  //! Clone the address.
  virtual iRemoteAddress* __stdcall Clone() const = 0;

  //! Compare to another address.
  //! \return 0 if the address is the same, -1 if it's 'smaller' and 1 if it's greater.
  //!     If the addresses are not of the same type the type value is compared.
  virtual tI32 __stdcall Compare(iRemoteAddress* apAddr) = 0;

  //! Get the address type.
  //! {Property}
  virtual eRemoteAddressType __stdcall GetType() const = 0;

  //! Set the address from a string.
  //! {Property}
  virtual tBool __stdcall SetAddressString(const achar* aaszAddress) = 0;
  //! Get the address's string.
  //! {Property}
  virtual cString __stdcall GetAddressString() const = 0;

  //! Set the port of the address.
  //! \param  usPort is the port to assign the address.
  //! {Property}
  virtual void __stdcall SetPort(tU32 anPort) = 0;
  //! Get the port of the address.
  //! \return the port assigned to the address.
  //! {Property}
  virtual tU32 __stdcall GetPort() const = 0;

  //! Get whether the address (NOT including the port) is null / equivalent to zero.
  //! {Property}
  virtual tBool __stdcall GetIsNull() const = 0;
};

//! IPv4 remote address reserved values.
/** These addresses are reserved for special usages. */
enum eRemoteAddressIPv4Reserved
{
  //! Any address will do. Used by server/listener.
  eRemoteAddressIPv4Reserved_Any = 0,
  //! Address used to broadcast a packet.
  eRemoteAddressIPv4Reserved_Broadcast = 0xFFFFFFFF,
  //! \internal
  eRemoteAddressIPv4Reserved_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! IPv4 remote address interface.
/**
   String format :<br>
   - 128.0.145.145:40000 (40000 is the port)<br>
   - www.stuff.com:40000 (40000 is the port)<br>
   - "http://www.stuff.com:40000" (40000 is the port)<br>
*/
struct iRemoteAddressIPv4 : public iRemoteAddress
{
  niDeclareInterfaceUUID(iRemoteAddressIPv4,0xc7be5396,0x23f2,0x46b7,0xad,0xca,0xa0,0x9f,0x58,0xcc,0x94,0xe3)

  //! Set the ip address from a FOURCC code.
  //! \param  ulFourCC is the FourCC code which represent the address. It can be build
  //!     with the niFourCC macro.
  //! \remark FourCC can be also a eRemoteAddressIPv4Reserved enumeration element.
  //! {Property}
  virtual void __stdcall SetHost(tU32 anFourCC) = 0;
  //! Get the ip address in FOURCC form.
  //! \return a FourCC code which contain the address. Individual elements of the address
  //!     can be retrived with the niFourCC macros.
  //! \remark The returned value can be also a eRemoteAddressIPv4Reserved enumeration element.
  //! {Property}
  virtual tU32 __stdcall GetHost() const = 0;

  //! Get the host name.
  //! {Property}
  virtual cString __stdcall GetHostName() const = 0;
};

//! Socket protocols.
enum eSocketProtocol
{
  //! Unknown protocol.
  eSocketProtocol_Unknown = 0,
  //! UDP protocol.
  eSocketProtocol_UDP = 1,
  //! TCP protocol
  eSocketProtocol_TCP = 2,
  //! \internal
  eSocketProtocol_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Socket wait flags.
enum eSocketWaitFlags
{
  //! No flags, failure/timeout.
  eSocketWaitFlags_None = 0,
  //! Wait for send.
  eSocketWaitFlags_Send = niBit(0),
  //! Wait for receive.
  eSocketWaitFlags_Receive = niBit(1),
  //! Wait error
  eSocketWaitFlags_Error = 0xFFFFFFFF,
  //! \internal
  eSocketWaitFlags_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Socket Error codes
enum eSocketErrno
{
  //! No Error
  eSocketErrno_OK = 0,
  //! Generic 'unknown' error.
  eSocketErrno_Unknown = 1,
  //! (EWOULDBLOCK/EAGAIN) Resource temporarily unavailable. This is a
  //! temporary condition and later calls to the same routine may
  //! complete normally.
  eSocketErrno_WouldBlock = 2,
  //! (EINTR) Interrupted function call. A blocking operation was
  //! interrupted.
  eSocketErrno_Interrupted = 3,
  //! (EINVAL) Invalid argument.
  eSocketErrno_Invalid = 4,
  //! (EALREADY) Operation already in progress. An operation was
  //! attempted on a non-blocking object that already had an
  //! operation in progress.
  eSocketErrno_Already = 5,
  //! (EBADF) Bad file descriptor.
  eSocketErrno_BadFile = 6,
  //! (EHOSTDOWN) Host is down. A socket operation failed because
  //! the destination host was down. A socket operation encountered
  //! a dead host. Networking activity on the local host has not
  //! been initiated.
  eSocketErrno_HostDown = 7,
  //! (EINPROGRESS) Operation now in progress. An operation that
  //! takes a long time to complete (such as a Connect()) was
  //! attempted on a non-blocking socket.
  eSocketErrno_InProgress = 8,
  //! (EMSGSIZE) Message too long. A message sent on a socket was
  //! larger than the internal message buffer or some other network
  //! limit.
  eSocketErrno_MessageTooLong = 9,
  //! (ESHUTDOWN) Cannot send after socket shutdown.
  eSocketErrno_Shutdown = 10,
  //! (ETIMEDOUT) Connection timed out. A connect or send request
  //! failed because the connected party did not properly respond
  //! after a period of time.
  eSocketErrno_Timeout = 11,
  //! (EUSERS) Too many users.
  eSocketErrno_TooManyUsers = 12,
  //! (EISCONN) Socket is already connected. A Connect request was
  //! made on an already connected socket; or, a send request on a
  //! connected socket specified a destination when already
  //! connected.
  eSocketErrno_AlreadyConnected = 13,
  //! (ENOTCONN) Socket is not connected. A request to send or
  //! receive data was disallowed because the socket is not
  //! connected and (when sending on a datagram socket) no address
  //! was supplied.
  eSocketErrno_NotConnected = 14,
  //! (ECONNABORTED) Software caused connection abort.
  eSocketErrno_ConnectionAborted = 15,
  //! (ECONNREFUSED) Connection refused. No connection could be made
  //! because the target machine actively refused it. This usually
  //! results from trying to connect to a service that is inactive
  //! on the foreign host.
  eSocketErrno_ConnectionRefused = 16,
  //! (ECONNRESET) Connection reset by peer. A connection was
  //! forcibly closed by a peer. This normally results from a loss
  //! of the connection on the remote socket due to a timeout or a
  //! reboot.
  eSocketErrno_ConnectionReset = 17,
  //! (EADDRINUSE) Address already in use. An attempt was made to
  //! access a file in a way forbidden by its file access
  //! permissions.
  eSocketErrno_AddressInUser = 18,
  //! (EADDRNOTAVAIL) Cannot assign requested address.
  eSocketErrno_CannotAssignAddress = 19,
  //! (ENETDOWN) Network is down.
  eSocketErrno_NetworkDown = 20,
  //! (ENETRESET) Network dropped connection on reset.
  eSocketErrno_NetworkReset = 21,
  //! (ENETUNREACH) Network is unreachable.
  eSocketErrno_NetworkUnreachable = 22,
  //! (ENOBUFS) No buffer space available.
  eSocketErrno_NoBufferSpace = 23,
  //! Only a portion of the message has been received.
  //! \remark Currently, this can be set by ReceiveBuffers only.
  eSocketErrno_MessagePartial = 24,
  //! \internal
  eSocketErrno_ForceDWORD niMaybeUnused = 0xFFFFFFFF
};

//! Socket buffer structure
struct sSocketBuffer
{
#ifdef niWindows
  tSize dataLength;
  void* data;
#else
  void* data;
  tSize dataLength;
#endif
};

//! Socket wait flags type. \see ni::eSocketWaitFlags
typedef tU32 tSocketWaitFlags;

//! Network socket interface.
struct iSocket : public iUnknown
{
  niDeclareInterfaceUUID(iSocket,0x9262952d,0xf667,0x47a9,0x81,0x47,0x25,0xc5,0x9f,0xbb,0x36,0x7d)

  //! Get the socket's handle/descriptor.
  //! {Property}
  virtual tIntPtr __stdcall GetHandle() const = 0;

  //! Get the socket's associated address.
  //! {Property}
  virtual iRemoteAddress* __stdcall GetAddress() const = 0;
  //! Get the protocol.
  //! {Property}
  virtual eSocketProtocol __stdcall GetProtocol() const = 0;

  //! Make the socket to be non-blocking.
  virtual tBool __stdcall MakeNonBlocking() = 0;
  //! Get whether the socket is non-blocking
  //! {Property}
  virtual tBool __stdcall GetIsNonBlocking() const = 0;

  //! Get the last error code.
  //! {Property}
  //! \remark The error code is never reset to OK, as such it
  //!         should be queried only when one of the functions
  //!         return an error code.
  virtual eSocketErrno __stdcall GetErrno() const = 0;
  //! Get the last raw error code.
  //! {Property}
  //! \remark Get the error code as returned by the platform/OS.
  //! \remark The error code is never reset to OK, as such it
  //!         should be queried only when one of the functions
  //!         return an error code.
  virtual tInt __stdcall GetRawErrno() const = 0;

  //! Connect the socket to the specified address.
  //! \param  apAddress is the address to connect to.
  //! \return eFalse if the connection failed, else eTrue.
  //! \remark Sets the socket address to the connected address.
  //! \remark Blocking on blocking sockets.
  virtual tBool __stdcall Connect(iRemoteAddress* apAddress) = 0;
  //! Accept incoming connections.
  //! \remark Blocking on blocking sockets.
  virtual iSocket* __stdcall Accept() = 0;

  //! Bind a socket to listen on a specific port.
  //! \param  usPort is the port to listen.
  //! \return eFalse if the binding failed, else eTrue. The binding will fail
  //!     if the port is already listened by another socket.
  virtual tBool __stdcall BindPortAny(tU32 anPort) = 0;
  //! Bind a socket to listen the specified address.
  //! \return eFalse if the binding failed, else eTrue.
  virtual tBool __stdcall Bind(iRemoteAddress* apAddr) = 0;

  //! Starts listening for incoming connections.
  //! \param anBacklog is the maximum length of the queue of pending
  //!        connections, if it is 0 or invalid a reasonable maximal value
  //!        will be used.
  virtual tBool __stdcall Listen(tU32 anBacklog) = 0;
  //! Get whether the socket is in listening mode.
  //! {Property}
  virtual tBool __stdcall GetIsListening() const = 0;

  //! Send a packet to this socket.
  //! \param  apFile is the file to send.
  //! \param  anBytes is the number of bytes from from the current file position to send.
  //!     If zero the file from it's current position to the end will be sent.
  //! \return -1 if an error happen, else the number of bytes effectivly sent.
  //! \remark The method will try to send all required bytes unless an error occur.
  virtual tI32 __stdcall Send(iFile* apFile, tU32 anBytes) = 0;
  //! Send a packet to the specified address.
  //! \param  apFile is the file to send.
  //! \param  anBytes is the number of bytes from the current file position to send.
  //!     If zero the file from it's current position to the end will be sent.
  //! \param  apAddress is where the data must be sent.
  //! \return -1 if an error happen, else the number of bytes effectivly sent.
  //!     An error happen if the destination address cannot be found or is
  //!     invalid.
  virtual tI32 __stdcall SendTo(iFile* apFile, tU32 anBytes, iRemoteAddress* pAddress) = 0;
  //! Send the specified buffers to the specified address.
  //! {NoAutomation}
  virtual tI32 __stdcall SendBuffers(const iRemoteAddress* apAddress, const sSocketBuffer* apBuffers, tSize anBufCount) = 0;

  //! Receive connected data.
  //! \param  apFile is where the received data will be stored.
  //! \return -1 if an error happen, else the number of bytes effectivly received.
  //!     An error can happen if the destination address cannot be found or is
  //!     invalid.
  //! \remark If no error happen, block until data are received.
  //! \remark Blocking on blocking sockets.
  virtual tI32 __stdcall Receive(iFile* apFile) = 0;
  //! Receive datagram data.
  //! \param  apFile is where the received data will be stored.
  //! \param  apAddress will contain the address of the data sender.
  //! \return -1 if an error happen, else the number of bytes effectivly received.
  //!     An error can happen if the destination address cannot be found or is
  //!     invalid.
  //! \remark If no error happen, block until data are received.
  //! \remark Blocking on blocking sockets.
  virtual tI32 __stdcall ReceiveFrom(iFile* apFile, iRemoteAddress* apAddress) = 0;
  //! Receive buffers.
  //! {NoAutomation}
  virtual tI32 __stdcall ReceiveBuffers(iRemoteAddress* apAddr, sSocketBuffer* apBuffers, tSize anBufCount) = 0;

  //! Wait on the specified events.
  //! \return The event that stopped the waiting, or ni::eSocketWaitFlags_None if timeout.
  virtual tSocketWaitFlags __stdcall Wait(tSocketWaitFlags aFlags, tU32 anTimeoutInMs = 0) = 0;

  //! Set the node name associated with the socket.
  //! {Property}
  virtual void __stdcall SetNodeName(const achar* aaszName) = 0;
  //! Get the node name associated with the socket.
  //! {Property}
  virtual const achar* __stdcall GetNodeName() const = 0;

  //! Set the socket to KEEPALIVE mode
  //! {Property}
  virtual tBool __stdcall SetKeepAlive(tBool aKeepAlive) = 0;
  //! Whether the socket is in KEEPALIVE mode
  //! {Property}
  virtual tBool __stdcall GetIsKeepAlive() const = 0;
  //! Set KEEPALIVE interval on the current socket
  //! \param anTimeMS the interval to wait before probing the idle connection, in milliseconds.
  //! \param anIntvlMS the interval to wait before retrying the probe after an initial failure to respond, in milliseconds.
  //! \param anProbes the maximum number of times to retry the probe. Not supported on Windows.
  //! \remark If you don't want to set all the parameters, just set the parameter to 0.
  //! {Property}
  virtual tBool __stdcall SetKeepAliveParameters(tU64 anTimeMS, tU64 anIntvl, tU32 anProbes) = 0;
  //! Get KEEPALIVE interval on the current socket, in milliseconds.
  //! \remark the interval to wait before retrying the probe after an initial failure to respond.
  //! {Property}
  virtual tU64 __stdcall GetKeepAliveInterval() const = 0;
  //! Get KEEPALIVE probes on the current socket.
  //! \remark the maximum number of times to retry the probe.
  //! {Property}
  virtual tU32 __stdcall GetKeepAliveProbes() const = 0;
  //! Get KEEPALIVE time on the current socket, in milliseconds.
  //! \remark the interval to wait before probing the idle connection
  //!         Not supported on Windows.
  //! {Property}
  virtual tU64 __stdcall GetKeepAliveTime() const = 0;

  //! {Property}
  virtual tBool __stdcall SetReuseAddress(tBool abReuseAddress) = 0;
  //! {Property}
  virtual tBool __stdcall GetReuseAddress() const = 0;
};

/// EOF //////////////////////////////////////////////////////////////////////////////////////
/**@}*/
}
#endif // __ISOCKET_H_1BFFA368_8773_EF48_ACA6_B81C73C1A2D7__
