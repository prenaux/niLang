/**
 *  Copyright (C) 2007-2013 TalanSoft, Pierre Renaux
 */
#include "API/niLang/ISocket.h"
#include "Lang.h"

#ifdef niNoSocket
namespace ni {
iRemoteAddressIPv4* cLang::CreateRemoteAddressIPv4FromString(const char* aAddress) {
  niError("Not implemented.");
  return NULL;
}
iRemoteAddressIPv4* cLang::CreateRemoteAddressIPv4(tU32 anIP, tU32 anPort) {
  niError("Not implemented.");
  return NULL;
}
iRemoteAddressIPv4* cLang::CreateRemoteAddressIPv4ROFromString(const char* aAddress) {
  niError("Not implemented.");
  return NULL;
}
iRemoteAddressIPv4* cLang::CreateRemoteAddressIPv4RO(tU32 anIP, tU32 anPort) {
  niError("Not implemented.");
  return NULL;
}
}
#else

#include "API/niLang/Utils/RemoteAddress.h"
#include "API/niLang/Utils/UnknownImpl.h"

#ifdef niWindows
#include <winsock2.h>
#ifdef niPragmaCommentLib
#pragma comment(lib,"Ws2_32.lib")
#endif
#elif defined niPosix
// Posix
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

using namespace ni;

//! Remote address IPv4 template implementation.
template <tIUnknownImplFlags FLAGS>
class cRemoteAddressIPv4Tpl : public cIUnknownImpl<iRemoteAddressIPv4,FLAGS|eIUnknownImplFlags_DontInherit1,iRemoteAddress,iToString>
{
  niBeginClass(cRemoteAddressIPv4);

 public:
  ///////////////////////////////////////////////
  cRemoteAddressIPv4Tpl() {
    ZeroMembers();
  }

  ///////////////////////////////////////////////
  cRemoteAddressIPv4Tpl(const achar* aaszAddress) {
    ZeroMembers();
    SetAddressString(aaszAddress);
  }

  ///////////////////////////////////////////////
  cRemoteAddressIPv4Tpl(tU32 anIP, tU32 anPort) {
    ZeroMembers();
    SetHost(anIP);
    SetPort(anPort);
  }

  ///////////////////////////////////////////////
  ~cRemoteAddressIPv4Tpl() {
    Invalidate();
  }

  ///////////////////////////////////////////////
  void __stdcall Invalidate() {
  }

  ///////////////////////////////////////////////
  void __stdcall ZeroMembers() {
    memset(&_addr,0,sizeof(_addr));
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = INADDR_ANY;
    _addr.sin_port = 0;
  }

  ///////////////////////////////////////////////
  ni::tBool __stdcall IsOK() const {
    return ni::eTrue;
  }

  ///////////////////////////////////////////////
  virtual iRemoteAddress* __stdcall Clone() const {
    cRemoteAddressIPv4Tpl<eIUnknownImplFlags_Default>* pNew = niNew cRemoteAddressIPv4Tpl<eIUnknownImplFlags_Default>();
    pNew->_addr = _addr;
    return pNew;
  }

  ///////////////////////////////////////////////
  virtual tI32 __stdcall Compare(iRemoteAddress* apAddr) {
    if (!niIsOK(apAddr)) return 1;  // error, greater...
    if (GetType() != apAddr->GetType())
      return GetType() > apAddr->GetType() ? 1 : -1;
    Ptr<iRemoteAddressIPv4> ra = ni::QueryInterface<iRemoteAddressIPv4>(apAddr);
    niAssert(ra.IsOK());
    if (ra.IsOK()) {
      if (GetPort() != ra->GetPort())
        return GetPort() > ra->GetPort() ? 1 : -1;
      if (GetHost() != ra->GetHost())
        return GetHost() > ra->GetHost() ? 1 : -1;
    }
    return 0;
  }

