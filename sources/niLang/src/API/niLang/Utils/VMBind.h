#ifndef __VMBIND_H_A854CA3F_506A_4385_8D9B_BB4FE84E05EE__
#define __VMBIND_H_A854CA3F_506A_4385_8D9B_BB4FE84E05EE__

#include "../Types.h"
#include "../ObjModel.h"
#include "../STL/type_traits.h"
#include "../STL/tuple.h"
#include "./VMCallCImpl.h"

/** \addtogroup niLang
 * @{
 */
/** \addtogroup niLang_VMAPI
 * @{
 */

EA_DISABLE_CLANG_WARNING(-Wc++23-extensions);

namespace ni {
namespace vmbind {

// Base param_traits template with read/write operations
template<typename T>
struct param_traits {
  static constexpr tType type = eType_Variant;
  static constexpr const char* type_name = "Var";
};

// Macro to declare param_traits for basic types
#define DECLARE_VMBIND_PARAM_TRAITS(TYPE, VM_TYPE, TYPE_NAME) \
  template<> struct param_traits<TYPE> {                      \
    static constexpr tType type = VM_TYPE;                    \
    static constexpr const char* type_name = TYPE_NAME;       \
    static void read(const Var& aVar, TYPE* value) {          \
      vmcall::BufRead(aVar, value);                           \
    }                                                         \
    static void write(Var& aVar, TYPE* value) {               \
      vmcall::BufWrite(aVar, value);                          \
    }                                                         \
  }

// Macro to declare param_traits for enum types
#define DECLARE_VMBIND_PARAM_ENUM_TRAITS(TYPE, VM_TYPE, TYPE_NAME)  \
  template<> struct param_traits<TYPE> {                            \
    static constexpr tType type = VM_TYPE;                          \
    static constexpr const char* type_name = TYPE_NAME;             \
    static void read(const Var& aVar, TYPE* value) {                \
      ni::tU32 tmpVal = 0;                                          \
      ni::vmcall::BufRead(aVar,&tmpVal);                            \
      *value = (TYPE)tmpVal;                                        \
    }                                                               \
    static void write(Var& aVar, TYPE* value) {                     \
      ni::tU32 tmpVal = *value;                                     \
      vmcall::BufWrite(aVar, &tmpVal);                              \
    }                                                               \
  }

DECLARE_VMBIND_PARAM_TRAITS(ni::tI8, eType_I8, "tI8");
DECLARE_VMBIND_PARAM_TRAITS(ni::tU8, eType_U8, "tU8");
DECLARE_VMBIND_PARAM_TRAITS(ni::tI16, eType_I16, "tI16");
DECLARE_VMBIND_PARAM_TRAITS(ni::tU16, eType_U16, "tU16");
DECLARE_VMBIND_PARAM_TRAITS(ni::tI32, eType_I32, "tI32");
DECLARE_VMBIND_PARAM_TRAITS(ni::tU32, eType_U32, "tU32");
DECLARE_VMBIND_PARAM_TRAITS(ni::tI64, eType_I64, "tI64");
DECLARE_VMBIND_PARAM_TRAITS(ni::tU64, eType_U64, "tU64");
DECLARE_VMBIND_PARAM_TRAITS(ni::tF32, eType_F32, "tF32");
DECLARE_VMBIND_PARAM_TRAITS(ni::tF64, eType_F64, "tF64");
DECLARE_VMBIND_PARAM_TRAITS(const ni::achar*, eType_ASZ, "const achar*");
DECLARE_VMBIND_PARAM_TRAITS(ni::cString, eType_String, "cString");
DECLARE_VMBIND_PARAM_TRAITS(ni::sVec2f, eType_Vec2f, "sVec2f");
DECLARE_VMBIND_PARAM_TRAITS(ni::sVec3f, eType_Vec3f, "sVec3f");
DECLARE_VMBIND_PARAM_TRAITS(ni::sVec4f, eType_Vec4f, "sVec4f");
DECLARE_VMBIND_PARAM_TRAITS(ni::sVec2i, eType_Vec2i, "sVec2i");
DECLARE_VMBIND_PARAM_TRAITS(ni::sVec3i, eType_Vec3i, "sVec3i");
DECLARE_VMBIND_PARAM_TRAITS(ni::sVec4i, eType_Vec4i, "sVec4i");
DECLARE_VMBIND_PARAM_TRAITS(ni::sMatrixf, eType_Matrixf, "sMatrixf");
DECLARE_VMBIND_PARAM_TRAITS(ni::sColor3ub, eType_U32, "sColor3ub");
DECLARE_VMBIND_PARAM_TRAITS(ni::sColor4ub, eType_U32, "sColor4ub");
DECLARE_VMBIND_PARAM_TRAITS(ni::tUUID, eType_UUID, "tUUID");

// Specialization for interface pointers
template<typename T>
struct param_traits<T*> {
  static constexpr tType type = eType_IUnknownPtr;
  static constexpr const char* type_name = "iUnknown*";

