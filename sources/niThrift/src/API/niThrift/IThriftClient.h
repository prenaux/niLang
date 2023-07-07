#pragma once
#ifndef __ITHRIFTCLIENT_H_CD8F056C_98FC_1346_9F23_A405674729C1__
#define __ITHRIFTCLIENT_H_CD8F056C_98FC_1346_9F23_A405674729C1__

#include <niLang/Types.h>
#include <niLang/Utils/ConcurrentImpl.h>

namespace apache {
namespace thrift {
class TException;
namespace protocol {
class TProtocol;
}
}}

namespace ni {

enum eThriftResponse {
  //! Receive a thrift message response.
  //! \param A: iFile* with the received buffer.
  //! \param B: unused
  eThriftResponse_ReceivedResponse = niMessageID('_', 'T', 'H', 'R', 0),
  //! Receive a thrift message error.
  //! \param A: iFile* with the received buffer.
  //! \param B: unused
  eThriftResponse_ReceivedError = niMessageID('_', 'T', 'H', 'E', 0),
  //! \internal
  eThriftResponse_ForceDWORD = 0xFFFFFFFF
};

struct iThriftClientSink : public iUnknown {
  niDeclareInterfaceUUID(iThriftClientSink, 0x7b04c544,0x48b8,0xd34b,0xb7,0x2d,0xab,0x40,0x17,0xf6,0xd4,0x5d);

  virtual void __stdcall OnThriftClientConnected() = 0;

  virtual tBool __stdcall OnThriftClientResponse(const achar* aFuncName, ni::tI32 aSeqId, ::apache::thrift::protocol::TProtocol* apReadProtocol) = 0;

  virtual tBool __stdcall OnThriftClientException(const ::apache::thrift::TException& aException) = 0;
};

struct iThriftClient : public iUnknown {
  niDeclareInterfaceUUID(iThriftClient, 0x35af7ae3,0x8dfc,0x6a4d,0xbf,0x43,0x15,0x95,0x24,0x01,0x38,0xaa);

  virtual const achar* __stdcall GetServiceUrl() const = 0;

  virtual const achar* __stdcall GetServiceName() const = 0;

  //! Connect the thrift client.
  //! \remark Note that this is async, connection is confirmed by iThriftClientSink::OnThriftClientConnected
  virtual tBool __stdcall Connect() = 0;

  virtual tBool __stdcall DispatchThriftResponse(iFile* apReceivedBuffer) = 0;

  virtual tI32 __stdcall GetLastSentSeqId() const = 0;
};

}
#endif // __ITHRIFTCLIENT_H_CD8F056C_98FC_1346_9F23_A405674729C1__