  ///////////////////////////////////////////////
  virtual eRemoteAddressType __stdcall GetType() const {
    return eRemoteAddressType_IPv4;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall SetAddressString(const achar* aaszAddress) {
    // resolve the address
    cString cstrAddress;
    cString cstrPort;
    cString cstrAddressCopy(aaszAddress);
    const achar* p = cstrAddressCopy.Chars();
    tBool bInPort = eFalse;
    while (*p) {
      tU32 cur = StrGetNext(p);
      if (cur == ':') {
        const tU32 next = StrGetNext(p+1);
        if (StrIsDigit(next)) {
          cur = next;
          bInPort = eTrue;
          ++p;    // skip the ':' char
        }
        else if (StrNICmp(p,"://",3) == 0) {
          cstrAddress.Clear();
          p += 3; // skip "://"
        }
      }
      if (bInPort) {
        cstrPort.appendChar(cur);
      }
      else {
        cstrAddress.appendChar(cur);
      }
      StrGetNextX(&p);
    }

    if (cstrPort.IsNotEmpty())
      SetPort(cstrPort.Long());

    if (cstrAddress.IsNotEmpty())
    {
      if ((cstrAddress.Len() >= 7) && // min ip length is 7 chars : 0.0.0.0
          (cstrAddress[0] >= '0' && cstrAddress[0] <= '9') &&
          ((cstrAddress[1] >= '0' && cstrAddress[1] <= '9') || cstrAddress[1] == '.') &&
          ((cstrAddress[2] >= '0' && cstrAddress[2] <= '9') || cstrAddress[2] == '.'))
      { // 127.0.0.1 address type
        _addr.sin_addr.s_addr = inet_addr(cstrAddress.Chars());
      }
      else if (cstrAddress == "loopback") {
        _addr.sin_addr.s_addr = INADDR_LOOPBACK;
      }
      else if (cstrAddress == "any") {
        _addr.sin_addr.s_addr = INADDR_ANY;
      }
      else if (cstrAddress == "broadcast") {
        _addr.sin_addr.s_addr = INADDR_BROADCAST;
      }
      else
      { // www.address.xxx address type
#ifdef niPosix
        struct hostent* h;
#else
        HOSTENT* h;
#endif
        h = gethostbyname(cstrAddress.Chars());
        if (!h) {
          // ERROR
        }
        else {
          *(int*)(&_addr.sin_addr) = *(int*)h->h_addr_list[0];
        }
      }
    }
    else
    { // no address, something like :40000 (where 40000 is the port) has been
      // passed.
      _addr.sin_family = AF_INET;
      _addr.sin_addr.s_addr = INADDR_ANY;
    }
    return eTrue;
  }

  ///////////////////////////////////////////////
  virtual cString __stdcall GetAddressString() const {
    tU32 ip = GetHost();
    cString str;
    str << (tU32)niFourCCA(ip) << _A(".")
        << (tU32)niFourCCB(ip) << _A(".")
        << (tU32)niFourCCC(ip) << _A(".")
        << (tU32)niFourCCD(ip);
    tU32 port = GetPort();
    if (port)
      str << _A(":") << port;
    return str;
  }

  ///////////////////////////////////////////////
  virtual tBool __stdcall GetIsNull() const {
    return _addr.sin_addr.s_addr == 0;
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetPort(tU32 anPort) {
    _addr.sin_port = niSwapBE16((tU16)anPort);
  }
  virtual tU32 __stdcall GetPort() const {
    return niSwapBE16(_addr.sin_port);
  }

  ///////////////////////////////////////////////
  virtual void __stdcall SetHost(tU32 anFourCC) {
    if (anFourCC == eRemoteAddressIPv4Reserved_Any) {
      _addr.sin_family = AF_INET;
      _addr.sin_addr.s_addr = INADDR_ANY;
    }
    else if (anFourCC == eRemoteAddressIPv4Reserved_Broadcast) {
      _addr.sin_family = AF_INET;
      _addr.sin_addr.s_addr = INADDR_BROADCAST;
    }
    else {
      _addr.sin_family = AF_INET;
      _addr.sin_addr.s_addr = anFourCC;
    }
  }
  virtual tU32 __stdcall GetHost() const {
    switch (_addr.sin_addr.s_addr) {
      case INADDR_ANY:    return eRemoteAddressIPv4Reserved_Any;
      case INADDR_BROADCAST:  return eRemoteAddressIPv4Reserved_Broadcast;
      default:        return _addr.sin_addr.s_addr;
    }
  }

  ///////////////////////////////////////////////
  cString __stdcall GetHostName() const {
    struct in_addr in;
    struct hostent * hostEntry;
    in.s_addr = _addr.sin_addr.s_addr;
    hostEntry = gethostbyaddr ((char *) & in, sizeof (struct in_addr), AF_INET);
    if (hostEntry == NULL)
      return AZEROSTR;
    return cString(hostEntry -> h_name);
  }

  ///////////////////////////////////////////////
  void SetSockAddrIn(sockaddr_in* apSockAddrIn) {
    _addr = *apSockAddrIn;
  }
  const sockaddr_in* GetSockAddrIn() const {
    return &_addr;
  }

  cString __stdcall ToString() const {
    return GetAddressString();
  }

  sockaddr_in _addr;
  niEndClass(cRemoteAddressIPv4);
};

//! Read-only remote address IPv4 template implementation.
template <tIUnknownImplFlags FLAGS>
class cRemoteAddressIPv4ROTpl : public cRemoteAddressIPv4Tpl<FLAGS>
{
 public:
  ///////////////////////////////////////////////
  cRemoteAddressIPv4ROTpl(const achar* aaszAddress) : cRemoteAddressIPv4Tpl<FLAGS>(aaszAddress) {}
  ///////////////////////////////////////////////
  cRemoteAddressIPv4ROTpl(tU32 anIP, tU32 anPort) : cRemoteAddressIPv4Tpl<FLAGS>(anIP,anPort) {}

  virtual tBool __stdcall SetAddressString(const achar* aaszAddress) { return eFalse; }
  virtual void __stdcall SetHost(tU32 anFourCC) {}
  virtual void __stdcall SetPort(tU32 anPort) {}
};

typedef cRemoteAddressIPv4Tpl<eIUnknownImplFlags_Default> cRemoteAddressIPv4;
typedef cRemoteAddressIPv4ROTpl<eIUnknownImplFlags_Default> cRemoteAddressIPv4RO;

niExportFunc(iRemoteAddressIPv4*) CreateRemoteAddressIPv4FromSockAddr(sockaddr_in* apSockAddrIn) {
  Ptr<cRemoteAddressIPv4> ip = niNew cRemoteAddressIPv4();
  ip->SetSockAddrIn(apSockAddrIn);
  return ip.GetRawAndSetNull();
}

iRemoteAddressIPv4* cLang::CreateRemoteAddressIPv4FromString(const char* aAddress) {
  return niNew cRemoteAddressIPv4(aAddress);
}
iRemoteAddressIPv4* cLang::CreateRemoteAddressIPv4(tU32 anIP, tU32 anPort) {
  return niNew cRemoteAddressIPv4(anIP,anPort);
}
iRemoteAddressIPv4* cLang::CreateRemoteAddressIPv4ROFromString(const char* aAddress) {
  return niNew cRemoteAddressIPv4RO(aAddress);
}
iRemoteAddressIPv4* cLang::CreateRemoteAddressIPv4RO(tU32 anIP, tU32 anPort) {
  return niNew cRemoteAddressIPv4RO(anIP,anPort);
}

#endif
