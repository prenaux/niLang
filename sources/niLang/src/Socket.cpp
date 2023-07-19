#include "API/niLang/Types.h"
#include "Lang.h"

#ifdef niNoSocket

iSocket* __stdcall cLang::CreateSocketFromHandle(
  ni::tInt aSocket,
  ni::iRemoteAddress* apAddr)
{
  niAssertUnreachable("Not implemented");
  return NULL;
}
iSocket* __stdcall cLang::CreateSocket(
  ni::eSocketProtocol aProtocol,
  ni::iRemoteAddress* apAddr)
{
  niAssertUnreachable("Not implemented");
  return NULL;
}
void __stdcall cLang::CloseSocketHandle(ni::tInt aSocket) {
  niAssertUnreachable("Not implemented");
}

#elif niMinFeatures(15)

#include "API/niLang/ISocket.h"
#include "API/niLang/Utils/RemoteAddress.h"

/// Windows
#if defined niWindows
#include <winsock2.h>
#include <MSTCPiP.h>
#ifdef niPragmaCommentLib
#pragma comment(lib,"Ws2_32.lib")
#endif

/// MacOSX
#elif defined niOSX
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>

#define INVALID_SOCKET   -1
#define SOCKET_ERROR     -1
typedef int SOCKET;
#define HAS_SOCKLEN_T
#undef HAS_POLL
#define HAS_FCNTL
#define HAS_MSGHDR_FLAGS

/// Posix
#elif defined niPosix

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#define INVALID_SOCKET   -1
#define SOCKET_ERROR     -1
typedef int SOCKET;
// This is for ArchLinux x64, the test dev platform, got to be replaced
// by a proper setup of UnixConfig.h ...
#define HAS_SOCKLEN_T
#define HAS_POLL
#define HAS_FCNTL
#define HAS_MSGHDR_FLAGS
#else
#error "Unsupported platform !"
#endif

#ifdef HAS_FCNTL
#include <fcntl.h>
#endif
#if defined niOSX
#undef HAS_POLL
#endif
#ifdef HAS_POLL
#include <sys/poll.h>
#endif
#ifndef HAS_SOCKLEN_T
typedef int socklen_t;
#endif
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

using namespace ni;

struct SocketInit {
  tBool _bOK;

  SocketInit() {
#ifdef niWindows
    // Startup winsock version 1.1
    WSADATA wsock;
    if (WSAStartup(MAKEWORD(1,1),&wsock) != 0) {
      _bOK = eFalse;
      return;
    }
#endif
    _bOK = eTrue;
  }
  ~SocketInit() {
#ifdef niWindows
    WSACleanup();
#endif
  }
} _socketInit;

static const tU32 _knSocketBufferSize = niStackBufferSize;

#define CHECKVALID(RET) if (mSocket == INVALID_SOCKET) { /*niError("Invalid Socket.");*/ return RET; }

struct sockaddrAlias : public sockaddr {};

class cSocket : public cIUnknownImpl<ni::iSocket>
{
  niBeginClass(cSocket);

  enum {
    FLAGS_LISTENING = niBit(0),
    FLAGS_NONBLOCKING = niBit(1),
  };

  Ptr<iRemoteAddress> mptrAddress;
  eSocketProtocol mProtocol;
#ifdef niWindows
  SOCKET        mSocket;
#else
  int           mSocket;
#endif
  cString         mstrNodeName;
  tU32            mFlags;
  tInt            mRawLastError;
  eSocketErrno    mLastError;

