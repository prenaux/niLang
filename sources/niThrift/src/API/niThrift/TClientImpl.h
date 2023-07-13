#pragma once
#ifndef __TCLIENTIMPL_H_E177FD19_1EBB_0744_9486_E42F1286AA07__
#define __TCLIENTIMPL_H_E177FD19_1EBB_0744_9486_E42F1286AA07__

#include <niCC.h>
#include "TNiFileTransport.h"
#include "TCurlWriterTransport.h"
#include "TSeqProtocol.h"
#include "TResultT.h"
#include "ThriftUtils.h"
#include "IThriftClient.h"

#include <thrift/TApplicationException.h>
#include <thrift/protocol/TJSONProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>

namespace ni {

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

typedef TSeqProtocol<TJSONProtocol> tThriftClientProtocol;

template <typename TCLIENT>
struct TClientImpl : public cIUnknownImpl<iThriftClient,eIUnknownImplFlags_Default,iThriftClientSink,iMessageHandler> {
  typedef TClientImpl<TCLIENT> tBase;

protected:
  const tU64 _threadId;
  const cString _serviceUrl;
  const cString _serviceName;
  std::shared_ptr<TNiFileReaderTransport> _readerTransport;
  std::shared_ptr<tThriftClientProtocol> _readProtocol;
  std::shared_ptr<TTransport> _writerTransport;
  std::shared_ptr<tThriftClientProtocol> _writeProtocol;
  std::shared_ptr<TProtocol> _clientProtocol;
  std::shared_ptr<TCLIENT> _client;

  virtual tU64 __stdcall GetThreadID() const niImpl {
    return _threadId;
  }
  virtual void __stdcall HandleMessage(const tU32 anMsg, const Var& avarA, const Var& avarB) niOverride {
    switch (anMsg) {
      case eThriftResponse_ReceivedError: {
        // XXX: Not sure what we can get out of the buffer? Probably unsafe to just read a string off it.
        // Ptr<iFile> fp = VarQueryInterface<iFile>(avarA);
        TApplicationException exception("ThriftIOError");
        this->OnThriftClientException(exception);
        break;
      }
      case eThriftResponse_ReceivedResponse: {
        Nonnull<iFile> fp = niCheckNN(fp, VarQueryInterface<iFile>(avarA), ;);
        this->DispatchThriftResponse(fp);
        break;
      }
    }
  }

  virtual tBool __stdcall OnThriftClientException(const ::apache::thrift::TException& aException) niOverride {
    niError(niFmt("ThriftClient(%s,%s): TApplicationException: %s.",
                  _serviceUrl, _serviceName,
                  aException.what()));
    return eFalse;
  }

public:
  TClientImpl(const achar* aURL, const char* aServiceName)
      : _threadId(ni::ThreadGetCurrentThreadID())
      , _serviceUrl(aURL)
      , _serviceName(aServiceName)
  {
    // niDebugFmt(("... TClientImpl: threadId: %d, serviceUrl: %s, serviceName: %s",
                // _threadId, _serviceUrl, _serviceName));
  }

  virtual const achar* __stdcall GetServiceUrl() const niImpl {
    return _serviceUrl.c_str();
  }

  virtual const achar* __stdcall GetServiceName() const niImpl {
    return _serviceName.c_str();
  }

  virtual tI32 __stdcall GetLastSentSeqId() const niImpl {
    return _writeProtocol->GetLastSentSeqId();
  }

  TCLIENT* __stdcall GetClient() const {
    return _client.get();
  }

  virtual tBool __stdcall Connect() niOverride {
    try {
      _readerTransport = std::make_shared<TNiFileReaderTransport>(ni::AsNonnull(
        ni::GetLang()->CreateFileWriteDummy()));
      _readProtocol = std::make_shared<tThriftClientProtocol>(_readerTransport);

      _writerTransport = std::make_shared<TCurlWriterTransport>(_serviceUrl.c_str(),this);
      _writeProtocol = std::make_shared<tThriftClientProtocol>(_writerTransport);

      if (_serviceName.IsEmpty()) {
        niLog(Info, niFmt("Connecting to default service at '%s'.",
                          _serviceUrl));
        _clientProtocol = _writeProtocol;
      }
      else {
        niLog(Info, niFmt("Connecting to multiplexed service '%s' at '%s'.",
                          _serviceName, _serviceUrl));
        _clientProtocol = std::make_shared<TMultiplexedProtocol>(_writeProtocol, _serviceName.c_str());
      }

      _client = std::make_shared<TCLIENT>(_readProtocol, _clientProtocol);

      _writerTransport->open(); // make sure our transport is opened...

      this->OnThriftClientConnected();
      return eTrue;
    }
    catch (TException& exc) {
      this->OnThriftClientException(exc);
      return eFalse;
    }
  }

  virtual tBool __stdcall DispatchThriftResponse(iFile* apReceivedBuffer) niOverride
  {
    niCheck(_client != nullptr, eFalse);
    niCheckIsOK(apReceivedBuffer, eFalse);
    try {
      tBool r = eFalse;

      _readerTransport->_readBuffer = AsNonnull(apReceivedBuffer);
      // niDebugFmt(("received: %s", _readerTransport->_readBuffer->ReadString()));

      _readerTransport->_readBuffer->SeekSet(0);
      std::string fname;
      protocol::TMessageType mtype;
      int32_t seqid;
      _readProtocol->readMessageBegin(fname, mtype, seqid);
      // niDebugFmt((
        // "readMessageBegin: name: %s, msgType: %s, seqid: %d",
        // fname.c_str(), TMessageTypeToString(mtype), (tU32)seqid));

      if (mtype == ::apache::thrift::protocol::T_EXCEPTION) {
        // General thrift exception
        ::apache::thrift::TApplicationException x;
        x.read(_readProtocol.get());
        r = this->OnThriftClientException(x);
      }
      else {
        if (mtype != ::apache::thrift::protocol::T_REPLY) {
          _readProtocol->skip(::apache::thrift::protocol::T_STRUCT);
          _readProtocol->readMessageEnd();
          _readProtocol->getTransport()->readEnd();
        }

        r = this->OnThriftClientResponse(fname.c_str(), seqid, _readProtocol.get());
      }

      _readProtocol->readMessageEnd();
      _readProtocol->getTransport()->readEnd();

      return r;
    }
    catch (TException& exc) {
      this->OnThriftClientException(exc);
      return eFalse;
    }
  }
};

}
#endif // __TCLIENTIMPL_H_E177FD19_1EBB_0744_9486_E42F1286AA07__
