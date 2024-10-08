#pragma once
#ifndef __STL_RANDOM_H_9A825C92_4685_EF11_BAD2_03EF948F804A__
#define __STL_RANDOM_H_9A825C92_4685_EF11_BAD2_03EF948F804A__

#include "bit.h"
#include "EASTL/type_traits.h"
#include "EASTL/numeric_limits.h"
#include "EASTL/iterator.h"
#include "EASTL/utility.h"
// for ni::ni_prng_get_seed_from_secure_source() which should be your default seed
#include "../Utils/Random.h"

namespace astl {

///////////////////////////////////////////////////////////////////////////
// log2
namespace detail {

template <class TUINT, TUINT Xp, ni::tSize Rp>
struct log2_impl;

template <ni::tU64 Xp, ni::tSize Rp>
struct log2_impl<ni::tU64, Xp, Rp> {
  static const ni::tSize value =
      Xp & ((ni::tU64)(1) << Rp) ? Rp : log2_impl<ni::tU64, Xp, Rp - 1>::value;
};

template <ni::tU64 Xp>
struct log2_impl<ni::tU64, Xp, 0> {
  static const ni::tSize value = 0;
};

template <ni::tSize Rp>
struct log2_impl<ni::tU64, 0, Rp> {
  static const ni::tSize value = Rp + 1;
};

#ifdef __SIZEOF_INT128__
template <unsigned __int128 Xp, ni::tSize Rp>
struct log2_impl<unsigned __int128, Xp, Rp> {
  static const ni::tSize value =
      (Xp >> 64) ? (64 + log2_impl<ni::tU64, (Xp >> 64), 63>::value)
      : log2_impl<ni::tU64, Xp, 63>::value;
};
#endif // __SIZEOF_INT128__

}

template <class TUINT, TUINT Xp>
struct ct_log2 {
  static const ni::tSize value = detail::log2_impl<
#ifdef __SIZEOF_INT128__
    eastl::conditional_t<sizeof(TUINT) <= sizeof(ni::tU64), ni::tU64, unsigned __int128>,
#else
    ni::tU64,
#endif // __SIZEOF_INT128__
    Xp,
    sizeof(TUINT) * CHAR_BIT - 1>::value;
};

///////////////////////////////////////////////////////////////////////////
/*!
 * \class independent_bits_engine
 * \brief A random number engine adaptor that produces random numbers with a specified number of bits.
 *
 * `independent_bits_engine` is a random number engine adaptor that produces random numbers with a specified number of bits (\c w bits), using an underlying random number engine (\c TENGINE). It wraps an existing random number engine and adjusts its output to produce values with an exact number of bits, suitable for applications that require random numbers of a specific bit width.
 *
 * The class works by extracting the required number of bits from the outputs of the underlying engine, potentially combining multiple outputs if necessary, to ensure uniformity and independence of the bits in the resulting numbers.
 *
 * \param TENGINE The underlying random number engine type.
 * \param TUINT The unsigned integer type for the generated random numbers.
 */
template <typename TENGINE, typename TUINT>
class independent_bits_engine {
 public:
  typedef TUINT result_type;

 private:
  typedef typename TENGINE::result_type engine_result_type;
  typedef typename eastl::make_unsigned<
    typename eastl::conditional<
      (sizeof(engine_result_type) < sizeof(result_type)),
      result_type,
      engine_result_type
      >::type
    >::type working_result_type;

  TENGINE& _engine;
  ni::tSize _w;
  ni::tSize _w0;
  ni::tSize _n;
  ni::tSize _n0;
  working_result_type _y0;
  working_result_type _y1;
  engine_result_type _mask0;
  engine_result_type _mask1;

  static constexpr working_result_type _kRp = (TENGINE::max()-TENGINE::min()) + working_result_type(1);

  static constexpr ni::tSize _km = ct_log2<working_result_type, _kRp>::value;
  static constexpr ni::tSize _kWDt = eastl::numeric_limits<working_result_type>::digits;
  static constexpr ni::tSize _kEDt = eastl::numeric_limits<engine_result_type>::digits;

 public:
  independent_bits_engine(TENGINE& e, ni::tSize w) : _engine(e), _w(w) {
    _n = _w / _km + (_w % _km != 0);
    _w0 = _w / _n;

    if constexpr (_kRp == 0)
      _y0 = _kRp;
    else if (_w0 < _kWDt)
      _y0 = (_kRp >> _w0) << _w0;
    else
      _y0 = 0;

    if (_kRp - _y0 > _y0 / _n) {
      ++_n;
      _w0 = _w / _n;
      if (_w0 < _kWDt)
        _y0 = (_kRp >> _w0) << _w0;
      else
        _y0 = 0;
    }

    _n0 = _n - _w % _n;

    if (_w0 < _kWDt - 1)
      _y1 = (_kRp >> (_w0 + 1)) << (_w0 + 1);
    else
      _y1 = 0;

    _mask0 = _w0 > 0 ? engine_result_type(~0) >> (_kEDt - _w0) : engine_result_type(0);
    _mask1 = _w0 < _kEDt - 1 ? engine_result_type(~0) >> (_kEDt - (_w0 + 1))
                             : engine_result_type(~0);
  }