  static void read(const Var& aVar, T** value) {
    if constexpr (astl::is_base_of_v<iUnknown, astl::remove_pointer_t<T>>) {
      vmcall::BufReadIntf<T>(aVar, value);
    } else {
      vmcall::BufRead(aVar, value);
    }
  }

  static void write(Var& aVar, T** value) {
    if constexpr (astl::is_base_of_v<iUnknown, astl::remove_pointer_t<T>>) {
      vmcall::BufWriteIntf<T>(aVar, value);
    } else {
      vmcall::BufWrite(aVar, value);
    }
  }
};

// Specialization for const interface pointers
template<typename T>
struct param_traits<const T*> {
  static constexpr tType type = eType_IUnknownPtr;
  static constexpr const char* type_name = "const iUnknown*";

  static void read(const Var& aVar, const T** value) {
    if constexpr (astl::is_base_of_v<iUnknown, astl::remove_pointer_t<T>>) {
      vmcall::BufReadIntf<T>(aVar, const_cast<T**>(value));
    } else {
      vmcall::BufRead(aVar, value);
    }
  }

  static void write(Var& aVar, const T** value) {
    if constexpr (astl::is_base_of_v<iUnknown, astl::remove_pointer_t<T>>) {
      vmcall::BufWriteIntf<T>(aVar, value);
    } else {
      vmcall::BufWrite(aVar, value);
    }
  }
};

// Specialization for Ptr<T>
template<typename T>
struct param_traits<Ptr<T>> {
  static constexpr tType type = eType_IUnknownPtr;
  static constexpr const char* type_name = "Ptr<T>";

  static void read(const Var& aVar, Ptr<T>* value) {
    T* ptr = nullptr;
    vmcall::BufReadIntf<T>(aVar, &ptr);
    *value = ptr;
  }

  static void write(Var& aVar, Ptr<T>* value) {
    vmcall::BufWriteIntf<T>(aVar, value);
  }
};

#ifdef niTypeIntIsOtherType
DECLARE_VMBIND_PARAM_TRAITS(ni::tInt, eType_Int, "tInt");
#endif

#ifdef niTypeIntPtrIsOtherType
DECLARE_VMBIND_PARAM_TRAITS(ni::tIntPtr, eType_IntPtr, "tIntPtr");
#endif

// Check if type has _GetInterfaceUUID
template<typename T>
concept HasInterfaceUUID = requires(T) {
  { T::_GetInterfaceUUID() } -> astl::same_as<const tUUID&>;
};

template<typename T>
concept HasInterfaceID = requires(T) {
  { T::_GetInterfaceID() } -> astl::same_as<const ni::achar*>;
};

// Simple helper to get UUID
template<typename T>
const tUUID* get_interface_uuid() {
  if constexpr (HasInterfaceUUID<astl::remove_pointer_t<T>>) {
    return &astl::remove_pointer_t<T>::_GetInterfaceUUID();
  }
  else {
    return nullptr;
  }
}

template<typename T>
const achar* get_type_name() {
  if constexpr (HasInterfaceID<astl::remove_pointer_t<T>>) {
    return astl::remove_pointer_t<T>::_GetInterfaceID();
  }
  else {
    return param_traits<T>::type_name;
  }
}

// Helper for parameter definitions
template<typename... Args>
struct param_def_gen {
  static constexpr size_t param_count = sizeof...(Args);

