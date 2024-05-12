#include <niCURL.h>
#include <niCURL_ModuleDef.h>

#include <thrift/transport/TTransport.h>

#include "IThriftClient.h"

namespace ni {

using namespace apache::thrift::transport;

#define CONTENT_TYPE "application/x-thrift"

#ifndef niJSCC
// Leads to browser outputing "Refused to set unsafe header "Transfer-Encoding""
#define USE_EXPECT
#define USE_CHUNKED
#endif

struct TCurlWriterTransport : public TTransport {
protected:
  const ni::cString _url;
  ni::Nonnull<ni::tStringCVec> _headers;
  ni::Nonnull<ni::iCURL> _curl;
  ni::Nonnull<ni::iFile> _writeBuffer;
  ni::WeakPtr<ni::iMessageHandler> _msgHandler;

public:
  TCurlWriterTransport(const char* aURL, iMessageHandler* apMessageHandler)
      : _url(aURL)
      , _headers(ni::tStringCVec::Create())
      , _curl((iCURL*)niCreateInstance(niCURL, CURL, niVarNull, niVarNull))
      , _writeBuffer(ni::CreateFileDynamicMemory(128, "TCurlWriterTransport_writeBuffer"))
      , _msgHandler(apMessageHandler)
  {
#ifdef USE_CHUNKED
    _headers->push_back("Transfer-Encoding: chunked");
#endif
#ifdef USE_EXPECT
    _headers->push_back("Expect:");
#endif
    _headers->push_back("Content-Type:" CONTENT_TYPE);
    _headers->push_back("Accept:" CONTENT_TYPE);
  }

  virtual ~TCurlWriterTransport() {
  }

  void open() override {
  }

  bool isOpen() const override {
    return true;
  }

  bool peek() override {
    return true;
  }

  void close() override {
  }

  uint32_t read_virt(uint8_t* buf, uint32_t len) override {
    niPanicUnreachable("Can't read from TCurlClient.");
    return 0;
  }
  uint32_t readAll_virt(uint8_t* buf, uint32_t len) override {
    niPanicUnreachable("Can't readAll from TCurlClient.");
    return 0;
  }

  uint32_t readEnd() override {
    return 0;
  }

  void write_virt(const uint8_t* buf, uint32_t len) override {
    // niDebugFmt(("... Write: len: %d, %c (%d)", len, buf[0], buf[0]));
    _writeBuffer->WriteRaw(buf, len);
  }
  uint32_t writeEnd() override {
    return 0;
  }

  void flush() override {
    // niDebugFmt(("... flush: available_read: %s", _writeBuffer->GetSize()));

    // XXX: This is not great, but we need a new buffer since the fetch is async
    ni::Nonnull<ni::iFile> postData = _writeBuffer;

    // Create a new write buffer for the next call
    _writeBuffer = ni::Nonnull{ni::CreateFileDynamicMemory(128, "TCurlWriterTransport_writeBuffer")};

    // Get the message handler, bail if no one can hear us scream...
    Nonnull<iMessageHandler> mh = niCheckNonnull(mh, _msgHandler, ;);

    // rewind to the begining since we're gonna read the data...
    postData->SeekSet(0);
    const tU64 threadId = mh->GetThreadID();
    // niDebugFmt(("... postData string data: %s, to mh: %p,%d",
                // postData->ReadString(), (tIntPtr)mh.raw_ptr(), threadId));

    // lot of memory allocation... could be reused?
    ni::Nonnull<sThriftFetchRequestSink> fetchSink{
      niNew sThriftFetchRequestSink(postData, mh.non_null())};

    // Rewind before posting
    postData->SeekSet(0);
    ni::Ptr<iFetchRequest> req =
      _curl->FetchPost(_url.c_str(), postData, fetchSink.raw_ptr(), _headers);

    // we actually don't neeed to keep the request object explicitly its kept
    // alive as long as the request is in flight
    niUnused(req);
  }

  virtual const std::string getOrigin() const override {
    return "TCurlWriterTransport";
  }

  struct sThriftFetchRequestSink : public ImplRC<iFetchSink> {
    Nonnull<iFile> _postData;
    Nonnull<iMessageHandler> _msgHandler;

    sThriftFetchRequestSink(
      Nonnull<iFile> aPostData,
      Nonnull<iMessageHandler> aMessageHandler)
        : _postData(aPostData)
        , _msgHandler(aMessageHandler) {
    }

    virtual void __stdcall OnFetchSink_Success(iFetchRequest* apFetch) override {
      // niDebugFmt(
      // ("... OnFetchSink_Success: status: %d, got %d bytes",
      // apFetch->GetStatus(),
      // apFetch->GetReceivedData()->GetSize()));

      ni::GetConcurrent()->SendMessage(
        _msgHandler,
        eThriftResponse_ReceivedResponse,
        apFetch->GetReceivedData(),
        niVarNull);
    }
    virtual void __stdcall OnFetchSink_Error(iFetchRequest* apFetch) override {
      // niDebugFmt(
        // ("... OnFetchSink_Error: status: %d, got %d bytes",
         // apFetch->GetStatus(),
         // apFetch->GetReceivedData()->GetSize()));

      ni::GetConcurrent()->SendMessage(
        _msgHandler,
        eThriftResponse_ReceivedError,
        apFetch->GetReceivedData(),
        niVarNull);
    }
    virtual void __stdcall OnFetchSink_Progress(iFetchRequest* apFetch) override {
      // niDebugFmt(
      // ("... OnFetchSink_Progress: status: %d, got %d bytes",
      // apFetch->GetStatus(),
      // apFetch->GetReceivedData()->GetSize()));
    }
    virtual void __stdcall OnFetchSink_ReadyStateChange(
      iFetchRequest* apFetch) override {
      // niDebugFmt(
      // ("... OnFetchSink_ReadyStateChanged: %s",
      // niEnumToChars(eFetchReadyState, apFetch->GetReadyState())));
    }
  };
};


}
