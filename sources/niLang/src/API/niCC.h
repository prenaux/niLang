#ifndef __NICC_H_2D298329_7F10_164A_B1C3_CF6D0695867A__
#define __NICC_H_2D298329_7F10_164A_B1C3_CF6D0695867A__

// SPDX-FileCopyrightText: (c) 2022 The niLang Authors
// SPDX-License-Identifier: MIT

#if defined niCCStrict
  // Strongly encouraged as it enforce safe usage of ni::Ptr & ni::QPtr.
  #define niNoUnsafePtr
#endif

// clang-format off

#include <niLang/STL/EASTL/EABase/config/eacompilertraits.h>

// This is because we want to allow [[nodiscard]] everywhere and I dont want
// to care about the warning when returning void.
EA_DISABLE_GCC_WARNING(-Wignored-attributes);
EA_DISABLE_GCC_WARNING(-Wattributes);
EA_DISABLE_CLANG_WARNING(-Wignored-attributes);

EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wc++11-narrowing);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wconversion-null);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wimplicit-fallthrough);
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
#if defined niCCStrict
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wfloat-conversion);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wimplicit-float-conversion);
EA_ENABLE_CLANG_WARNING_AS_ERROR(-Wimplicit-int-conversion);
#endif

//
// MSVC: switch case.
//
// Note: Unfortunately unusable as it doesnt respect [[maybe_unused]] on enum
// members
#if 0
EA_ENABLE_VC_WARNING_AS_ERROR(
  // C4061: The specified enumerator identifier has no associated
  // handler in a switch statement that has a default case.
  // 4061 -> this is taking it too far...

  // C4062: The enumerator identifier doesn't have a case handler
  // associated with it in a switch statement, and there's no default label
  // that can catch it.
  4062
)
#endif

// MSVC: unreference variables
EA_ENABLE_VC_WARNING_AS_ERROR(
  // C4101: 'abc': unreferenced local variable
  4101
  // C4189: 'xyz': local variable is initialized but not referenced
  4189
  // C4701: potentially uninitialized local variable 'pos' used
  4701
)

// clang-format on

#include <niLang/Types.h>
#include <niLang/STL/string.h>
#include <niLang/STL/string_view.h>
#include <niLang/STL/memory.h>
#include <niLang/STL/optional.h>
#include <niLang/STL/span.h>
#include <niLang/Utils/Nonnull.h>
#include <niLang/Math/MathRect.h>
#include <niLang/ILang.h> // For _HDecl & _HC
#include <niLang/STL/source_location.h>
#include <niLang/STL/run_once.h>
#include <niLang/STL/scope_guard.h>

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

#ifdef NULL
  #undef NULL
#endif
#define NULL nullptr

namespace astl {

template <bool B, typename T, typename F>
using conditional = eastl::conditional<B, T, F>;

template <bool B, typename T, typename F>
using conditional_t = typename conditional<B, T, F>::type;

using eastl::false_type;
using eastl::true_type;

using eastl::is_trivially_copy_constructible_v;

using eastl::is_base_of_v;
using eastl::is_null_pointer_v;
using eastl::is_pointer_v;
using eastl::is_same_v;
using eastl::is_void_v;
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

using eastl::remove_cvref_t;
using eastl::remove_pointer_t;

#define niDeclTypeBase(V) eastl::remove_cvref_t<decltype(V)>

// Default: remove pointers and references
template <typename T>
struct extract_value_type {
  using type = remove_cvref_t<remove_pointer_t<T>>;
};
// Specialization for types with value_type (containers, optionals, etc.)
template <typename T>
requires requires { typename T::value_type; }
struct extract_value_type<T> {
  using type = typename T::value_type;
};
// Helper alias
template <typename T>
using extract_value_type_t = typename extract_value_type<T>::type;

// For containers with value_type
static_assert(is_same_v<extract_value_type_t<astl::vector<int>>, int>);
static_assert(is_same_v<extract_value_type_t<astl::optional<float>>, float>);

// For pointers and raw types
static_assert(is_same_v<extract_value_type_t<int*>, int>);
static_assert(is_same_v<extract_value_type_t<const char*>, char>);
static_assert(is_same_v<extract_value_type_t<int>, int>);
static_assert(is_same_v<extract_value_type_t<int&>, int>);

} // namespace astl

