#pragma once
#ifndef __TRESULTT_H_7B486319_C658_8A40_AB78_22605F67E43C__
#define __TRESULTT_H_7B486319_C658_8A40_AB78_22605F67E43C__

#include <niLang/Types.h>
#include <thrift/protocol/TProtocol.h>

namespace ni {

template <typename T>
struct TResultT {
private:
  T _result = {};

public:
  inline TResultT() {}

  inline TResultT(::apache::thrift::protocol::TProtocol* iproto) {
    this->Read(iproto);
  }

  inline tBool Read(::apache::thrift::protocol::TProtocol* iproto) {
    _result.read(iproto);
    return HasResult();
  }

  inline tBool HasResult() const {
    return !!_result.__isset.success;
  }

  inline auto& GetResult() {
    niPanicAssert(HasResult());
    return _result.success;
  }

  inline const auto& GetResult() const {
    niPanicAssert(HasResult());
    return _result.success;
  }

  inline tBool HasException() const {
    return !!_result.__isset.exc;
  }

  inline const auto &GetException() const {
    niPanicAssert(HasException());
    return _result.exc;
  }
};

template <typename T>
struct TVoidResultT {
private:
  T _result = {};

public:
  inline TVoidResultT() {}

  inline TVoidResultT(::apache::thrift::protocol::TProtocol* iproto) {
    this->Read(iproto);
  }

  inline void Read(::apache::thrift::protocol::TProtocol* iproto) {
    _result.read(iproto);
  }

  inline tBool HasException() const {
    return !!_result.__isset.exc;
  }

  inline const auto& GetException() const {
    niPanicAssert(HasException());
    return _result.exc;
  }
};

}
#endif // __TRESULTT_H_7B486319_C658_8A40_AB78_22605F67E43C__