  result_type operator()() {
    return eval(eastl::integral_constant<bool, _kRp != 0>());
  }

 private:
  result_type eval(eastl::false_type) {
    return static_cast<result_type>(_engine() & _mask0);
  }

  result_type eval(eastl::true_type) {
    const ni::tSize w_rt = eastl::numeric_limits<result_type>::digits;
    result_type sp = 0;

    for (ni::tSize k = 0; k < _n0; ++k) {
      engine_result_type u;
      do {
        u = _engine() - TENGINE::min();
      } while (u >= _y0);

      if (_w0 < w_rt)
        sp <<= _w0;
      else
        sp = 0;
      sp += u & _mask0;
    }

    for (ni::tSize k = _n0; k < _n; ++k) {
      engine_result_type u;
      do {
        u = _engine() - TENGINE::min();
      } while (u >= _y1);

      if (_w0 < w_rt - 1)
        sp <<= _w0 + 1;
      else
        sp = 0;
      sp += u & _mask1;
    }

    return sp;
  }
};

/*!
 * \class uniform_int_distribution
 * \brief Implements a uniform integer distribution using an underlying random number generator.
 *
 * This class generates random integers uniformly distributed over a specified range [a, b], inclusive.
 * It uses `independent_bits_engine` internally to produce the required random numbers.
 *
 * \param TINT The integer type of the generated random numbers.
 */
template<class TINT = ni::tI32>
class uniform_int_distribution {
  static_assert(eastl::is_integral<TINT>::value, "uniform_int_distribution: TINT must be integral.");

 public:
  typedef TINT result_type;

  //! \brief Defines the parameters (a and b) for the distribution.
  struct param_type {
    explicit param_type(TINT a = 0, TINT b = eastl::numeric_limits<TINT>::max())
        : mA(a), mB(b) {
      EASTL_ASSERT(a <= b);
    }

    result_type a() const { return mA; }
    result_type b() const { return mB; }

    bool operator==(const param_type& x) const { return (x.mA == mA) && (x.mB == mB); }
    bool operator!=(const param_type& x) const { return !(*this == x); }

   protected:
    TINT mA;
    TINT mB;
  };

  uniform_int_distribution(TINT a = 0, TINT b = eastl::numeric_limits<TINT>::max())
      : mParam(a, b) {}

  uniform_int_distribution(const param_type& params)
      : mParam(params) {}

  void reset() { /* nothing to do */ }

  template<class Generator>
  result_type operator()(Generator& g) {
    return operator()(g, mParam);
  }

  template<class Generator>
  result_type operator()(Generator& g, const param_type& params) {
    typedef typename eastl::make_unsigned<TINT>::type uresult_type;

    uresult_type a = static_cast<uresult_type>(params.a());
    uresult_type b = static_cast<uresult_type>(params.b());
    uresult_type range = b - a;

    if (range == 0) {
      return params.a();
    } else {
      ni::tSize bits = eastl::numeric_limits<uresult_type>::digits - astl::countl_zero(range);
      if ((range & (range + 1)) != 0) {
        ++bits;
      }
      independent_bits_engine<Generator, uresult_type> ib_engine(g, bits);
      uresult_type u;
      do {
        u = ib_engine();
      } while (u > range);
      return static_cast<result_type>(a + u);
    }
  }

  result_type a() const { return mParam.a(); }
  result_type b() const { return mParam.b(); }

  param_type param() const { return mParam; }
  void param(const param_type& params) { mParam = params; }

  result_type min() const { return mParam.a(); }
  result_type max() const { return mParam.b(); }

  bool operator==(const uniform_int_distribution& rhs) const {
    return mParam == rhs.mParam;
  }

  bool operator!=(const uniform_int_distribution& rhs) const {
    return !(*this == rhs);
  }

 protected:
  param_type mParam;
};

///////////////////////////////////////////////////////////////////////////
// MersenneTwister
template <typename TUINT, ni::tSize w, ni::tSize n, ni::tSize m, ni::tSize r,
          TUINT a, ni::tSize u, TUINT d, ni::tSize s,
          TUINT b, ni::tSize t,
          TUINT c, ni::tSize l, TUINT f>
struct sMersenneTwister {
  using result_type = TUINT;
  static constexpr ni::tSize state_size = n;
  static constexpr result_type default_seed = 5489u;

  explicit sMersenneTwister(result_type seed = default_seed) { this->seed(seed); }

  void seed(result_type value = default_seed) {
    _state[0] = value;
    for (ni::tSize i = 1; i < n; ++i) {
      _state[i] = f * (_state[i-1] ^ (_state[i-1] >> (w-2))) + i;
    }
    _index = n;
  }

  result_type next() {
    if (_index >= n) {
      twist();
    }
    result_type y = _state[_index++];
    y ^= ((y >> u) & d);
    y ^= ((y << s) & b);
    y ^= ((y << t) & c);
    y ^= (y >> l);
    return y;
  }