namespace ni {

_HDecl(panic_NN_nullptr_raw);
_HDecl(panic_NN_nullptr_SmartPtr);
_HDecl(panic_NN_nullptr_QPtr);
_HDecl(panic_NN_nullptr_opt);
_HDecl(panic_nn_nullptr_raw);
_HDecl(panic_nn_nullptr_SmartPtr);
_HDecl(panic_nn_nullptr_QPtr);
_HDecl(panic_nn_nullptr_opt);
_HDecl(panic_unn_optional);
_HDecl(panic_snn_optional);

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

typedef sVec2<f32> vec2f;
typedef sVec3<f32> vec3f;
typedef sVec4<f32> vec4f;
typedef sQuat<f32> quatf;
typedef sRect<f32> rectf;
typedef sVec2<i32> vec2i;
typedef sVec3<i32> vec3i;
typedef sVec4<i32> vec4i;
typedef sRect<i32> recti;
typedef sMatrix<f32> matrixf;
typedef sMatrix<f64> matrix64;

typedef sVec2<u32> vec2u;
typedef sVec3<u32> vec3u;
typedef sVec4<u32> vec4u;
typedef sVec2<u32> sVec2u;
typedef sVec3<u32> sVec3u;
typedef sVec4<u32> sVec4u;

// <experimental>
template <typename T, typename... Args>
inline niConstExpr ni::QPtr<T> Create(Args&&... args)
{
  ni::QPtr<T> o = niNew T();
  niCheck(o->_Create(astl::forward<Args>(args)...), nullptr);
  return o;
}

#define niStaticAssertExprType(EXPR, TYPE) \
  static_assert(astl::is_same_v<decltype(EXPR), TYPE>)

#define niInline __forceinline

#define niLet const auto
#define niVar auto
#define niLetK static constexpr auto // "compile time constant (k)" let

#define niFn(TYPE) [[nodiscard]] TYPE __stdcall
#define niFnS(TYPE) [[nodiscard]] static TYPE __stdcall
#define niFnV(TYPE) [[nodiscard]] virtual TYPE __stdcall
#define niExportFn(TYPE) extern __ni_module_export TYPE __ni_export_call_decl
#define niInlineFn(TYPE) [[nodiscard]] niInline TYPE __stdcall

// Force lambda inlining, needed on MSVC...
#if defined niCLang || defined niGCC
  #define niInlineLambda __attribute__((always_inline))
#elif defined niMSVC
  #define niInlineLambda [[msvc::forceinline]]
#else
  #error "E/niCC: C++ Preprocessor: niInlineLambda not defined."
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
T* Decay(const T* ptr)
{
  niPanicAssert(ptr != nullptr);
  return const_cast<T*>(ptr);
}

// To use to call APIs that are not defined with _nn(). That is usually
// functions that take a mutable iUnknown* while the const version would do.
#define niDecay(PTR) ni::Decay((PTR).raw_ptr())

#define niDecayType(EXPR) astl::decay<decltype(EXPR)>::type

#define niDeferredInit(TYPE)          \
  TYPE                                \
  {                                   \
    TYPE::tUnsafeUncheckedInitializer \
    {                                 \
      nullptr                         \
    }                                 \
  }

template <typename T>
using nn = astl::non_null<T*>;

template <typename T>
using unn = astl::unique_non_null<T>;

template <typename T>
using snn = astl::shared_non_null<T>;

template <typename T>
using NN = ni::Nonnull<T>;

template <typename T>
concept IsNonNullType =
  requires {
    typename astl::remove_cvref_t<T>::element_type;
    typename astl::remove_cvref_t<T>::is_non_null_type;
  } &&
  astl::is_base_of_v<iUnknown, typename astl::remove_cvref_t<T>::element_type>;

template <typename T>
struct opt_raw_ptr : public astl::optional<T> {
  static_assert(astl::is_pointer_v<T>, "T must be a pointer type");
  using astl::optional<T>::optional;
  // do not allow nullptr, caller meant nullopt 99% of the time
  opt_raw_ptr(std::nullptr_t) = delete;
  // do not allow nullptr, caller meant nullopt 99% of the time
  opt_raw_ptr& operator=(std::nullptr_t) = delete;
};

using eastl::nullopt;
using nullopt_t = eastl::nullopt_t;

template <typename T>
using opt = opt_raw_ptr<T*>;

// primary template handles types that have no nested ::type member:
template <typename T, typename = void>
struct to_ain_t : astl::false_type {
  using type = astl::conditional_t<(sizeof(T) < 2 * sizeof(void*) &&
                                    astl::is_trivially_copy_constructible_v<T>),
                                   T const, T const&>;
};

// specialization recognizes types that do have a nested ::type member:
template <typename T>
struct to_ain_t<T, astl::void_t<typename T::in_type_t>> : astl::true_type {
  typedef typename T::in_type_t const type;
  static_assert(not astl::is_same_v<void* const, type> &&
                  not astl::is_same_v<void const, type>,
                "This type shouldn't be used as input parameter.");
};

template <typename T>
using ain = typename to_ain_t<T>::type;

template <typename T>
using ain_nn = typename to_ain_t<nn<T>>::type;

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
static_assert(astl::is_same_v<ain<QPtr<iUnknown>>, const QPtr<iUnknown>&>);
static_assert(sizeof(ain<nn<iUnknown>>) == sizeof(void*));

static_assert(astl::is_same_v<amove<sVec2f>, sVec2f&&>);
static_assert(astl::is_same_v<aout<sVec2f>, sVec2f&>);

static_assert(sizeof(ain<nn<iUnknown>>) == sizeof(const nn<iUnknown>));

// Not allowed as input types
// static_assert(astl::is_same_v<ain<NN<iUnknown>>, const nn<iUnknown>>);
// static_assert(astl::is_same_v<ain<Ptr<iUnknown>>, const Ptr<iUnknown>&>);
// static_assert(astl::is_same_v<ain<WeakPtr<iUnknown>>, const WeakPtr<iUnknown>&>);

namespace details {
template <class T, class U>
struct is_same_signedness
    : public astl::integral_constant<bool, astl::is_signed_v<T> ==
                                             astl::is_signed_v<U>> {};
} // namespace details

// unsafe_narrow_cast(): a searchable way to do narrowing casts of values
template <class T, class U>
constexpr T unsafe_narrow_cast(U&& u) noexcept
{
  return static_cast<T>(astl::forward<U>(u));
}

#undef niUnsafeNarrowCast
#define niUnsafeNarrowCast(T, EXP) ni::unsafe_narrow_cast<T>(EXP)

template <class T, class U>
constexpr T narrow_cast(U u) noexcept(false)
{
  constexpr const bool is_different_signedness =
    (astl::is_signed_v<T> != astl::is_signed_v<U>);
  const T t = unsafe_narrow_cast<T>(u);
  if (static_cast<U>(t) != u ||
      (is_different_signedness && ((t < T{}) != (u < U{}))))
  {
    niPanic(ni, invalid_cast,
            niFmt("narrows_cast changes the value: '%s' -> '%s'", u, t));
  }
  return t;
}

template <typename T, typename... Args>
concept IsConstructibleType =
  requires(Args&&... args) { new T(std::forward<Args>(args)...); };

template <typename F>
concept IsHFmtPanicMsgFn = requires(F f) {
  {
    f()
  } -> std::convertible_to<iHString*>;
};

template <class T, typename F>
requires IsHFmtPanicMsgFn<F>
inline auto as_non_null(T&& t, F&& afnHFmtPanicMsg,
                        ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT) noexcept
{
  if (!t) {
    ni_panic(_HSym(ni, panic), niHStr(afnHFmtPanicMsg()),
             ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  typedef astl::non_null<eastl::remove_cv_t<eastl::remove_reference_t<T>>> tNN;
  return tNN{ typename tNN::tUnsafeUncheckedInitializer(eastl::forward<T>(t)) };
}

template <typename T, typename... Args>
requires IsConstructibleType<T, Args...>
unn<T> make_unn(Args&&... args)
{
  return astl::as_non_null(astl::make_unique<T>(astl::forward<Args>(args)...));
}

template <typename T, typename... Args>
requires IsConstructibleType<T, Args...>
snn<T> make_snn(Args&&... args)
{
  return astl::as_non_null(astl::make_shared<T>(astl::forward<Args>(args)...));
}

template <typename T, typename F, typename... Args>
requires IsConstructibleType<T, Args...> && IsHFmtPanicMsgFn<F>
unn<T> make_unn(F&& afnHFmtPanicMsg, Args&&... args)
{
  return astl::as_non_null(astl::make_unique<T>(astl::forward<Args>(args)...),
                           afnHFmtPanicMsg);
}

template <typename T, typename F, typename... Args>
requires IsConstructibleType<T, Args...> && IsHFmtPanicMsgFn<F>
snn<T> make_snn(F&& afnHFmtPanicMsg, Args&&... args)
{
  return astl::as_non_null(astl::make_shared<T>(astl::forward<Args>(args)...),
                           afnHFmtPanicMsg);
}

template <typename T, typename... Args>
requires IsConstructibleType<T, Args...>
inline EA_CONSTEXPR ni::Nonnull<T> MakeNN(Args&&... args)
{
  return ni::Nonnull<T>(niNew T(eastl::forward<Args>(args)...));
}

typedef ni::cString tStr;
typedef const achar* tChars;
typedef achar* tMutChars;

template <typename... Args>
inline tStr Fmt(ain<tChars> aFmt, Args&&... args)
{
  tStr s;
  s.Format(aFmt, astl::forward<Args>(args)...);
  return s;
}

template <typename... Args>
inline NN<iHString> HFmt(ain<tChars> aFmt, Args&&... args)
{
  tStr s;
  s.Format(aFmt, astl::forward<Args>(args)...);
  return ni::CreateHStringFromView(s).non_null();
}

//##################################################################
// AsNN, as_nn, as_maybe_null
//##################################################################
template <typename TTo, typename TFrom>
concept IsConvertiblePointer =
  !std::is_same_v<TTo, TFrom> && std::derived_from<TTo, TFrom>;

template <typename T>
inline ni::Nonnull<T> AsNN(T* p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p) {
    ni_panic(_HC(panic_NN_nullptr_raw), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  return typename ni::Nonnull<T>::tUnsafeUncheckedInitializer{ p };
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline ni::Nonnull<TTo> AsNN(TFrom* p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p) {
    ni_panic(_HC(panic_NN_nullptr_raw), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  return typename ni::Nonnull<TTo>::tUnsafeUncheckedInitializer{
    static_cast<TTo*>(p)
  };
}

template <typename T>
inline ni::Nonnull<T> AsNN(const Ptr<T>& p,
                           ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p.has_value()) {
    ni_panic(_HC(panic_NN_nullptr_SmartPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  return typename ni::Nonnull<T>::tUnsafeUncheckedInitializer{ p.raw_ptr() };
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline ni::Nonnull<TTo> AsNN(const Ptr<TFrom>& p,
                             ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p.has_value()) {
    ni_panic(_HC(panic_NN_nullptr_SmartPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  return typename ni::Nonnull<TTo>::tUnsafeUncheckedInitializer{
    static_cast<TTo*>(p.raw_ptr())
  };
}

template <typename T>
inline ni::Nonnull<T> AsNN(const QPtr<T>& p,
                           ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p.has_value()) {
    ni_panic(_HC(panic_NN_nullptr_QPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  return typename ni::Nonnull<T>::tUnsafeUncheckedInitializer{ p.raw_ptr() };
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline ni::Nonnull<TTo> AsNN(const QPtr<TFrom>& p,
                             ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p.has_value()) {
    ni_panic(_HC(panic_NN_nullptr_QPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  return typename ni::Nonnull<TTo>::tUnsafeUncheckedInitializer{
    static_cast<TTo*>(p.raw_ptr())
  };
}

template <typename T>
inline niConstExpr ni::Nonnull<T> AsNN(const astl::non_null<T*> p)
{
  return ni::Nonnull<T>{ p };
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline niConstExpr ni::Nonnull<TTo> AsNN(const astl::non_null<TFrom*> p)
{
  return ni::Nonnull<TTo>{ static_cast<TTo*>(p.get()) };
}

template <typename T>
inline astl::non_null<T*> as_nn(T* p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p) {
    ni_panic(_HC(panic_nn_nullptr_raw), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  T* rp = p;
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{ astl::move(rp) };
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline astl::non_null<TTo*> as_nn(TFrom* p,
                                  ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p) {
    ni_panic(_HC(panic_nn_nullptr_raw), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  TTo* rp = static_cast<TTo*>(p);
  return typename astl::non_null<TTo*>::tUnsafeUncheckedInitializer{ astl::move(
    rp) };
}

template <typename T>
inline astl::non_null<T*> as_nn(const Ptr<T>& p,
                                ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p.has_value()) {
    ni_panic(_HC(panic_nn_nullptr_SmartPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  T* rp = p.raw_ptr();
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{ astl::move(rp) };
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline astl::non_null<TTo*> as_nn(const Ptr<TFrom>& p,
                                  ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p.has_value()) {
    ni_panic(_HC(panic_nn_nullptr_SmartPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  TTo* rp = static_cast<TTo*>(p.raw_ptr());
  return typename astl::non_null<TTo*>::tUnsafeUncheckedInitializer{ astl::move(
    rp) };
}

template <typename T>
inline astl::non_null<T*> as_nn(const QPtr<T>& p,
                                ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p.has_value()) {
    ni_panic(_HC(panic_nn_nullptr_QPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  T* rp = p.raw_ptr();
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{ astl::move(rp) };
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline astl::non_null<TTo*> as_nn(const QPtr<TFrom>& p,
                                  ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p.has_value()) {
    ni_panic(_HC(panic_nn_nullptr_QPtr), "", ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  TTo* rp = static_cast<TTo*>(p.raw_ptr());
  return typename astl::non_null<TTo*>::tUnsafeUncheckedInitializer{ astl::move(
    rp) };
}

template <typename T>
inline nn<T> as_nn(const opt<T>& p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p.has_value()) {
    ni_panic(_HC(panic_nn_nullptr_opt), AZEROSTR,
             ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  T* rp = p.value();
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{ astl::move(rp) };
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline nn<TTo> as_nn(const opt<TFrom>& p,
                     ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  if (!p.has_value()) {
    ni_panic(_HC(panic_nn_nullptr_opt), AZEROSTR,
             ASTL_SOURCE_LOCATION_ARG_CALL);
  }
  TTo* rp = static_cast<TTo*>(p.value());
  return typename astl::non_null<TTo*>::tUnsafeUncheckedInitializer{ astl::move(
    rp) };
}

template <typename T>
inline astl::non_null<T*> as_nn(const astl::shared_non_null<T>& v,
                                ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  T* rp = v.raw_ptr().get();
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{ astl::move(rp) };
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline astl::non_null<TTo*> as_nn(const astl::shared_non_null<TFrom>& v,
                                  ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  TTo* rp = static_cast<TTo*>(v.raw_ptr().get());
  return typename astl::non_null<TTo*>::tUnsafeUncheckedInitializer{ astl::move(
    rp) };
}

template <typename T>
inline astl::non_null<T*> as_nn(const astl::unique_non_null<T>& v,
                                ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  T* rp = v.raw_ptr().get();
  return
    typename astl::non_null<T*>::tUnsafeUncheckedInitializer{ astl::move(rp) };
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline astl::non_null<TTo*> as_nn(const astl::unique_non_null<TFrom>& v,
                                  ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  TTo* rp = static_cast<TTo*>(v.raw_ptr().get());
  return typename astl::non_null<TTo*>::tUnsafeUncheckedInitializer{ astl::move(
    rp) };
}

template <typename T>
inline T* as_maybe_null(T* p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  return p;
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline TTo* as_maybe_null(TFrom* p, ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  return static_cast<TTo*>(p);
}

template <typename T>
inline T* as_maybe_null(const Ptr<T>& p,
                        ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  return p.raw_ptr();
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline TTo* as_maybe_null(const Ptr<TFrom>& p,
                          ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  return static_cast<TTo*>(p.raw_ptr());
}

template <typename T>
inline T* as_maybe_null(const QPtr<T>& p,
                        ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  return p.raw_ptr();
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline TTo* as_maybe_null(const QPtr<TFrom>& p,
                          ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  return static_cast<TTo*>(p.raw_ptr());
}

template <typename T>
inline T* as_maybe_null(const opt<T>& p,
                        ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  return p.has_value() ? p.value() : nullptr;
}

template <typename TTo, typename TFrom>
requires IsConvertiblePointer<TTo, TFrom>
inline TTo* as_maybe_null(const opt<TFrom>& p,
                          ASTL_SOURCE_LOCATION_PARAM_WITH_DEFAULT)
{
  return p.has_value() ? static_cast<TTo*>(p.value()) : nullptr;
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
  using type = std::conditional_t<std::is_base_of_v<ni::iUnknown, T>,
                                  ni::Nonnull<T>, astl::non_null<T*>>;
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

static_assert(
  std::is_same<select_local_type_t<int*>, astl::non_null<int*>>::value);
static_assert(std::is_same<select_local_type_t<int>, int>::value);
static_assert(std::is_same<select_local_type_t<ni::iUnknown*>,
                           ni::Nonnull<ni::iUnknown>>::value);
static_assert(std::is_same<select_local_type_t<Ptr<ni::iUnknown>>,
                           ni::Nonnull<ni::iUnknown>>::value);
static_assert(std::is_same<select_local_type_t<QPtr<ni::iUnknown>>,
                           ni::Nonnull<ni::iUnknown>>::value);
static_assert(std::is_same<select_local_type_t<WeakPtr<ni::iUnknown>>,
                           ni::Nonnull<ni::iUnknown>>::value);

#define niCheckNNIfNull(V, EXPR)                                          \
  ni::select_local_type_t<decltype(EXPR)>{                                \
    ni::select_local_type_t<decltype(EXPR)>::tUnsafeUncheckedInitializer{ \
      EXPR }                                                              \
  };                                                                      \
  if ((V).raw_ptr() == nullptr)

#define niCheckNN_(V, EXPR, MSG, RET) \
  niCheckNNIfNull (V, EXPR) {         \
    niError(MSG);                     \
    return RET;                       \
  }

#define niCheckNN(V, EXPR, RET)               \
  niCheckNNIfNull (V, EXPR) {                 \
    niError("niCheckNN '" #EXPR "' failed."); \
    return RET;                               \
  }

#define niCheckNNSilent(V, EXPR, RET) \
  niCheckNNIfNull (V, EXPR) {         \
    return RET;                       \
  }

#define niLetNN(V, EXPR, RET)               \
  niLet V = niCheckNNIfNull (V, EXPR)       \
  {                                         \
    niError("niLetNN '" #EXPR "' failed."); \
    return RET;                             \
  }

#define niVarNN(V, EXPR, RET)               \
  niVar V = niCheckNNIfNull (V, EXPR)       \
  {                                         \
    niError("niVarNN '" #EXPR "' failed."); \
    return RET;                             \
  }

//##################################################################
// to_container & to_vector
//##################################################################

template <typename TOUT, typename TIN, typename TFUN>
void to_container(TOUT& out, const TIN& aIn, TFUN afnConvertItem)
{
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

template <typename TOUT, typename TIN, typename TFUN>
TOUT to_container(const TIN& aIn, TFUN afnConvertItem)
{
  TOUT out;
  to_container(out, aIn, afnConvertItem);
  return out;
}

template <typename TOUT, typename TIN>
void to_container(TOUT& out, const TIN& aIn)
{
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

template <typename TOUT, typename TIN>
TOUT to_container(const TIN& aIn)
{
  TOUT out;
  to_container(out, aIn);
  return out;
}

template <typename TOUT, typename TIN, typename TFUN>
astl::vector<TOUT> to_vector(const TIN& aIn, TFUN afnConvertItem)
{
  return to_container<astl::vector<TOUT>>(aIn, afnConvertItem);
}

template <typename TOUT, typename TIN>
astl::vector<TOUT> to_vector(const TIN& aIn)
{
  return to_container<astl::vector<TOUT>>(aIn);
}

} // namespace ni

//##################################################################
// Literals
//##################################################################
namespace ni {

inline constexpr ni::tI8 operator"" _i8(unsigned long long aVal)
{
  return static_cast<ni::tI8>(aVal);
}
inline constexpr ni::tI16 operator"" _i16(unsigned long long aVal)
{
  return static_cast<ni::tI16>(aVal);
}
inline constexpr ni::tI32 operator"" _i32(unsigned long long aVal)
{
  return static_cast<ni::tI32>(aVal);
}
inline constexpr ni::tI64 operator"" _i64(unsigned long long aVal)
{
  return static_cast<ni::tI64>(aVal);
}

inline constexpr ni::tU8 operator"" _u8(unsigned long long aVal)
{
  return static_cast<ni::tU8>(aVal);
}
inline constexpr ni::tU16 operator"" _u16(unsigned long long aVal)
{
  return static_cast<ni::tU16>(aVal);
}
inline constexpr ni::tU32 operator"" _u32(unsigned long long aVal)
{
  return static_cast<ni::tU32>(aVal);
}
inline constexpr ni::tU64 operator"" _u64(unsigned long long aVal)
{
  return static_cast<ni::tU64>(aVal);
}

inline constexpr ni::tF32 operator"" _f32(unsigned long long aVal)
{
  return static_cast<ni::tF32>(aVal);
}
inline constexpr ni::tF32 operator"" _f32(long double aVal)
{
  return static_cast<ni::tF32>(aVal);
}
inline constexpr ni::tF64 operator"" _f64(unsigned long long aVal)
{
  return static_cast<ni::tF64>(aVal);
}
inline constexpr ni::tF64 operator"" _f64(long double aVal)
{
  return static_cast<ni::tF64>(aVal);
}

inline ni::cString operator"" _str(const char* aStr, std::size_t aLen)
{
  return ni::cString(aStr, aLen);
}

inline ni::tHStringPtr operator"" _hstr(const char* aStr, std::size_t aLen)
{
  return ni::CreateHStringFromView(astl::string_view(aStr, aLen));
}

inline constexpr ni::tSize operator"" _sz(unsigned long long aVal)
{
  return static_cast<ni::tSize>(aVal);
}

inline constexpr astl::string_view operator"" _sv(const char* str,
                                                  size_t len) EA_NOEXCEPT
{
  return { str, len };
}
inline constexpr astl::ustring_view operator"" _sv(const ni::uchar* str,
                                                   size_t len) EA_NOEXCEPT
{
  return { str, len };
}

} // end namespace ni

//##################################################################
// HString
//##################################################################
namespace ni {

typedef NN<iHString> tHStringNN;

#undef niDefConstHString_
#define niDefConstHString_(VARNAME, STRING)                   \
  __forceinline ni::nn<ni::iHString> GetHString_##VARNAME()   \
  {                                                           \
    static ni::NN<ni::iHString> _hstr_##VARNAME = _H(STRING); \
    return _hstr_##VARNAME;                                   \
  }

#undef _H
#define _H(STR) ni::CreateHStringFromView(STR).non_null()

} // end namespace ni

//##################################################################
// ToString
//##################################################################
namespace ni {

template <typename T>
concept StringSetConvertible = requires(cString str, T value) {
  {
    str.Set(value)
  } -> std::same_as<void>;
};
template <StringSetConvertible T>
inline cString ToString(T aValue)
{
  cString str;
  str.Set(aValue);
  return str;
}

template <typename T>
concept StringAppendConvertible = requires(cString str, T value) {
  {
    str.append(value)
  };
};
template <StringAppendConvertible T>
inline cString ToString(T aValue)
{
  cString str;
  str.append(aValue);
  return str;
}

// To avoid unnecessary copies
inline const cString& ToString(const cString& aValue)
{
  return aValue;
}

} // end namespace ni
#endif // __NICC_H_2D298329_7F10_164A_B1C3_CF6D0695867A__