 public:
#ifdef niPosix
  //////////////////////////////////////////////////////////////////////
  // POSIX
  static inline void _CloseSocket(SOCKET aSocket) {
    close(aSocket);
  }
  static inline void _SetNonBlocking(SOCKET aSocket) {
#ifdef HAS_FCNTL
    int fl = fcntl(aSocket,F_GETFL,0);
    if (fl == -1) return;
    fcntl(aSocket,F_SETFL,fl|O_NONBLOCK);
#else
    tU32 i = 1;
    ioctl(aSocket, FIONBIO, &i);
#endif
  }
  void _UpdateErrno() {
    mRawLastError = errno;
    mLastError = _GetSocketErrno(mRawLastError);
  }
  static eSocketErrno _GetSocketErrno(const tInt e) {
    switch (e) {
      case 0: return eSocketErrno_OK;
      case EWOULDBLOCK: return eSocketErrno_WouldBlock;
      case EINTR: return eSocketErrno_Interrupted;
      case EINVAL: return eSocketErrno_Invalid;
      case EALREADY: return eSocketErrno_Already;
      case EBADF: return eSocketErrno_BadFile;
      case EHOSTDOWN: return eSocketErrno_HostDown;
      case EINPROGRESS: return eSocketErrno_InProgress;
      case EMSGSIZE: return eSocketErrno_MessageTooLong;
      case ESHUTDOWN: return eSocketErrno_Shutdown;
      case ETIMEDOUT: return eSocketErrno_Timeout;
      case EUSERS: return eSocketErrno_TooManyUsers;
      case EISCONN: return eSocketErrno_AlreadyConnected;
      case ENOTCONN: return eSocketErrno_NotConnected;
      case ECONNABORTED: return eSocketErrno_ConnectionAborted;
      case ECONNREFUSED: return eSocketErrno_ConnectionRefused;
      case ECONNRESET: return eSocketErrno_ConnectionReset;
      case EADDRINUSE: return eSocketErrno_AddressInUser;
      case EADDRNOTAVAIL: return eSocketErrno_CannotAssignAddress;
      case ENETDOWN: return eSocketErrno_NetworkDown;
      case ENETRESET: return eSocketErrno_NetworkReset;
      case ENETUNREACH: return eSocketErrno_NetworkUnreachable;
      case ENOBUFS: return eSocketErrno_NoBufferSpace;
    }
    return eSocketErrno_Unknown;
  }

#else
  //////////////////////////////////////////////////////////////////////
  // Windows
  void _UpdateErrno() {
    mRawLastError = WSAGetLastError();
    mLastError = _GetSocketErrno(mRawLastError);
  }
  static eSocketErrno _GetSocketErrno(const tInt e) {
    switch (e) {
      case 0: return eSocketErrno_OK;
      case WSAEWOULDBLOCK: return eSocketErrno_WouldBlock;
      case WSAEINTR: return eSocketErrno_Interrupted;
      case WSAEINVAL: return eSocketErrno_Invalid;
      case WSAEALREADY: return eSocketErrno_Already;
      case WSAEBADF: return eSocketErrno_BadFile;
      case WSAEHOSTDOWN: return eSocketErrno_HostDown;
      case WSAEINPROGRESS: return eSocketErrno_InProgress;
      case WSAEMSGSIZE: return eSocketErrno_MessageTooLong;
      case WSAESHUTDOWN: return eSocketErrno_Shutdown;
      case WSAETIMEDOUT: return eSocketErrno_Timeout;
      case WSAEUSERS: return eSocketErrno_TooManyUsers;
      case WSAEISCONN: return eSocketErrno_AlreadyConnected;
      case WSAENOTCONN: return eSocketErrno_NotConnected;
      case WSAECONNABORTED: return eSocketErrno_ConnectionAborted;
      case WSAECONNREFUSED: return eSocketErrno_ConnectionRefused;
      case WSAECONNRESET: return eSocketErrno_ConnectionReset;
      case WSAEADDRINUSE: return eSocketErrno_AddressInUser;
      case WSAEADDRNOTAVAIL: return eSocketErrno_CannotAssignAddress;
      case WSAENETDOWN: return eSocketErrno_NetworkDown;
      case WSAENETRESET: return eSocketErrno_NetworkReset;
      case WSAENETUNREACH: return eSocketErrno_NetworkUnreachable;
      case WSAENOBUFS: return eSocketErrno_NoBufferSpace;
    }
    return eSocketErrno_Unknown;
  }
  static inline void _CloseSocket(SOCKET aSocket) {
    ::closesocket(aSocket);
  }
  inline void _SetNonBlocking(SOCKET aSocket) {
    tU32 i = 1;
    ioctlsocket(mSocket, FIONBIO, &i);
  }
#endif

