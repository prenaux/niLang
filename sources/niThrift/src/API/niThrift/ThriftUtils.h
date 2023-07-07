#pragma once
#ifndef __THRIFTUTILS_H_B6720A61_EE47_ED11_8C9A_EF92CA7B3E55__
#define __THRIFTUTILS_H_B6720A61_EE47_ED11_8C9A_EF92CA7B3E55__

#include <niLang/StringDef.h>
#include <thrift/TToString.h>
#include <thrift/protocol/TEnum.h>

namespace ni {

template <typename T>
inline cString TToString(const T& t) {
  // XXX: So horribly inefficent... but does the job for a prototype
  return ::apache::thrift::to_string(t).c_str();
}

inline const char* TTypeToString(const ::apache::thrift::protocol::TType v) {
  using namespace ::apache::thrift::protocol;

  switch (v) {
    case T_STOP: return "STOP";
    case T_VOID: return "VOID";
    case T_BOOL: return "BOOL";
    case T_BYTE: return "BYTE";
    // case T_I08: return "I08";
    case T_I16: return "I16";
    case T_I32: return "I32";
    case T_U64: return "U64";
    case T_I64: return "I64";
    case T_DOUBLE: return "DOUBLE";
    case T_STRING: return "STRING";
    // case T_UTF7: return "UTF7";
    case T_STRUCT: return "STRUCT";
    case T_MAP: return "MAP";
    case T_SET: return "SET";
    case T_LIST: return "LIST";
    case T_UTF8: return "UTF8";
    case T_UTF16: return "UTF16";
  }
  return "UNKNOWN";
};

/**
 * Enumerated definition of the message types that the Thrift protocol
 * supports.
 */
inline const achar* TMessageTypeToString(const ::apache::thrift::protocol::TMessageType v) {
  using namespace ::apache::thrift::protocol;

  switch (v) {
    case T_CALL: return "CALL";
    case T_REPLY: return "REPLY";
    case T_EXCEPTION: return "EXCEPTION";
    case T_ONEWAY: return "ONEWAY";
  }
  return "UNKNOWN";
}

}  // namespace ni
#endif  // __THRIFTUTILS_H_B6720A61_EE47_ED11_8C9A_EF92CA7B3E55__
