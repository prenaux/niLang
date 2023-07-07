#pragma once
#ifndef __TSEQPROTOCOL_H_864FFB6D_4048_9649_93C5_F670CAE077F6__
#define __TSEQPROTOCOL_H_864FFB6D_4048_9649_93C5_F670CAE077F6__

#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

namespace ni {

using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

template<typename Proto>
class TSeqProtocol : public Proto
{
 public:
  TSeqProtocol(std::shared_ptr<TTransport> transport)
      : Proto(transport) { }

  virtual uint32_t writeMessageBegin_virt(const std::string& name,
                                          const TMessageType messageType,
                                          const int32_t in_seqid) override
  {
    int32_t seqid = in_seqid;
    if (seqid == 0) { // this is typical for normal cpp generated code
      seqid = ++mLastSeqId;
    }
    mLastSentSeqId = seqid;
    return Proto::writeMessageBegin_virt(name, messageType, seqid);
  }

  virtual uint32_t readMessageBegin_virt(std::string& name,
                                         TMessageType& messageType,
                                         int32_t& seqid) override
  {
    uint32_t result = Proto::readMessageBegin_virt(name, messageType, seqid);
    return result;
  }

  int32_t GetLastSentSeqId() const {
    return mLastSeqId;
  }

 private:
  // start at 1000 because its more readable in debug output
  int32_t mLastSeqId = 1000;
  int32_t mLastSentSeqId = -1;
};

}
#endif // __TSEQPROTOCOL_H_864FFB6D_4048_9649_93C5_F670CAE077F6__