 public:
  ///////////////////////////////////////////////
  cSocket(tInt aSocket, iRemoteAddress* apAddr) {
    ZeroMembers();
    mptrAddress = apAddr;

    mSocket = aSocket;
    if (mSocket == INVALID_SOCKET) {
      niError("Invalid socket.");
      return;
    }
    int type = -1;
    socklen_t typeLen = sizeof(type);
    if (getsockopt(mSocket,SOL_SOCKET,SO_TYPE,(char*)&type,&typeLen) != 0) {
      _UpdateErrno();
      mSocket = INVALID_SOCKET;
      return;
    }
    mProtocol = (type==SOCK_DGRAM) ? eSocketProtocol_UDP : eSocketProtocol_TCP;
    if (!mptrAddress.IsOK()) {
      sockaddr_in addr; socklen_t addrLen = sizeof(addr);
      sockaddrAlias* paddr = (sockaddrAlias*)&addr;
      if (getsockname(mSocket,paddr,&addrLen) != 0) {
        _UpdateErrno();
        mSocket = INVALID_SOCKET;
        return;
      }
      mptrAddress = CreateRemoteAddressIPv4FromSockAddr(&addr);
    }
  }

  ///////////////////////////////////////////////
  cSocket(eSocketProtocol aProtocol, iRemoteAddress* apAddr = NULL) {
    ZeroMembers();
    mptrAddress = apAddr;
    mProtocol = aProtocol;

    switch (aProtocol) {
      //// UDP Socket ////
      case eSocketProtocol_UDP:
        {
          mSocket = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
          if (mSocket == INVALID_SOCKET) {
            _UpdateErrno();
            return;
          }
          // set the buffer size
          if (setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (char*)&_knSocketBufferSize, sizeof(int)) == SOCKET_ERROR) {
            _UpdateErrno();
            _CloseSocket(mSocket);
            mSocket = INVALID_SOCKET;
            return;
          }
          break;
        }
        //// TCP Socket ////
      case eSocketProtocol_TCP:
        {
          mSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
          if (mSocket == INVALID_SOCKET) {
            _UpdateErrno();
            return;
          }
          break;
        }
      default:
        {
          niError(niFmt("Unknown protocol '%d'.",aProtocol));
          break;
        }
    }
  }

  ///////////////////////////////////////////////
  ~cSocket() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  tBool __stdcall IsOK() const {
    niClassIsOK(cSocket);
    return mSocket != INVALID_SOCKET;
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
    if (mSocket != INVALID_SOCKET) {
      _CloseSocket(mSocket);
      mSocket = INVALID_SOCKET;
    }
  }

  ///////////////////////////////////////////////
  virtual tIntPtr __stdcall GetHandle() const {
    return mSocket;
  }

  ///////////////////////////////////////////////
  void ZeroMembers() {
    mProtocol = eSocketProtocol_Unknown;
    mSocket = INVALID_SOCKET;
    mFlags = 0;
    mRawLastError = 0;
    mLastError = eSocketErrno_OK;
  }

  ///////////////////////////////////////////////
  virtual iRemoteAddress* __stdcall GetAddress() const {
    return mptrAddress;
  }

  ///////////////////////////////////////////////
  virtual eSocketProtocol __stdcall GetProtocol() const {
    return mProtocol;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Connect(iRemoteAddress* apAddress) {
    CHECKVALID(eFalse);
    sockaddr_in addr;
    if (!_RemoteToSocketAddress(apAddress,addr)) {
      niError(niFmt("Invalid address: %s.",apAddress->GetAddressString().Chars()));
      return eFalse;
    }
    if (connect(mSocket, (sockaddrAlias*)&addr, sizeof(addr)) != 0) {
      _UpdateErrno();
      return eFalse;
    }
    mptrAddress = apAddress;
    return eTrue;
  }


  ///////////////////////////////////////////////
  // make it non-blocking
  virtual tBool __stdcall MakeNonBlocking() {
    if (mFlags&FLAGS_NONBLOCKING) return eTrue;
    _SetNonBlocking(mSocket);
    mFlags |= FLAGS_NONBLOCKING;
    return eTrue;
  }
  virtual tBool __stdcall GetIsNonBlocking() const {
    return mFlags&FLAGS_NONBLOCKING;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall BindPortAny(tU32 anPort) {
    CHECKVALID(eFalse);

    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = niSwapBE16((tU16)anPort);
    if (bind(mSocket,(sockaddrAlias*)&addr,sizeof(addr)) == SOCKET_ERROR) {
      _UpdateErrno();
      return eFalse;
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Bind(iRemoteAddress* apAddr) {
    CHECKVALID(eFalse);

    sockaddr_in addr;
    if (!_RemoteToSocketAddress(apAddr,addr)) {
      niError(niFmt("Invalid address: %s.",apAddr->GetAddressString().Chars()));
      return eFalse;
    }

    if (bind(mSocket,(sockaddrAlias*)&addr,sizeof(addr)) == SOCKET_ERROR) {
      _UpdateErrno();
      return eFalse;
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall Listen(tU32 anBacklog) {
    if (mFlags&FLAGS_LISTENING) {
      return eTrue;
    }
    switch (mProtocol) {
      // TCP/IP listening
      case eSocketProtocol_TCP:
        if (anBacklog == 0 || anBacklog == eInvalidHandle) {
          anBacklog = SOMAXCONN;
        }
        if (listen(mSocket, anBacklog) == SOCKET_ERROR) {
          _UpdateErrno();
          return eFalse;
        }
        break;
        // UDP listening
      case eSocketProtocol_UDP:
        // nothing to do for UDP
        break;
      default:
        break;
    }
    mFlags |= FLAGS_LISTENING;
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsListening() const {
    return mFlags&FLAGS_LISTENING;
  }

  ///////////////////////////////////////////////
  virtual eSocketErrno __stdcall GetErrno() const {
    return mLastError;
  }
  virtual tInt __stdcall GetRawErrno() const {
    return mRawLastError;
  }

  ///////////////////////////////////////////////
  virtual iSocket* __stdcall Accept() {
    niCheck(GetIsListening(),NULL);
    Ptr<iSocket> newSocket;
    switch (mProtocol) {
      //// TCP/IP connection ////
      case eSocketProtocol_TCP:
        {
          sockaddr_in addr;
          memset(&addr,0,sizeof(addr));
          socklen_t len = sizeof(addr);
          SOCKET s = accept(mSocket,(sockaddrAlias*)&addr,&len);
          if (s == INVALID_SOCKET) {
            _UpdateErrno();
            return NULL;
          }
          Ptr<iRemoteAddress> ptrAddr = CreateRemoteAddressIPv4FromSockAddr(&addr);
          newSocket = niNew cSocket(s,ptrAddr);
          niAssert(newSocket->GetProtocol() == eSocketProtocol_TCP);
          break;
        }
        //// UDP connection ////
      case eSocketProtocol_UDP:
        // nothing to do for UDP
        break;
        //// Unknown Protocol ////
      default:
        return NULL;
    }
    return newSocket.GetRawAndSetNull();
  }

  ///////////////////////////////////////////////
  tI32 __stdcall DoSend(iFile* apFile, tSize anBytes, iRemoteAddress* apAddress) {
    CHECKVALID(-1);
    tU8 buffer[_knSocketBufferSize];
    tSize remaining = anBytes;
    if (!remaining) {
      remaining = (tSize)(apFile->GetSize()-apFile->Tell());
    }

    tSize bufRead = ni::Min(remaining,_knSocketBufferSize);
    if (apFile->ReadRaw(buffer,bufRead) != bufRead) {
      niError("Error reading the input file.");
      return -1;
    }

    int toSend = (int)bufRead;
    int pos = 0;
    while (toSend > 0) {
      int sent;
      if (apAddress) {
        sockaddr_in addr;
        if (!_RemoteToSocketAddress(apAddress,addr)) {
          niError(niFmt("Invalid address: %s.",apAddress->GetAddressString().Chars()));
          return -1;
        }
        sent = sendto(mSocket,(char*)&buffer[pos],toSend,0, (sockaddrAlias*)&addr, sizeof(addr));
        if (sent < 0) {
          _UpdateErrno();
          return -1;
        }
      }
      else {
        sent = send(mSocket,(char*)&buffer[pos],toSend,0);
        if (sent < 0) {
          _UpdateErrno();
          return (mLastError == eSocketErrno_WouldBlock) ? 0 : -1;
        }
      }
      pos += sent;
      toSend -= sent;
    }
    return pos;
  }

  ///////////////////////////////////////////////
  virtual tI32 __stdcall Send(iFile* apFile, tU32 anBytes) {
    niCheckIsOK(apFile,-1);
    return DoSend(apFile,anBytes,NULL);
  }

  ///////////////////////////////////////////////
  virtual tI32 __stdcall SendTo(iFile* apFile, tU32 anBytes, iRemoteAddress* apAddress) {
    niCheckIsOK(apFile,-1);
    niCheckIsOK(apAddress,-1);
    return DoSend(apFile,anBytes,apAddress);
  }

  ///////////////////////////////////////////////
  virtual tI32 __stdcall SendBuffers(const iRemoteAddress* apAddress, const sSocketBuffer* apBuffers, tSize anBufCount)
  {
    struct sockaddr_in sin;
    tU32 sentLength;

    if (apAddress != NULL) {
      Ptr<iRemoteAddressIPv4> addr = ni::QueryInterface<iRemoteAddressIPv4>(apAddress);
      if (!addr.IsOK())
        return -1;
      sin.sin_family = AF_INET;
      sin.sin_port = niSwapBE16(addr->GetPort());
      sin.sin_addr.s_addr = addr->GetHost();
    }

#ifdef niPosix
    niCAssert(sizeof(struct iovec) == sizeof(sSocketBuffer));
    struct msghdr msgHdr;
    memset (& msgHdr, 0, sizeof (struct msghdr));
    if (apAddress != NULL) {
      msgHdr.msg_name = &sin;
      msgHdr.msg_namelen = sizeof (struct sockaddr_in);
    }
    msgHdr.msg_iov = (struct iovec*)apBuffers;
    msgHdr.msg_iovlen = anBufCount;
    sentLength = sendmsg(mSocket, & msgHdr, MSG_NOSIGNAL);
    if (sentLength == -1) {
      _UpdateErrno();
      return (mLastError == eSocketErrno_WouldBlock) ? 0 : -1;
    }
#else
    if (WSASendTo(mSocket,
                  (LPWSABUF)apBuffers,
                  (DWORD)anBufCount,
                  &sentLength,
                  0,
                  apAddress != NULL ? (struct sockaddr *) & sin : 0,
                  apAddress != NULL ? sizeof (struct sockaddr_in) : 0,
                  NULL,
                  NULL) == SOCKET_ERROR)
    {
      _UpdateErrno();
      return (mLastError == eSocketErrno_WouldBlock) ? 0 : -1;
    }
#endif

    return (tI32)sentLength;
  }

  ///////////////////////////////////////////////
  virtual tI32 __stdcall Receive(iFile* apFile) {
    CHECKVALID(-1);
    niCheckIsOK(apFile,-1);

    tU8 buffer[_knSocketBufferSize];
    tU32 pos = 0;
    int ret = recv(mSocket, (char*)&buffer[pos], _knSocketBufferSize, 0);
    if (ret > 0) {
      apFile->WriteRaw(&buffer[0],ret);
    }
    else if (ret < 0) {
      _UpdateErrno();
      return (mLastError == eSocketErrno_WouldBlock) ? 0 : -1;
    }

    return ret;
  }

  ///////////////////////////////////////////////
  virtual tI32 __stdcall ReceiveFrom(iFile* apFile, iRemoteAddress* apAddress) {
    CHECKVALID(-1);
    niCheckIsOK(apFile,-1);
    niCheckIsOK(apAddress,-1);

    tU8 buffer[_knSocketBufferSize];

    tU32 pos = 0;

    sockaddr_in addr;
    socklen_t fromlen = sizeof(addr);
    int ret = recvfrom(mSocket, (char*)&buffer[pos], _knSocketBufferSize, 0, (sockaddrAlias*)&addr, &fromlen);
    if (ret == SOCKET_ERROR) {
      _UpdateErrno();
      return (mLastError == eSocketErrno_WouldBlock) ? 0 : -1;
    }

    if (ret > 0) {
      apFile->WriteRaw(&buffer[0],ret);
    }

    _SocketAddressToRemote(addr,apAddress);
    return ret;
  }

  ///////////////////////////////////////////////
  virtual tI32 __stdcall ReceiveBuffers(iRemoteAddress* apAddress, sSocketBuffer* apBuffers, tSize anBufCount)
  {
    tI32 recvLength;
    struct sockaddr_in sin;

#ifdef niPosix
    niCAssert(sizeof(struct iovec) == sizeof(sSocketBuffer));
    struct msghdr msgHdr;
    memset (& msgHdr, 0, sizeof (struct msghdr));
    if (apAddress != NULL) {
      msgHdr.msg_name = & sin;
      msgHdr.msg_namelen = sizeof (struct sockaddr_in);
    }
    msgHdr.msg_iov = (struct iovec *)apBuffers;
    msgHdr.msg_iovlen = anBufCount;

    recvLength = recvmsg(mSocket, &msgHdr, MSG_NOSIGNAL);
    if (recvLength < 0) {
      _UpdateErrno();
      return (mLastError == eSocketErrno_WouldBlock) ? 0 : -1;
    }
#ifdef HAS_MSGHDR_FLAGS
    if (msgHdr.msg_flags & MSG_TRUNC) {
      _CustomErrno(eSocketErrno_MessagePartial);
      return -1;
    }
#endif

#else
    int sinLength = sizeof(sin);
    DWORD flags = 0;
    if (WSARecvFrom(mSocket,
                    (LPWSABUF)apBuffers,
                    (DWORD)anBufCount,
                    (LPDWORD)&recvLength,
                    &flags,
                    apAddress ? (struct sockaddr *)&sin : NULL,
                    apAddress ? &sinLength : NULL,
                    NULL,
                    NULL) == SOCKET_ERROR)
    {
      _UpdateErrno();
      switch (mLastError) {
        case eSocketErrno_WouldBlock:
        case eSocketErrno_ConnectionReset:
          return 0;
        default:
          return -1;
      }
    }
    if (flags & MSG_PARTIAL) {
      _CustomErrno(eSocketErrno_MessagePartial);
      return -1;
    }
#endif

    if (apAddress != NULL) {
      QPtr<iRemoteAddressIPv4> addr = apAddress;
      if (addr.IsOK()) {
        addr->SetHost((tU32)sin.sin_addr.s_addr);
        addr->SetPort(niSwapBE16(sin.sin_port));
      }
    }

    return (tI32)recvLength;
  }

  ///////////////////////////////////////////////
  virtual tSocketWaitFlags __stdcall Wait(tSocketWaitFlags aFlags, tU32 anTimeoutInMs) {
    CHECKVALID(eFalse);
    if (!aFlags) return 0;

#ifdef HAS_POLL
    struct pollfd pollSocket;
    int pollCount;

    pollSocket.fd = mSocket;
    pollSocket.events = 0;

    if (niFlagIs(aFlags,eSocketWaitFlags_Send))
      pollSocket.events |= POLLOUT;
    if (niFlagIs(aFlags,eSocketWaitFlags_Receive))
      pollSocket.events |= POLLIN;

    pollCount = poll(&pollSocket,1,anTimeoutInMs);
    if (pollCount < 0) {
      _UpdateErrno();
      return eSocketWaitFlags_Error;
    }

    if (pollCount == 0)
      return 0;

    tSocketWaitFlags ret = 0;

    if (pollSocket.revents & POLLOUT)
      ret |= eSocketWaitFlags_Send;
    if (pollSocket.revents & POLLIN)
      ret |= eSocketWaitFlags_Receive;

    return ret;
#else
    fd_set readSet, writeSet;
    timeval timeVal;

    timeVal.tv_sec = anTimeoutInMs / 1000;
    timeVal.tv_usec = (anTimeoutInMs % 1000) * 1000;

    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);

    if (niFlagIs(aFlags,eSocketWaitFlags_Send)) {
      FD_SET(mSocket,&writeSet);
    }

    if (niFlagIs(aFlags,eSocketWaitFlags_Receive)) {
      FD_SET(mSocket,&readSet);
    }

    int selectCount = select((int)(mSocket+1),&readSet,&writeSet,NULL,&timeVal);
    if (selectCount < 0) {
      _UpdateErrno();
      return eSocketWaitFlags_Error;
    }
    if (selectCount == 0)
      return 0;

    tSocketWaitFlags ret = 0;

    if (FD_ISSET(mSocket,&writeSet))
      ret |= eSocketWaitFlags_Send;
    if (FD_ISSET(mSocket,&readSet))
      ret |= eSocketWaitFlags_Receive;

    return ret;
#endif
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetNodeName(const achar* aaszName) {
    mstrNodeName = aaszName;
  }
  virtual const achar* __stdcall GetNodeName() const {
    return mstrNodeName.Chars();
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetKeepAlive(tBool aKeepAlive) {
    tI32 nKeepAlive = 1;
    socklen_t nOptSize = sizeof(nKeepAlive);
#if defined niPosix
    return setsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE, &nKeepAlive, nOptSize) == 0;
#elif defined niWindows
    return setsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&nKeepAlive, nOptSize) == 0;
#else
    niError("Not implemented");
    return eFalse;
#endif
  }
  virtual tBool __stdcall GetIsKeepAlive() const {
    tI32 nKeepAlive = 1;
    socklen_t nOptSize = sizeof(nKeepAlive);
#if defined niPosix
    if (getsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE, &nKeepAlive, &nOptSize) != 0) {
      niError("Can't get SO_KEEPALIVE.");
      return eFalse;
    }
    return nKeepAlive!=0;
#elif defined niWindows
    if (getsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&nKeepAlive, &nOptSize) != 0) {
      niError("Can't get SO_KEEPALIVE.");
      return eFalse;
    }
    return nKeepAlive!=0;
#else
    niError("Not implemented");
    return eFalse;
#endif
  }
  //////////////////////////////////////////////////////////////
  virtual tBool __stdcall SetKeepAliveParameters(tU64 anTimeMS, tU64 anIntvlMS, tU32 anProbes) {
#if defined niOSX
    if (anTimeMS!=0) {
      tU64 nTimeMS = anTimeMS / 1000;
      if (setsockopt(mSocket, IPPROTO_TCP, TCP_KEEPALIVE, &nTimeMS, sizeof(nTimeMS)) != 0) {
        niError("Can't set TCP_KEEPALIVE.");
        return eFalse;
      }
    }
    if (anIntvlMS!=0) {
      tU64 nIntvl = anIntvlMS / 1000;
      if (setsockopt(mSocket, IPPROTO_TCP, TCP_KEEPINTVL, &nIntvl, sizeof(nIntvl)) != 0) {
        niError("Can't set TCP_KEEPINTVL.");
        return eFalse;
      }
    }
    if (anProbes!=0) {
      if (setsockopt(mSocket, IPPROTO_TCP, TCP_KEEPCNT, &anProbes, sizeof(anProbes)) != 0) {
        niError("Can't set TCP_KEEPCNT.");
        return eFalse;
      }
    }
    return eTrue;
#elif defined niLinux
    if (anIntvlMS!=0) {
      tU64 nTimeMS = anTimeMS / 1000;
      if (setsockopt(mSocket, SOL_SOCKET, TCP_KEEPIDLE, &nTimeMS, sizeof(nTimeMS)) != 0) {
        niError("Can't set TCP_KEEPIDLE.");
        return eFalse;
      }
    }
    if (anIntvlMS!=0) {
      tU64 nIntvl = anIntvlMS / 1000;
      if (setsockopt(mSocket, SOL_SOCKET, TCP_KEEPINTVL, &nIntvl, sizeof(nIntvl)) != 0) {
        niError("Can't set TCP_KEEPINTVL.");
        return eFalse;
      }
    }
    if (anProbes!=0) {
      if (setsockopt(mSocket, SOL_SOCKET, TCP_KEEPCNT, &anProbes, sizeof(anProbes)) != 0) {
        niError("Can't set TCP_KEEPCNT.");
        return eFalse;
      }
    }
    return eTrue;
#elif defined niWindows
    DWORD nRet = 0;
    tcp_keepalive stKeepAlive;
    stKeepAlive.keepaliveinterval = anIntvlMS;
    stKeepAlive.keepalivetime = anTimeMS;
    stKeepAlive.onoff = 1;
    if (WSAIoctl(mSocket,SIO_KEEPALIVE_VALS,&stKeepAlive,sizeof(stKeepAlive),NULL,0,&nRet,NULL,NULL)!=0) {
      niError("Can't set SIO_KEEPALIVE_VALS.");
      return eFalse;
    }
    return eTrue;
#else
    niError("Not implemented.");
    return eFalse;
#endif
  }

  //////////////////////////////////////////////////
  virtual tU64 __stdcall GetKeepAliveTime() const {
#if defined niOSX
    tU32 nTimeMS = 0;
    socklen_t nOptSize = sizeof(nTimeMS);
    if (getsockopt(mSocket, IPPROTO_TCP, TCP_KEEPALIVE, &nTimeMS, &nOptSize) != 0) {
      niError("Can't get TCP_KEEPALIVE.");
      return 0;
    }
    return nTimeMS * 1000;
#elif defined niLinux
    tU32 nTimeMS = 0;
    socklen_t nOptSize = sizeof(nTimeMS);
    if (getsockopt(mSocket, SOL_SOCKET, TCP_KEEPIDLE, &nTimeMS, &nOptSize) != 0) {
      niError("Can't get TCP_KEEPIDLE.");
      return 0;
    }
    return nTimeMS * 1000;
#elif defined niWindows
    niError("Not supported.");
    return 0;
#else
    niError("Not implemented.");
    return 0;
#endif
  }
  virtual tU64 __stdcall GetKeepAliveInterval() const {
#if defined niOSX
    tU32 nIntvl = 0;
    socklen_t nOptSize = sizeof(nIntvl);
    if (getsockopt(mSocket, IPPROTO_TCP, TCP_KEEPINTVL, &nIntvl, &nOptSize) != 0) {
      niError("Can't get TCP_KEEPINTVL.");
      return 0;
    }
    return nIntvl * 1000;
#elif defined niLinux
    tU32 nIntvl = 0;
    socklen_t nOptSize = sizeof(nIntvl);
    if (getsockopt(mSocket, SOL_SOCKET, TCP_KEEPINTVL, &nIntvl, &nOptSize) != 0) {
      niError("Can't get TCP_KEEPINTVL.");
      return 0;
    }
    return nIntvl * 1000;
#elif defined niWindows
    niError("Not supported on the current platform.");
    return 0;
#else
    niError("Not implementated on the current platform.");
    return 0;
#endif
  }
  virtual tU32 __stdcall GetKeepAliveProbes() const {
#if defined niOSX
    tU32 nProbes = 0;
    socklen_t nOptSize = sizeof(nProbes);
    if (getsockopt(mSocket, IPPROTO_TCP, TCP_KEEPCNT, &nProbes, &nOptSize) != 0) {
      niError("Can't get TCP_KEEPCNT.");
      return 0;
    }
    return nProbes;
#elif defined niLinux
    tU32 nProbes = 0;
    socklen_t nOptSize = sizeof(nProbes);
    if (getsockopt(mSocket, SOL_SOCKET, TCP_KEEPCNT, &nProbes, &nOptSize) != 0) {
      niError("Can't get TCP_KEEPCNT.");
      return 0;
    }
    return nProbes;
#elif defined niWindows
    niError("Not supported");
    return 0;
#else
    niError("Not implemented");
    return 0;
#endif
  }

  ///////////////////////////////////////////////
  tBool _RemoteToSocketAddress(iRemoteAddress* apAddr, sockaddr_in& dest) {
    if (!niIsOK(apAddr)) return eFalse;
    if (apAddr->GetType() != eRemoteAddressType_IPv4) return eFalse;
    Ptr<iRemoteAddressIPv4> ra = ni::QueryInterface<iRemoteAddressIPv4>(apAddr);
    if (!ra.IsOK()) return eFalse;
    memset(&dest,0,sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_addr.s_addr = ra->GetHost();
    dest.sin_port = niSwapBE16(ra->GetPort());
    return eTrue;
  }
  tBool _SocketAddressToRemote(const sockaddr_in& src, iRemoteAddress* apAddr) {
    if (!niIsOK(apAddr)) return eFalse;
    if (apAddr->GetType() != eRemoteAddressType_IPv4) return eFalse;
    Ptr<iRemoteAddressIPv4> ra = ni::QueryInterface<iRemoteAddressIPv4>(apAddr);
    if (!ra.IsOK()) return eFalse;
    ra->SetHost(src.sin_addr.s_addr);
    ra->SetPort(niSwapBE16(src.sin_port));
    return eTrue;
  }

  ///////////////////////////////////////////////
  void _CustomErrno(eSocketErrno aErrno) {
    mRawLastError = -1;
    mLastError = aErrno;
  }

  niEndClass(cSocket);
};

iSocket* __stdcall cLang::CreateSocketFromHandle(
  ni::tInt aSocket,
  ni::iRemoteAddress* apAddr)
{
  return niNew cSocket(aSocket,apAddr);
}
iSocket* __stdcall cLang::CreateSocket(
  ni::eSocketProtocol aProtocol,
  ni::iRemoteAddress* apAddr)
{
  return niNew cSocket(aProtocol,apAddr);
}
void __stdcall cLang::CloseSocketHandle(ni::tInt aSocket) {
  cSocket::_CloseSocket(aSocket);
}

#undef CHECKVALID

#endif // niMinFeatures(15)
