#ifndef __NICC_H_2D298329_7F10_164A_B1C3_CF6D0695867A__
#define __NICC_H_2D298329_7F10_164A_B1C3_CF6D0695867A__

// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#if !defined niCCAllowUnsafePtr
  // Strongly encouraged as it enforce safe usage of ni::Ptr & ni::QPtr.
  #define niNoUnsafePtr
#endif

// Makes the default types and member functions mutable by default.
// #define niCCScriptMode

#include <niLang/Types.h>
#include <niLang/STL/string.h>
#include <niLang/STL/memory.h>
#include <niLang/STL/optional.h>
#include <niLang/STL/span.h>
#include <niLang/Utils/Nonnull.h>
#include <niLang/Math/MathRect.h>
#include <niLang/ILang.h>  // For _HDecl & _HC
#include <niLang/STL/source_location.h>
#include <niLang/STL/run_once.h>
#include <niLang/STL/scope_guard.h>

// clang-format off
// This is because we want to allow [[nodiscard]] everywhere and I dont want
// to care about the warning when returning void.
EA_DISABLE_GCC_WARNING(-Wignored-attributes);
EA_DISABLE_CLANG_WARNING(-Wignored-attributes);

#if !defined niCCNoStrictWarnings
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wc++11-narrowing);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wconversion-null);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wimplicit-fallthrough);
#if !defined niCCNoStrictImplicitConversionWarnings
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wfloat-conversion);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wimplicit-float-conversion);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wimplicit-int-conversion);
#endif
#if !defined niCCNoStrictOverrideWarnings
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Winconsistent-missing-override);
#endif
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wswitch);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wuninitialized);
// EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wunused-member-function)
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wunused-private-field);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wunused-result);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wunused-value);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wconditional-uninitialized);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wsometimes-uninitialized);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wuninitialized);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wreturn-type);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wunused-but-set-variable);
// clang-format on
#endif

#ifdef __cplusplus
  #if __cplusplus == 199711L
    #define niStdCppVersion 1998
  #elif __cplusplus == 201103L
    #define niStdCppVersion 2011
  #elif __cplusplus == 201402L
    #define niStdCppVersion 2014
  #elif __cplusplus == 201703L
    #define niStdCppVersion 2017
  #elif __cplusplus > 201703L
    #define niStdCppVersion 2020
  #else
    #error "Unknown C++ standard"
  #endif
#endif

#if niStdCppVersion < 2020
  #error "niCC.h requires C++20 or above."
#endif

namespace astl {

template <bool B, typename T, typename F>
using conditional = eastl::conditional<B, T, F>;

template <bool B, typename T, typename F>
using conditional_t = typename conditional<B, T, F>::type;

using eastl::false_type;
using eastl::true_type;

using eastl::is_trivially_copy_constructible_v;

using eastl::is_same_v;
using eastl::is_pointer_v;
using eastl::void_t;

using eastl::integral_constant;
using eastl::is_signed;
using eastl::is_signed_v;
using eastl::is_unsigned;
using eastl::is_unsigned_v;

using eastl::decay;

using eastl::enable_if_t;
using eastl::is_convertible;
using eastl::is_convertible_v;

}  // namespace astl