  static constexpr const sParameterDef* make_param_defs(const achar* const* names) {
    if constexpr (sizeof...(Args) == 0) {
      return nullptr;
    } else {
      static const sParameterDef params[] = {
        sParameterDef{
          nullptr,  // name will be set later if needed
          param_traits<Args>::type,
          get_interface_uuid<Args>(),
          get_type_name<Args>()
        }...
      };
      return params;
    }
  }
};

// Static function wrapper
template<typename Ret, typename... Args>
struct static_wrapper {
  using function_type = Ret (*)(Args...);
  using args_tuple = eastl::tuple<std::remove_cvref_t<Args>...>;

  template<size_t... Is>
  static tInt call_impl(const Var* aArgs, args_tuple& args, astl::index_sequence<Is...>) {
    (param_traits<astl::remove_cvref_t<astl::tuple_element_t<Is, astl::tuple<Args...>>>>::read(
      aArgs[Is], &astl::get<Is>(args)), ...);
    return eVMRet_OK;
  }

  static tInt call(iUnknown* apThis, const Var* aArgs, tU32 aNumArgs, Var* apRet) {
    if (aNumArgs != sizeof...(Args))
      return eVMRet_InvalidArgCount;

    args_tuple args;

    // Read arguments
    if constexpr (sizeof...(Args) > 0) {
      call_impl(aArgs, args, astl::make_index_sequence<sizeof...(Args)>());
    }

    // Call function
    if constexpr (astl::is_void_v<Ret>) {
      astl::apply(function, args);
      if (apRet) apRet->SetNull();
    } else {
      auto result = astl::apply(function, args);
      if (apRet) {
        if constexpr (astl::is_base_of_v<iUnknown, astl::remove_pointer_t<Ret>>) {
          param_traits<Ret>::write(*apRet, &result);
        } else {
          param_traits<Ret>::write(*apRet, &result);
        }
      }
    }

    return eVMRet_OK;
  }

  static sMethodDef make_method_def(const achar* name, function_type f, tType aRetFlags) {
    function = f;
    return {
      name,
      param_traits<Ret>::type|aRetFlags,
      get_interface_uuid<Ret>(),
      get_type_name<Ret>(),
      sizeof...(Args),
      param_def_gen<Args...>::make_param_defs(nullptr),
      &call
    };
  }

 private:
  static function_type function;
};

template<typename Ret, typename... Args>
typename static_wrapper<Ret, Args...>::function_type static_wrapper<Ret, Args...>::function;

// Static registration helper
class static_registrar {
 private:
  template<typename R, typename... Args>
  static sMethodDef make_static_impl(const achar* name, R(*func)(Args...), tType aRetFlags) {
    return static_wrapper<R, Args...>::make_method_def(name, func, aRetFlags);
  }

 public:
  template<auto Function>
  static sMethodDef make_static(const achar* name, tType aRetFlags = 0) {
    return make_static_impl(name, Function, aRetFlags);
  }
};

// Method wrapper for class methods
template<typename Class, typename Ret, typename... Args>
struct method_wrapper {
  using method_type = Ret (Class::*)(Args...);
  using const_method_type = Ret (Class::*)(Args...) const;
  using args_tuple = eastl::tuple<std::remove_cvref_t<Args>...>;

  // Non-const version
  static sMethodDef make_method_def(const achar* name, method_type m, tType aRetFlags) {
    method = m;
    return {
      name,
      param_traits<Ret>::type|aRetFlags,
      get_interface_uuid<Ret>(),
      get_type_name<Ret>(),
      sizeof...(Args),
      param_def_gen<Args...>::make_param_defs(nullptr),
      &call
    };
  }

  // Const version
  static sMethodDef make_method_def(const achar* name, const_method_type m, tType aRetFlags) {
    const_method = m;
    is_const = true;
    return {
      name,
      param_traits<Ret>::type|aRetFlags,
      get_interface_uuid<Ret>(),
      get_type_name<Ret>(),
      sizeof...(Args),
      param_def_gen<Args...>::make_param_defs(nullptr),
      &call
    };
  }