  void twist() {
    static constexpr result_type lower_mask = (static_cast<result_type>(1) << r) - 1;
    static constexpr result_type upper_mask = ~lower_mask;

    for (ni::tSize i = 0; i < n; ++i) {
      result_type x = (_state[i] & upper_mask) + (_state[(i+1) % n] & lower_mask);
      result_type xA = x >> 1;
      if (x & 0x1) {
        xA ^= a;
      }
      _state[i] = _state[(i + m) % n] ^ xA;
    }
    _index = 0;
  }

  result_type _state[n];
  ni::tSize _index;
};

using tMersenneTwisterU32 = sMersenneTwister<
  ni::tU32, 32, 624, 397, 31,
  0x9908b0df, 11, 0xffffffff, 7,
  0x9d2c5680, 15,
  0xefc60000, 18, 1812433253>;

using tMersenneTwisterU64 = sMersenneTwister<
  ni::tU64, 64, 312, 156, 31,
  0xb5026f5aa96619e9ULL, 29, 0x5555555555555555ULL, 17,
  0x71d67fffeda60000ULL, 37,
  0xfff7eee000000000ULL, 43, 6364136223846793005ULL>;

template <typename TTYPE>
struct sMTEngineTpl {
  static_assert(eastl::is_same_v<TTYPE,ni::tU32> || eastl::is_same_v<TTYPE,ni::tU64>,
                "sMTEngineTpl only accepts tU32 or tU64 as template parameter");

  using TMT = eastl::conditional_t<eastl::is_same_v<TTYPE,ni::tU32>, tMersenneTwisterU32, tMersenneTwisterU64>;
  TMT _mt;

  typedef typename TMT::result_type result_type;

  sMTEngineTpl() = delete;

  explicit sMTEngineTpl(ni::tU64 anSeed)
      : _mt(anSeed) {}

  sMTEngineTpl(const sMTEngineTpl& other) : _mt(other._mt) {}

  sMTEngineTpl& operator=(const sMTEngineTpl& other) {
    if (this != &other) {
      _mt = other._mt;
    }
    return *this;
  }

  void seed(ni::tU64 anSeed) {
    _mt.seed(anSeed);
  }

  result_type operator()() {
    return _mt.next();
  }

  void discard(ni::tSize aCount) {
    niLoop(i,aCount) {
      _mt.next();
    }
  }

  static constexpr result_type min() {
    return ni::TypeMin<result_type>();
  }

  static constexpr result_type max() {
    return ni::TypeMax<result_type>();
  }

  friend bool operator==(const sMTEngineTpl& lhs, const sMTEngineTpl& rhs) {
    return lhs._mt == rhs._mt;
  }

  friend bool operator!=(const sMTEngineTpl& lhs, const sMTEngineTpl& rhs) {
    return !(lhs == rhs);
  }
};

using mt19937_32 = sMTEngineTpl<ni::tU32>;
using mt19937_64 = sMTEngineTpl<ni::tU64>;

///////////////////////////////////////////////////////////////////////////
/// shuffle
///
/// Randomizes a sequence of values via a user-supplied
/// UniformRandomNumberGenerator.  The difference between this and the
/// original random_shuffle function is that this uses the more advanced and
/// flexible UniformRandomNumberGenerator interface as opposed to the more
/// limited RandomNumberGenerator interface of random_shuffle.
///
/// Effects: Shuffles the elements in the range [first, last) with uniform distribution.
///
/// Complexity: Exactly '(last - first) - 1' swaps.
///
/// Example usage:
///     struct Rand{ eastl_ni::tSize operator()(eastl_ni::tSize n) { return (eastl_ni::tSize)(rand() % n); } }; // Note: The C rand function is poor and slow.
///     Rand randInstance;
///     shuffle(pArrayBegin, pArrayEnd, randInstance);
///
// See the C++11 Standard, 26.5.1.3, Uniform random number generator requirements.
// Also http://en.cppreference.com/w/cpp/numeric/random/uniform_int_distribution
template <typename RandomAccessIterator, typename UniformRandomNumberGenerator>
void shuffle(RandomAccessIterator first, RandomAccessIterator last, UniformRandomNumberGenerator&& urng)
{
  if(first != last)
  {
    typedef typename eastl::iterator_traits<RandomAccessIterator>::difference_type difference_type;
    typedef typename eastl::make_unsigned<difference_type>::type                   unsigned_difference_type;
    typedef typename astl::uniform_int_distribution<unsigned_difference_type>      uniform_int_distribution;
    typedef typename uniform_int_distribution::param_type                          uniform_int_distribution_param_type;

    uniform_int_distribution uid;

    for(RandomAccessIterator i = first + 1; i != last; ++i)
      eastl::iter_swap(i, first + uid(urng, uniform_int_distribution_param_type(0, i - first)));
  }
}

}
#endif // __STL_RANDOM_H_9A825C92_4685_EF11_BAD2_03EF948F804A__