namespace ni {

_HDecl(panic_NN_nullptr_raw);
_HDecl(panic_NN_nullptr_SmartPtr);
_HDecl(panic_NN_nullptr_QPtr);
_HDecl(panic_NN_nullptr_opt);
_HDecl(panic_nn_nullptr_raw);
_HDecl(panic_nn_nullptr_SmartPtr);
_HDecl(panic_nn_nullptr_QPtr);
_HDecl(panic_nn_nullptr_opt);

using i8 = ni::tI8;
using i16 = ni::tI16;
using i32 = ni::tI32;
using i64 = ni::tI64;
using u8 = ni::tU8;
using u16 = ni::tU16;
using u32 = ni::tU32;
using u64 = ni::tU64;

typedef tF32 f32;
typedef tF64 f64;

template <typename T>
using vec2 = sVec2<T>;
template <typename T>
using vec3 = sVec3<T>;
template <typename T>
using vec4 = sVec4<T>;
template <typename T>
using quat = sQuat<T>;
template <typename T>
using matrix = sMatrix<T>;
template <typename T>
using rect = sRect<T>;

typedef vec2<f32> vec2f;
typedef vec3<f32> vec3f;
typedef vec4<f32> vec4f;
typedef quat<f32> quatf;
typedef rect<f32> rectf;
typedef vec2<i32> vec2i;
typedef vec3<i32> vec3i;
typedef vec4<i32> vec4i;
typedef rect<i32> recti;
typedef matrix<f32> matrixf;
typedef matrix<f64> matrix64;

using str = ni::cString;

// <experimental>
template <typename T, typename... Args>
inline niConstExpr ni::QPtr<T> Create(Args&&... args) {
  ni::QPtr<T> o = niNew T();
  niCheck(o->_Create(astl::forward<Args>(args)...), nullptr);
  return o;
}

#define niStaticAssertExprType(EXPR, TYPE) \
  static_assert(astl::is_same_v<decltype(EXPR), TYPE>)

#define niLet const auto
#define niLetMut auto
// "compile time constant (k)" let
#define niLetK static constexpr auto

#define niFn(TYPE) [[nodiscard]] TYPE __stdcall
#define niFnS(TYPE) [[nodiscard]] static TYPE __stdcall
#define niFnV(TYPE) [[nodiscard]] virtual TYPE __stdcall
#define niExportFn(TYPE) extern __ni_module_export TYPE __ni_export_call_decl

// Force lambda inlining, needed on MSVC...
#ifdef _MSC_VER
  #define niInlineLambda [[msvc::forceinline]]
#else
  #define niInlineLambda __attribute__((always_inline))
#endif

// Simple lambdas. XXX: These are not great but typing C++ lambdas is really
// tedious, maybe expanding the `fun` syntax in niCC would improve this.
#define niExpr(EXPR) ([]() niInlineLambda { return EXPR; })
#define niExpr1(EXPR) \
  ([]([[maybe_unused]] auto&& _0) niInlineLambda { return EXPR; })
#define niExpr2(EXPR)                                         \
  ([]([[maybe_unused]] auto&& _0, [[maybe_unused]] auto&& _1) \
     niInlineLambda { return EXPR; })
#define niRefExpr(EXPR) ([&]() niInlineLambda { return EXPR; })
#define niRefExpr1(EXPR) \
  ([&]([[maybe_unused]] auto&& _0) niInlineLambda { return EXPR; })
#define niRefExpr2(EXPR)                                       \
  ([&]([[maybe_unused]] auto&& _0, [[maybe_unused]] auto&& _1) \
     niInlineLambda { return EXPR; })
#define niFun(...) [__VA_ARGS__]() niInlineLambda
#define niFun1(...) [__VA_ARGS__]([[maybe_unused]] auto&& _0) niInlineLambda
#define niFun2(...)                                                     \
  [__VA_ARGS__]([[maybe_unused]] auto&& _0, [[maybe_unused]] auto&& _1) \
    niInlineLambda

template <typename T>
T* Decay(const T* ptr) {
  niPanicAssert(ptr != nullptr);
  return const_cast<T*>(ptr);
}

// To use to call APIs that are not defined with _nn(). That is usually
// functions that take a mutable iUnknown* while the const version would do.
#define niDecay(PTR) ni::Decay((PTR).raw_ptr())

#define niDecayType(EXPR) astl::decay<decltype(EXPR)>::type

#define niDeferredInit(TYPE)            \
  TYPE {                                \
    TYPE::tUnsafeUncheckedInitializer { \
      nullptr                           \
    }                                   \
  }

template <typename T>
using nn_mut = astl::non_null<T*>;

#ifdef niCCScriptMode
template <typename T>
using nn = nn_mut<T>;
#else
template <typename T>
using nn = astl::non_null<const T*>;
#endif

template <typename T>
using unn_mut = astl::unique_non_null<T>;

#ifdef niCCScriptMode
template <typename T>
using unn = unn_mut<T>;
#else
template <typename T>
using unn = astl::unique_non_null<const T>;
#endif

template <typename T>
using snn_mut = astl::shared_non_null<T>;

#ifdef niCCScriptMode
template <typename T>
using snn = snn_mut<T>;
#else
template <typename T>
using snn = astl::shared_non_null<const T>;
#endif

template <typename T>
using NN_mut = ni::Nonnull<T>;

#ifdef niCCScriptMode
template <typename T>
using NN = NN_mut<T>;
#else
template <typename T>
using NN = ni::Nonnull<const T>;
#endif

template <typename T>
struct opt_raw_ptr : public astl::optional<T> {
  static_assert(astl::is_pointer_v<T>, "T must be a pointer type");
  using astl::optional<T>::optional;
  // do not allow nullptr, caller meant nullopt 99% of the time
  opt_raw_ptr(std::nullptr_t) = delete;
  // do not allow nullptr, caller meant nullopt 99% of the time
  opt_raw_ptr& operator=(std::nullptr_t) = delete;
};

template <typename T>
using opt_mut = opt_raw_ptr<T*>;
#ifdef niCCScriptMode
template <typename T>
using opt = opt_raw_ptr<T*>;
#else
template <typename T>
using opt = opt_raw_ptr<const T*>;
#endif

template <typename T>
using Opt_mut = ni::QPtr<T>;

#ifdef niCCScriptMode
template <typename T>
using Opt = Opt_mut<T>;
#else
template <typename T>
using Opt = ni::QPtr<const T>;
#endif

// primary template handles types that have no nested ::type member:
template <typename T, typename = void>
struct to_ain_t : astl::false_type {
  using type = astl::conditional_t<
    (sizeof(T) < 2 * sizeof(void*) &&
     astl::is_trivially_copy_constructible_v<T>),
    T const,
    T const&>;
};

// specialization recognizes types that do have a nested ::type member:
template <typename T>
struct to_ain_t<T, astl::void_t<typename T::in_type_t>> : astl::true_type {
  typedef typename T::in_type_t const type;
  static_assert(
    not astl::is_same_v<void* const, type> &&
      not astl::is_same_v<void const, type>,
    "This type shouldn't be used as input parameter.");
};

template <typename T>
using ain = typename to_ain_t<T>::type;

template <typename T>
struct to_amove_t {
  using type = T&&;
};

template <typename T>
using amove = typename to_amove_t<T>::type;

template <typename T>
struct to_aout_t {
  using type = T&;
};

template <typename T>
using aout = typename to_aout_t<T>::type;

template <typename T>
struct to_ainout_t {
  using type = T&;
};

template <typename T>
using ainout = typename to_ainout_t<T>::type;

template <typename T>
struct to_acopy_t {
  using type = T;
};

template <typename T>
using acopy = typename to_acopy_t<T>::type;

#ifdef ni32
static_assert(astl::is_same_v<ain<sVec2f>, const sVec2f&>);
static_assert(astl::is_same_v<ain<sVec3f>, const sVec3f&>);
#else
static_assert(astl::is_same_v<ain<sVec2f>, const sVec2f>);
static_assert(astl::is_same_v<ain<sVec3f>, const sVec3f>);
#endif
static_assert(astl::is_same_v<ain<sVec4f>, const sVec4f&>);
static_assert(astl::is_same_v<ain<sMatrixf>, const sMatrixf&>);
#if !defined niMSVC // Types are the same (as checked while dumping them by other means) but still MSVC fails the static_assert
static_assert(astl::is_same_v<ain<nn<iUnknown>>, const nn<iUnknown>>);
static_assert(astl::is_same_v<ain<nn_mut<iUnknown>>, const nn_mut<iUnknown>>);
#endif
static_assert(astl::is_same_v<ain<QPtr<iUnknown>>, const QPtr<iUnknown>&>);
static_assert(sizeof(ain<nn<iUnknown>>) == sizeof(void*));
static_assert(sizeof(ain<nn_mut<iUnknown>>) == sizeof(void*));

static_assert(astl::is_same_v<amove<sVec2f>, sVec2f&&>);
static_assert(astl::is_same_v<aout<sVec2f>, sVec2f&>);

// Not allowed as input types
// static_assert(astl::is_same_v<ain<NN<iUnknown>>, const nn<iUnknown>>);
// static_assert(astl::is_same_v<ain<NN_mut<iUnknown>>, const nn_mut<iUnknown>>);
// static_assert(astl::is_same_v<ain<Ptr<iUnknown>>, const Ptr<iUnknown>&>);
// static_assert(astl::is_same_v<ain<WeakPtr<iUnknown>>, const WeakPtr<iUnknown>&>);

// ain_nn_mut doesnt need ain since we know its a trivial pointer sized constructor
template <typename T>
using ain_nn_mut = ain<astl::non_null<T*>>;

#ifdef niCCScriptMode
template <typename T>
using ain_nn = ain_nn_mut<T>;
#else
template <typename T>
using ain_nn = ain<astl::non_null<const T*>>;
#endif

namespace details {
template <class T, class U>
struct is_same_signedness
    : public astl::
        integral_constant<bool, astl::is_signed_v<T> == astl::is_signed_v<U>> {
};
}  // namespace details

// unsafe_narrow_cast(): a searchable way to do narrowing casts of values
template <class T, class U>
constexpr T unsafe_narrow_cast(U&& u) noexcept {
  return static_cast<T>(astl::forward<U>(u));
}

template <class T, class U>
constexpr T narrow_cast(U u) noexcept(false) {
  constexpr const bool is_different_signedness =
    (astl::is_signed_v<T> != astl::is_signed_v<U>);
  const T t = unsafe_narrow_cast<T>(u);
  if (
    static_cast<U>(t) != u ||
    (is_different_signedness && ((t < T{}) != (u < U{})))) {
    niThrowPanic(
      ni,
      invalid_cast,
      niFmt("narrows_cast changes the value: '%s' -> '%s'", u, t));
  }
  return t;
}

template <typename T, typename... Args>
unn_mut<T> make_unn(Args&&... args) {
  return unn_mut<T>(astl::make_unique<T>(astl::forward<Args>(args)...));
}

template <typename T, typename... Args>
snn_mut<T> make_snn(Args&&... args) {
  return snn_mut<T>(astl::make_shared<T>(astl::forward<Args>(args)...));
}

typedef ni::cString tStr;
typedef const achar* tChars;
typedef achar* tMutChars;

template <typename... Args>
inline tStr Fmt(ain<tChars> aFmt, Args&&... args) {
  tStr s;
  s.Format(aFmt, astl::forward<Args>(args)...);
  return s;
}

// XXX: iHString is in fact always immutable, but a most legacy APIs just use
// iHString* instead of const iHString* so we use NN_mut.
template <typename... Args>
inline NN_mut<iHString> HFmt(ain<tChars> aFmt, Args&&... args) {
  tStr s;
  s.Format(aFmt, astl::forward<Args>(args)...);
  return ni::GetLang()->CreateHString(s.c_str()).non_null();
}

//##################################################################
// as_nn, as_NN, as_opt, as_Opt
//##################################################################

template <typename T>
inline ni::Nonnull<T> as_NN(T* p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  if (!p) {
    ni_throw_panic(
      _HC(panic_NN_nullptr_raw), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  return typename ni::Nonnull<T>::tUnsafeUncheckedInitializer{p};
}

template <typename T>
inline ni::Nonnull<T> as_NN(
  const Ptr<T>& p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  if (!p.has_value()) {
    ni_throw_panic(
      _HC(panic_NN_nullptr_SmartPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  return typename ni::Nonnull<T>::tUnsafeUncheckedInitializer{p.raw_ptr()};
}

template <typename T>
inline ni::Nonnull<T> as_NN(
  const QPtr<T>& p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  if (!p.has_value()) {
    ni_throw_panic(
      _HC(panic_NN_nullptr_QPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  return typename ni::Nonnull<T>::tUnsafeUncheckedInitializer{p.raw_ptr()};
}

template <typename T>
inline niConstExpr ni::Nonnull<T> as_NN(const astl::non_null<T*> p) {
  return ni::Nonnull<T>{p};
}

template <typename T>
inline astl::non_null<T*> as_nn(T* p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  if (!p) {
    ni_throw_panic(
      _HC(panic_nn_nullptr_raw), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  T* rp = p;
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{astl::move(rp)};
}

template <typename T>
inline astl::non_null<T*> as_nn(
  const Ptr<T>& p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  if (!p.has_value()) {
    ni_throw_panic(
      _HC(panic_nn_nullptr_SmartPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  T* rp = p;
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{astl::move(rp)};
}

template <typename T>
inline astl::non_null<T*> as_nn(
  const QPtr<T>& p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  if (!p.has_value()) {
    ni_throw_panic(
      _HC(panic_nn_nullptr_QPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  T* rp = p;
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{astl::move(rp)};
}

template <typename T>
inline nn_mut<T> as_nn(
  const opt_mut<T>& p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  if (!p.has_value()) {
    ni_throw_panic(
      _HC(panic_nn_nullptr_opt), AZEROSTR, ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  T* rp = p.value();
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{astl::move(rp)};
}

template <typename T>
inline astl::non_null<T*> as_nn(
  const astl::shared_non_null<T>& v, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  T* rp = v.raw_ptr().get();
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{astl::move(rp)};
}

template <typename T>
inline astl::non_null<T*> as_nn(
  const astl::unique_non_null<T>& v, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  T* rp = v.raw_ptr().get();
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{astl::move(rp)};
}

template <typename T>
inline ni::opt<T> as_opt(T* p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  return typename ni::opt<T>{p};
}

template <typename T>
inline ni::Opt<T> as_Opt(T* p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  return typename ni::Opt<T>{p};
}

template <typename I, typename T>
inline ni::Opt<I> as_Opt(T* p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  return typename ni::Opt<I>{p};
}

template <typename T>
inline ni::Opt<T> as_Opt(const Ptr<T>& p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  return typename ni::Opt<T>{p.raw_ptr};
}

template <typename I, typename T>
inline ni::Opt<I> as_Opt(const Ptr<T>& p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) {
  return typename ni::Opt<I>{p.raw_ptr};
}

//##################################################################
// niCheckNN
//##################################################################

template <typename T>
struct select_local_type {
  using type = T;
};

template <typename T>
struct select_local_type<T*> {
  using type = std::conditional_t<
    std::is_base_of_v<ni::iUnknown, T>,
    ni::Nonnull<T>,
    astl::non_null<T*>>;
};

template <typename T>
struct select_local_type<Ptr<T>> {
  using type = Nonnull<T>;
};

template <typename T>
struct select_local_type<QPtr<T>> {
  using type = Nonnull<T>;
};

template <typename T>
struct select_local_type<WeakPtr<T>> {
  using type = Nonnull<T>;
};

template <typename T>
using select_local_type_t = typename select_local_type<T>::type;

static_assert(std::is_same<select_local_type_t<int*>, astl::non_null<int*>>::value);
static_assert(std::is_same<select_local_type_t<int>, int>::value);
static_assert(
  std::is_same<select_local_type_t<ni::iUnknown*>, ni::Nonnull<ni::iUnknown>>::value);
static_assert(std::is_same<
              select_local_type_t<Ptr<ni::iUnknown>>,
              ni::Nonnull<ni::iUnknown>>::value);
static_assert(std::is_same<
              select_local_type_t<QPtr<ni::iUnknown>>,
              ni::Nonnull<ni::iUnknown>>::value);
static_assert(std::is_same<
              select_local_type_t<WeakPtr<ni::iUnknown>>,
              ni::Nonnull<ni::iUnknown>>::value);

#define niCheckNNIfNull(V, EXPR)                                           \
  ni::select_local_type_t<decltype(EXPR)>{                                       \
    ni::select_local_type_t<decltype(EXPR)>::tUnsafeUncheckedInitializer{EXPR}}; \
  if ((V).raw_ptr() == nullptr)

#define niCheckNN_(V, EXPR, MSG, RET)           \
  niCheckNNIfNull(V, EXPR) {                    \
    niError(MSG);                               \
    return RET;                                 \
  }

#define niCheckNN(V, EXPR, RET)               \
  niCheckNNIfNull(V, EXPR) {                  \
    niError("niCheckNN '" #EXPR "' failed."); \
    return RET;                               \
  }

//##################################################################
// to_container & to_vector
//##################################################################

template<typename TOUT, typename TIN, typename TFUN>
void to_container(TOUT& out, const TIN& aIn, TFUN afnConvertItem) {
  if constexpr (requires { aIn.size(); }) {
    out.reserve(aIn.size());
  }
#if 0
  else {
    aIn.__error_no_size_function__();
  }
#endif
  for (niLet& item : aIn) {
    out.push_back(afnConvertItem(item));
  }
}

template<typename TOUT, typename TIN, typename TFUN>
TOUT to_container(const TIN& aIn, TFUN afnConvertItem) {
  TOUT out;
  to_container(out, aIn, afnConvertItem);
  return out;
}

template<typename TOUT, typename TIN>
void to_container(TOUT& out, const TIN& aIn) {
  typedef typename TOUT::value_type tOutValue;
  if constexpr (requires { aIn.size(); }) {
    out.reserve(aIn.size());
  }
#if 0
  else {
    aIn.__error_no_size_function__();
  }
#endif
  for (niLet& item : aIn) {
    if constexpr (requires { tOutValue(item); }) {
      out.push_back(tOutValue(item));
    }
    else if constexpr (requires { tOutValue(item.c_str()); }) {
      out.push_back(tOutValue(item.c_str()));
    }
    else {
      item.__not_compatible_with_output_element_type__();
    }
  }
}

template<typename TOUT, typename TIN>
TOUT to_container(const TIN& aIn) {
  TOUT out;
  to_container(out, aIn);
  return out;
}

template<typename TOUT, typename TIN, typename TFUN>
astl::vector<TOUT> to_vector(const TIN& aIn, TFUN afnConvertItem) {
  return to_container<astl::vector<TOUT>>(aIn,afnConvertItem);
}

template<typename TOUT, typename TIN>
astl::vector<TOUT> to_vector(const TIN& aIn) {
  return to_container<astl::vector<TOUT>>(aIn);
}

}  // namespace ni

//##################################################################
// Literals
//##################################################################
namespace ni {

inline constexpr ni::tI8 operator"" _i8(unsigned long long aVal) { return static_cast<ni::tI8>(aVal); }
inline constexpr ni::tI16 operator"" _i16(unsigned long long aVal) { return static_cast<ni::tI16>(aVal); }
inline constexpr ni::tI32 operator"" _i32(unsigned long long aVal) { return static_cast<ni::tI32>(aVal); }
inline constexpr ni::tI64 operator"" _i64(unsigned long long aVal) { return static_cast<ni::tI64>(aVal); }

inline constexpr ni::tU8 operator"" _u8(unsigned long long aVal) { return static_cast<ni::tU8>(aVal); }
inline constexpr ni::tU16 operator"" _u16(unsigned long long aVal) { return static_cast<ni::tU16>(aVal); }
inline constexpr ni::tU32 operator"" _u32(unsigned long long aVal) { return static_cast<ni::tU32>(aVal); }
inline constexpr ni::tU64 operator"" _u64(unsigned long long aVal) { return static_cast<ni::tU64>(aVal); }

inline constexpr ni::tF32 operator"" _f32(unsigned long long aVal) { return static_cast<ni::tF32>(aVal); }
inline constexpr ni::tF32 operator"" _f32(long double aVal) { return static_cast<ni::tF32>(aVal); }
inline constexpr ni::tF64 operator"" _f64(unsigned long long aVal) { return static_cast<ni::tF64>(aVal); }
inline constexpr ni::tF64 operator"" _f64(long double aVal) { return static_cast<ni::tF64>(aVal); }

inline ni::cString operator"" _str(const char* aStr, std::size_t aLen) {
    return ni::cString(aStr,aLen);
}

inline ni::tHStringPtr operator"" _hstr(const char* aStr, std::size_t aLen) {
    return ni::GetLang()->CreateHString(ni::cString(aStr,aLen));
}

inline constexpr ni::tSize operator"" _sz(unsigned long long aVal) { return static_cast<ni::tSize>(aVal); }

} // end namespace ni
#endif  // __NICC_H_2D298329_7F10_164A_B1C3_CF6D0695867A__