  static tInt call(iUnknown* apThis, const Var* aArgs, tU32 aNumArgs, Var* apRet) {
    if (aNumArgs != sizeof...(Args))
      return eVMRet_InvalidArgCount;

    auto* _this = niUnsafeCast(Class*, apThis);
    args_tuple args;

    // Read arguments
    if constexpr (sizeof...(Args) > 0) {
      call_impl(aArgs, args, astl::make_index_sequence<sizeof...(Args)>());
    }

    // Call method (handle both const and non-const)
    if constexpr (astl::is_void_v<Ret>) {
      if (is_const) {
        astl::apply([_this](auto&&... args) {
          (_this->*const_method)(astl::forward<decltype(args)>(args)...);
        }, args);
      } else {
        astl::apply([_this](auto&&... args) {
          (_this->*method)(astl::forward<decltype(args)>(args)...);
        }, args);
      }
      if (apRet) apRet->SetNull();
    } else {
      auto result = is_const ?
          astl::apply([_this](auto&&... args) {
            return (_this->*const_method)(astl::forward<decltype(args)>(args)...);
          }, args) :
          astl::apply([_this](auto&&... args) {
            return (_this->*method)(astl::forward<decltype(args)>(args)...);
          }, args);
      if (apRet) {
        param_traits<Ret>::write(*apRet, &result);
      }
    }

    return eVMRet_OK;
  }

 private:
  static method_type method;
  static const_method_type const_method;
  static bool is_const;

  template<size_t... Is>
  static tInt call_impl(const Var* aArgs, args_tuple& args, astl::index_sequence<Is...>) {
    (param_traits<astl::remove_cvref_t<Args>>::read(aArgs[Is], &astl::get<Is>(args)), ...);
    return eVMRet_OK;
  }
};

template<typename Class, typename Ret, typename... Args>
typename method_wrapper<Class, Ret, Args...>::method_type
method_wrapper<Class, Ret, Args...>::method;

template<typename Class, typename Ret, typename... Args>
typename method_wrapper<Class, Ret, Args...>::const_method_type
method_wrapper<Class, Ret, Args...>::const_method;

template<typename Class, typename Ret, typename... Args>
bool method_wrapper<Class, Ret, Args...>::is_const = false;

template<typename Class>
class interface_def {
  astl::vector<sMethodDef> methods;
  astl::vector<const tUUID*> parents;

 private:
  // Non-const member function
  template<typename R, typename... Args>
  static sMethodDef make_method(const achar* name, R(Class::*m)(Args...), tType aRetFlags = 0) {
    return method_wrapper<Class, R, Args...>::make_method_def(name, m, aRetFlags);
  }

  // Const member function
  template<typename R, typename... Args>
  static sMethodDef make_method(const achar* name, R(Class::*m)(Args...) const, tType aRetFlags = 0) {
    return method_wrapper<Class, R, Args...>::make_method_def(name, m, aRetFlags);
  }

 public:
  template<auto Method>
  interface_def& method(const achar* name, tType aRetFlags = 0) {
    methods.push_back(make_method(name, Method, aRetFlags));
    return *this;
  }

  template<typename Parent>
  interface_def& parent() {
    parents.push_back(&Parent::_GetInterfaceUUID());
    return *this;
  }

  const sInterfaceDef& build() {
    // Create static storage for our vectors' contents
    static astl::vector<sMethodDef> stored_methods = astl::move(methods);
    static astl::vector<const sMethodDef*> method_ptrs;
    static astl::vector<const tUUID*> stored_parents = astl::move(parents);

    // Build method pointer array
    method_ptrs.clear();
    for(auto& method : stored_methods) {
      method_ptrs.push_back(&method);
    }

    static const sInterfaceDef def = {
      get_type_name<Class>(),
      get_interface_uuid<Class>(),
      (tU32)stored_parents.size(),
      stored_parents.data(),
      (tU32)method_ptrs.size(),
      method_ptrs.data(),
      nullptr  // No dispatch wrapper
    };

    return def;
  }
};

} // end of namespace vmbind
} // end of namespace ni

/**@}*/
/**@}*/
#endif // __VMBIND_H_A854CA3F_506A_4385_8D9B_BB4FE84E05EE__
