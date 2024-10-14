#include "stdafx.h"
#include <niLang/STL/bit.h>
#include <niLang/STL/random.h>

using namespace ni;
using namespace astl;

struct ASTL_random_ct_log2 {};
struct ASTL_random_independent_bits_engine {};
struct ASTL_random_uniform_int_distribution {};
struct ASTL_random_engine {};
struct ASTL_random_shuffle {};

///////////////////////////////////////////////////////////////////////////
//
// ASTL_random_ct_log2
//
///////////////////////////////////////////////////////////////////////////
TEST_FIXTURE(ASTL_random_ct_log2,basic) {
  // 8-bit tests
  CHECK_EQUAL(0, (astl::ct_log2<ni::tU8, 1>::value));
  CHECK_EQUAL(1, (astl::ct_log2<ni::tU8, 2>::value));
  CHECK_EQUAL(2, (astl::ct_log2<ni::tU8, 4>::value));
  CHECK_EQUAL(3, (astl::ct_log2<ni::tU8, 8>::value));
  CHECK_EQUAL(7, (astl::ct_log2<ni::tU8, 255>::value));

  // 16-bit tests
  CHECK_EQUAL(8, (astl::ct_log2<ni::tU16, 256>::value));
  CHECK_EQUAL(15, (astl::ct_log2<ni::tU16, 0xFFFF>::value));

  // 32-bit tests
  CHECK_EQUAL(16, (astl::ct_log2<ni::tU32, 0x10000>::value));
  CHECK_EQUAL(31, (astl::ct_log2<ni::tU32, 0xFFFFFFFF>::value));

  // 64-bit tests
  CHECK_EQUAL(32, (astl::ct_log2<ni::tU64, 0x100000000ULL>::value));
  CHECK_EQUAL(63, (astl::ct_log2<ni::tU64, 0xFFFFFFFFFFFFFFFFULL>::value));
}

TEST_FIXTURE(ASTL_random_ct_log2,edgecases) {
  // Edge cases
#if 0
  // fails at compile-time
  CHECK_EQUAL(0, (astl::ct_log2<ni::tU8, 0>::value));
#endif
  CHECK_EQUAL(0, (astl::ct_log2<ni::tU64, 1>::value));
  CHECK_EQUAL(63, (astl::ct_log2<ni::tU64, 0x8000000000000000ULL>::value));
}

TEST_FIXTURE(ASTL_random_ct_log2,npot) {
  // Non-power-of-two values
  CHECK_EQUAL(1, (astl::ct_log2<ni::tU8, 3>::value));
  CHECK_EQUAL(2, (astl::ct_log2<ni::tU8, 7>::value));
  CHECK_EQUAL(4, (astl::ct_log2<ni::tU16, 31>::value));
  CHECK_EQUAL(5, (astl::ct_log2<ni::tU32, 63>::value));
  CHECK_EQUAL(6, (astl::ct_log2<ni::tU64, 127>::value));
}

///////////////////////////////////////////////////////////////////////////
//
// ASTL_random_independent_bits_engine
//
///////////////////////////////////////////////////////////////////////////
TEST_FIXTURE(ASTL_random_independent_bits_engine,basic_functionality) {
  mt19937_32 engine(42);  // Fixed seed for reproducibility
  size_t w = 16;  // Number of bits

  independent_bits_engine<mt19937_32, ni::tU32> ind_engine(engine, w);

  for (int i = 0; i < 1000; ++i) {
    ni::tU32 value = ind_engine();
    ni::tU32 max_value = (1u << w) - 1;
    // Check that the value is within the correct range
    CHECK(value <= max_value);
  }
}

TEST_FIXTURE(ASTL_random_independent_bits_engine,with_min_not_zero_engine) {
  // Custom engine with min != 0
  struct custom_engine {
    typedef ni::tU32 result_type;
    static constexpr ni::tU32 min() { return 100; }
    static constexpr ni::tU32 max() { return 200; }
    ni::tU32 operator()() { return _dist(_engine); }

    mt19937_32 _engine = mt19937_32(42);
    uniform_int_distribution<ni::tU32> _dist{min(), max()};
  };

  custom_engine engine;
  size_t w = 8;
  independent_bits_engine<custom_engine, ni::tU32> ind_engine(engine, w);

  for (int i = 0; i < 1000; ++i) {
    ni::tU32 value = ind_engine();
    ni::tU32 max_value = (1u << w) - 1;
    CHECK(value <= max_value);
  }
}

TEST_FIXTURE(ASTL_random_independent_bits_engine,with_32bit_engine) {
  mt19937_32 engine(42);  // 32-bit Mersenne Twister engine

  for (size_t w = 1; w <= 32; ++w) {
    independent_bits_engine<mt19937_32, ni::tU32> ind_engine(engine, w);

    for (int i = 0; i < 10; ++i) {
      ni::tU32 value = ind_engine();
      ni::tU32 max_value = (w == 32) ? 0xFFFFFFFFu : (ni::tU32)((1ull << w) - 1);
      // Check that the value is within the correct range
      CHECK(value <= max_value);
      // Check that bits beyond w are not set
      ni::tU32 mask = (w == 32) ? 0xFFFFFFFFu : (ni::tU32)((1ull << w) - 1);
      CHECK_EQUAL(value, value & mask);
      // niDebugFmt(("... w: %d, i: %d, value: %d", w, i, value));
    }
  }
}

TEST_FIXTURE(ASTL_random_independent_bits_engine,with_64bit_engine) {
  mt19937_64 engine(42);  // 64-bit Mersenne Twister engine

  for (size_t w = 1; w <= 64; ++w) {
    independent_bits_engine<mt19937_64, ni::tU64> ind_engine(engine, w);

    for (int i = 0; i < 10; ++i) {
      ni::tU64 value = ind_engine();
      ni::tU64 max_value = (w == 64) ? 0xFFFFFFFFu : (ni::tU64)((1ull << w) - 1);
      // Check that the value is within the correct range
      CHECK(value <= max_value);
      // Check that bits beyond w are not set
      ni::tU64 mask = (w == 64) ? 0xFFFFFFFFu : (ni::tU64)((1ull << w) - 1);
      CHECK_EQUAL(value, value & mask);
      // niDebugFmt(("... w: %d, i: %d, value: %d", w, i, value));
    }
  }
}

TEST_FIXTURE(ASTL_random_independent_bits_engine,bit_width_correctness) {
  mt19937_32 engine(42);  // Fixed seed for reproducibility
  size_t w = 16;  // Number of bits

  independent_bits_engine<mt19937_32, ni::tU32> ind_engine(engine, w);
  const ni::tU32 max_value = (1u << w) - 1;
  const int num_samples = 1000;
  bool out_of_range = false;

  for (int i = 0; i < num_samples; ++i) {
    ni::tU32 value = ind_engine();
    if (value > max_value) {
      out_of_range = true;
      break;
    }
  }

  CHECK(!out_of_range);
}

TEST_FIXTURE(ASTL_random_independent_bits_engine,bit_masking) {
  mt19937_32 engine(42);  // Fixed seed for reproducibility
  size_t w = 16;  // Number of bits

  independent_bits_engine<mt19937_32, ni::tU32> ind_engine(engine, w);
  const ni::tU32 mask = (1u << w) - 1;
  const int num_samples = 1000;
  bool bits_outside_mask_set = false;

  for (int i = 0; i < num_samples; ++i) {
    ni::tU32 value = ind_engine();
    if ((value & ~mask) != 0) {
      bits_outside_mask_set = true;
      break;
    }
  }

  CHECK(!bits_outside_mask_set);
}

TEST_FIXTURE(ASTL_random_independent_bits_engine,reproducibility) {
  mt19937_32 engine1(42);  // Fixed seed
  mt19937_32 engine2(42);  // Same seed
  size_t w = 16;

  independent_bits_engine<mt19937_32, ni::tU32> ind_engine1(engine1, w);
  independent_bits_engine<mt19937_32, ni::tU32> ind_engine2(engine2, w);

  const int num_samples = 1000;
  bool mismatch_found = false;

  for (int i = 0; i < num_samples; ++i) {
    ni::tU32 value1 = ind_engine1();
    ni::tU32 value2 = ind_engine2();
    if (value1 != value2) {
      mismatch_found = true;
      break;
    }
  }

  CHECK(!mismatch_found);
}

TEST_FIXTURE(ASTL_random_independent_bits_engine,different_bit_widths) {
  mt19937_32 engine(42);  // Fixed seed for reproducibility
  const int num_samples = 1000;
  bool error_found = false;

  for (size_t w = 1; w <= 32; ++w) {
    independent_bits_engine<mt19937_32, ni::tU32> ind_engine(engine, w);
    const ni::tU32 max_value = (w == 32) ? 0xFFFFFFFFu : (1u << w) - 1;
    const ni::tU32 mask = max_value;

    for (int i = 0; i < num_samples; ++i) {
      ni::tU32 value = ind_engine();
      if (value > max_value || (value & ~mask) != 0) {
        CHECK_LE(value, max_value);
        CHECK_EQUAL(0, (value & ~mask));
        error_found = true;
        break;
      }
    }
    if (error_found) {
      niError(niFmt("Error found at bit width %d.", w));
      break;
    }
  }

  CHECK(!error_found);
}

TEST_FIXTURE(ASTL_random_independent_bits_engine,with_different_engines) {
  // Test with mt19937_32
  {
    mt19937_32 engine(42);
    size_t w = 16;
    independent_bits_engine<mt19937_32, ni::tU32> ind_engine(engine, w);
    const ni::tU32 max_value = (1u << w) - 1;
    bool out_of_range = false;

    for (int i = 0; i < 1000; ++i) {
      ni::tU32 value = ind_engine();
      if (value > max_value) {
        out_of_range = true;
        break;
      }
    }
    CHECK(!out_of_range);
  }

  // Test with mt19937_64
  {
    mt19937_64 engine(42);
    size_t w = 64;
    independent_bits_engine<mt19937_64, ni::tU64> ind_engine(engine, w);
    bool out_of_range = false;

    for (int i = 0; i < 1000; ++i) {
      ni::tU64 value = ind_engine();
      // No need to check max_value since ni::tU64 can hold the maximum value
      if (value < 0) {
        out_of_range = true;
        break;
      }
    }
    CHECK(!out_of_range);
  }
}

TEST_FIXTURE(ASTL_random_independent_bits_engine,edge_cases) {
  mt19937_32 engine(42);

  // Test with w = 0
  {
    size_t w = 0;
    independent_bits_engine<mt19937_32, ni::tU32> ind_engine(engine, w);
    bool non_zero_generated = false;

    for (int i = 0; i < 100; ++i) {
      ni::tU32 value = ind_engine();
      if (value != 0) {
        non_zero_generated = true;
        break;
      }
    }
    CHECK(!non_zero_generated);
  }

  // Test with w equal to the number of bits in UIntType
  {
    size_t w = 32;
    independent_bits_engine<mt19937_32, ni::tU32> ind_engine(engine, w);
    bool error_found = false;

    for (int i = 0; i < 100; ++i) {
      ni::tU32 value = ind_engine();
      // Since w == 32, value can be any ni::tU32, so we just ensure no error occurs
      if (value < 0) {
        error_found = true;
        break;
      }
    }
    CHECK(!error_found);
  }
}

TEST_FIXTURE(ASTL_random_independent_bits_engine,test) {
  mt19937_32 engine(42);
  size_t w = 4;  // 4-bit numbers (0 to 15)
  independent_bits_engine<mt19937_32, ni::tU32> ind_engine(engine, w);
  const int num_samples = 16000;
  const int num_bins = 1 << w;
  vector<int> counts(num_bins, 0);

  for (int i = 0; i < num_samples; ++i) {
    ni::tU32 value = ind_engine();
    counts[value]++;
  }

  // Compute expected count per value
  double expected_count = num_samples / double(num_bins);
  double chi_squared = 0.0;

  for (int i = 0; i < num_bins; ++i) {
    double diff = counts[i] - expected_count;
    chi_squared += diff * diff / expected_count;
  }

  // Degrees of freedom = num_bins - 1
  // For num_bins = 16, degrees of freedom = 15
  // At 95% confidence level, chi-squared critical value ≈ 24.996
  const double chi_squared_critical = 24.996;
  CHECK(chi_squared < chi_squared_critical);
}

///////////////////////////////////////////////////////////////////////////
//
// ASTL_random_uniform_int_distribution
//
///////////////////////////////////////////////////////////////////////////
TEST_FIXTURE(ASTL_random_uniform_int_distribution,SmallType32Bit) {
  // This test remains unchanged
  astl::uniform_int_distribution<ni::tU32> dist(0, 100);
  astl::vector<ni::tU32> results;
  mt19937_32 gen(42); // Fixed seed for reproducibility

  const int num_iterations = 100000;  // Increase sample size for better distribution

  // Generate 10000 numbers
  results.reserve(num_iterations);
  niLoop(i, num_iterations) {
    results.push_back(dist(gen));
    if (i < 10) {
      niDebugFmt(("... result: %d", results.back()));
    }
  }

  // Check range
  niLoop(i, results.size()) {
    CHECK(results[i] <= 100);
  }

  // Check distribution
  astl::vector<int> counts(101, 0);
  niLoop(i, results.size()) {
    counts[results[i]]++;
  }

  // Simple uniformity check
  int min_count = *astl::min_element(counts.begin(), counts.end());
  int max_count = *astl::max_element(counts.begin(), counts.end());
  niLet variance = static_cast<double>(min_count) / max_count;
  niDebugFmt(("... variance: %g", variance));
  CHECK_GT(variance, 0.1);
  CHECK_CLOSE(1.0, variance, 0.5);
}

TEST_FIXTURE(ASTL_random_uniform_int_distribution,LargeType64Bit) {
  niLetK kMaxVal = 1ULL<<48ULL;
  astl::uniform_int_distribution<ni::tU64> dist(0, kMaxVal);
  mt19937_64 gen(42); // Fixed seed for reproducibility

  const int num_iterations = 1000000;  // Increase sample size for better distribution
  const int num_buckets = 16;  // Divide the range into 16 buckets
  astl::vector<int> bucket_counts(num_buckets, 0);

  // Generate numbers and count bucket occurrences
  niLoop(i, num_iterations) {
    ni::tU64 result = dist(gen);
    if (i < 10) {
      niDebugFmt(("... result: %d", result));
    }
    CHECK(result >= 0 && result <= kMaxVal);
    int bucket = static_cast<int>(result / (kMaxVal / num_buckets + 1));
    bucket_counts[bucket]++;
  }

  // Simple uniformity check
  niLet min_count = *astl::min_element(bucket_counts.begin(), bucket_counts.end());
  niLet max_count = *astl::max_element(bucket_counts.begin(), bucket_counts.end());
  double variance = static_cast<double>(min_count) / max_count;
  niDebugFmt(("... variance: %g", variance));
  CHECK_GT(variance, 0.1);
  CHECK_CLOSE(1.0, variance, 0.1);  // Tighter tolerance due to larger sample
}

TEST_FIXTURE(ASTL_random_uniform_int_distribution,cmp_32_and_64) {
  const ni::tU32 max_value = 1000;  // Small enough to be within 32-bit range
  const int num_iterations = 10;

  astl::uniform_int_distribution<ni::tU32> dist32(0, max_value);
  astl::uniform_int_distribution<ni::tU64> dist64(0, max_value);

  // Use the same seed for both generators to ensure identical sequences
  const ni::tU32 seed = 42;
  mt19937_32 gen32(seed);
  mt19937_32 gen64(seed);

  niLoop(i, num_iterations) {
    ni::tU32 result32 = dist32(gen32);
    ni::tU64 result64 = dist64(gen64);
    if (i < 10) {
      niDebugFmt(("... result32: %d, result64: %d", result32, result64));
    }

    // Check that results are identical
    CHECK_EQUAL(result32, result64);

    // Also check that results are within the specified range
    CHECK(result32 >= 0 && result32 <= max_value);
    CHECK(result64 >= 0 && result64 <= max_value);
  }
}

TEST_FIXTURE(ASTL_random_uniform_int_distribution,RandEngineU64) {
  niLetK kMaxVal = 1ULL<<48ULL;
  astl::uniform_int_distribution<ni::tU64> dist(0, kMaxVal);
  tRandEngineU64 gen(42); // Fixed seed for reproducibility

  const int num_iterations = 1000000;  // Increase sample size for better distribution
  const int num_buckets = 16;  // Divide the range into 16 buckets
  astl::vector<int> bucket_counts(num_buckets, 0);

  // Generate numbers and count bucket occurrences
  niLoop(i, num_iterations) {
    ni::tU64 result = dist(gen);
    if (i < 10) {
      niDebugFmt(("... result: %d", result));
    }
    CHECK(result >= 0 && result <= kMaxVal);
    int bucket = static_cast<int>(result / (kMaxVal / num_buckets + 1));
    bucket_counts[bucket]++;
  }

  // Simple uniformity check
  niLet min_count = *astl::min_element(bucket_counts.begin(), bucket_counts.end());
  niLet max_count = *astl::max_element(bucket_counts.begin(), bucket_counts.end());
  double variance = static_cast<double>(min_count) / max_count;
  niDebugFmt(("... variance: %g", variance));
  CHECK_GT(variance, 0.1);
  CHECK_CLOSE(1.0, variance, 0.1);  // Tighter tolerance due to larger sample
}

TEST_FIXTURE(ASTL_random_uniform_int_distribution,RandEngineBool) {
  astl::uniform_int_distribution<ni::tU64> dist(0,1);
  tRandEngineBool gen(42);

  const int num_iterations = 1000000;
  int numTrue = 0;
  int numFalse = 0;
  int numOther = 0;

  niLoop(i, num_iterations) {
    niLet result = dist(gen);
    if (result == eTrue) {
      ++numTrue;
    }
    else if (result == eFalse) {
      ++numFalse;
    }
    else {
      ++numOther;
    }
  }

  niDebugFmt(("... numTrue: %d", numTrue));
  niDebugFmt(("... numFalse: %d", numFalse));
  niDebugFmt(("... diff: %d", ni::Abs(numTrue-numFalse)));
  niDebugFmt(("... numOther: %d", numOther));
  CHECK_EQUAL(0, numOther);
  CHECK_CLOSE(numTrue, numFalse, (num_iterations/1000));
}

///////////////////////////////////////////////////////////////////////////
//
// ASTL_random_engine
//
///////////////////////////////////////////////////////////////////////////
TEST_FIXTURE(ASTL_random_engine,mt19937_32) {
  {
    static const ni::tU32 expected_values_seed42[10] = {
      1608637542ul,
      3421126067ul,
      4083286876ul,
      787846414ul,
      3143890026ul,
      3348747335ul,
      2571218620ul,
      2563451924ul,
      670094950ul,
      1914837113ul,
    };
    mt19937_32 engine_seed42(42);
    ni::tU32 generated_values_seed42[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed42[i] = engine_seed42();
      // niDebugFmt(("Seed 42 - Value %d: %d", i, generated_values_seed42[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_EQUAL(expected_values_seed42[i], generated_values_seed42[i]);
    }
  }

  {
    mt19937_32 engine_seed42(42);
    engine_seed42.discard(3);
    CHECK_EQUAL(787846414ul, engine_seed42());
  }

  {
    static const ni::tU32 expected_values_seed123[10] = {
      2991312382ul,
      3062119789ul,
      1228959102ul,
      1840268610ul,
      974319580ul,
      2967327842ul,
      2367878886ul,
      3088727057ul,
      3090095699ul,
      2109339754ul,
    };

    mt19937_32 engine_seed123(123);
    ni::tU32 generated_values_seed123[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed123[i] = engine_seed123();
      // niDebugFmt(("Seed 123 - Value %d: %d", i, generated_values_seed123[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_EQUAL(expected_values_seed123[i], generated_values_seed123[i]);
    }
  }
}

TEST_FIXTURE(ASTL_random_engine,mt19937_64) {
  {
    static const ni::tU64 expected_values_seed42[10] = {
      13930160852258120406ull,
      11788048577503494824ull,
      13874630024467741450ull,
      2513787319205155662ull,
      16662371453428439381ull,
      1735254072534978428ull,
      10598951352238613536ull,
      6878563960102566144ull,
      5052085463162682550ull,
      7199227068870524257ull,
    };
    mt19937_64 engine_seed42(42);
    ni::tU64 generated_values_seed42[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed42[i] = engine_seed42();
      // niDebugFmt(("Seed 42 - Value %d: %d", i, generated_values_seed42[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_EQUAL(expected_values_seed42[i], generated_values_seed42[i]);
    }
  }

  {
    mt19937_64 engine_seed42(42);
    engine_seed42.discard(6);
    CHECK_EQUAL((ni::tU64)10598951352238613536ull, engine_seed42());
  }

  {
    static const ni::tU64 expected_values_seed123[10] = {
      5777523539921853504ull,
      10256004525803361771ull,
      17308305258728183101ull,
      13582745572890801790ull,
      3549292889148046380ull,
      3599698824668116388ull,
      17542111641530190888ull,
      5357772020071635406ull,
      15109402569541188053ull,
      9005878083635208240ull,
    };

    mt19937_64 engine_seed123(123);
    ni::tU64 generated_values_seed123[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed123[i] = engine_seed123();
      // niDebugFmt(("Seed 123 - Value %d: %d", i, generated_values_seed123[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_EQUAL(expected_values_seed123[i], generated_values_seed123[i]);
    }
  }
}

TEST_FIXTURE(ASTL_random_engine,RandEngineI64) {
  {
    static const ni::tI64 expected_values_seed42[10] = {
      3553440125194606449ll,
      -1850319953427250933ll,
      2944846008281095542ll,
      2967949513766538177ll,
      -871195780171771494ll,
      8991170170311133055ll,
      5782326992556742051ll,
      -6738130162589255370ll,
      8429767824228939555ll,
      -7269967666377612241ll,
    };
    ni::sRandEngineTpl<tI64> engine_seed42(42);
    ni::tI64 generated_values_seed42[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed42[i] = engine_seed42();
      // niDebugFmt(("Seed 42 - Value %d: %d", i, generated_values_seed42[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_EQUAL(expected_values_seed42[i], generated_values_seed42[i]);
    }
  }

  {
    ni::sRandEngineTpl<tI64> engine_seed42(42);
    engine_seed42.discard(4);
    CHECK_EQUAL((ni::tI64)-871195780171771494ll, engine_seed42());
  }

  {
    static const ni::tI64 expected_values_seed123[10] = {
      -4048697855658168662ll,
      -7082651766117788805ll,
      3890004149087476432ll,
      -5400778987231701158ll,
      -2592972133096244477ll,
      6259147604711855511ll,
      3783169746704144679ll,
      -7141846280920818374ll,
      -3186356822603128222ll,
      842289039469464282ll,
    };

    ni::sRandEngineTpl<tI64> engine_seed123(123);
    ni::tI64 generated_values_seed123[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed123[i] = engine_seed123();
      // niDebugFmt(("Seed 123 - Value %d: %d", i, generated_values_seed123[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_EQUAL(expected_values_seed123[i], generated_values_seed123[i]);
    }
  }
}

TEST_FIXTURE(ASTL_random_engine,RandEngineI32) {
  {
    static const ni::tI32 expected_values_seed42[10] = {
      827349751,
      -430811186,
      685650391,
      691029595,
      -202841075,
      2093419937,
      1346302915,
      -1568843183,
      1962708268,
      -1692671252,
    };
    ni::sRandEngineTpl<tI32> engine_seed42(42);
    ni::tI32 generated_values_seed42[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed42[i] = engine_seed42();
      // niDebugFmt(("Seed 42 - Value %d: %d", i, generated_values_seed42[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_EQUAL(expected_values_seed42[i], generated_values_seed42[i]);
    }
  }

  {
    ni::sRandEngineTpl<tI32> engine_seed42(42);
    engine_seed42.discard(9);
    CHECK_EQUAL((ni::tI64)-1692671252, engine_seed42());
  }

  {
    static const ni::tI32 expected_values_seed123[10] = {
      -942660928,
      -1649058370,
      905712169,
      -1257466848,
      -603723371,
      1457321365,
      880837847,
      -1662840667,
      -741881510,
      196110699,
    };

    ni::sRandEngineTpl<tI32> engine_seed123(123);
    ni::tI32 generated_values_seed123[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed123[i] = engine_seed123();
      // niDebugFmt(("Seed 123 - Value %d: %d", i, generated_values_seed123[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_EQUAL(expected_values_seed123[i], generated_values_seed123[i]);
    }
  }
}

TEST_FIXTURE(ASTL_random_engine,RandEngineF32) {
  {
    static const ni::tF32 expected_values_seed42[10] = {
      0.022209f,
      0.146437f,
      0.887168f,
      0.017177f,
      0.555677f,
      0.440828f,
      0.934513f,
      0.834550f,
      0.784466f,
      0.742902f,
    };
    ni::sRandEngineTpl<tF32> engine_seed42(42);
    ni::tF32 generated_values_seed42[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed42[i] = engine_seed42();
      // niDebugFmt(("Seed 42 - Value %d: %f", i, generated_values_seed42[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_CLOSE(expected_values_seed42[i], generated_values_seed42[i], niEpsilon3);
    }
  }

  {
    ni::sRandEngineTpl<tF32> engine_seed42(42);
    engine_seed42.discard(2);
    CHECK_CLOSE(0.887168f, engine_seed42(), niEpsilon3);
  }

  {
    static const ni::tF32 expected_values_seed123[10] = {
      0.008484f,
      0.335387f,
      0.754434f,
      0.786164f,
      0.244527f,
      0.809957f,
      0.032432f,
      0.191564f,
      0.486658f,
      0.025737f,
    };

    ni::sRandEngineTpl<tF32> engine_seed123(123);
    ni::tF32 generated_values_seed123[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed123[i] = engine_seed123();
      // niDebugFmt(("Seed 123 - Value %d: %f", i, generated_values_seed123[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_CLOSE(expected_values_seed123[i], generated_values_seed123[i], niEpsilon3);
    }
  }
}

TEST_FIXTURE(ASTL_random_engine,RandEngineF64) {
  {
    static const ni::tF64 expected_values_seed42[10] = {
      0.022209,
      0.146437,
      0.887168,
      0.017177,
      0.555677,
      0.440828,
      0.934513,
      0.834550,
      0.784466,
      0.742902,
    };
    tRandEngineF64 engine_seed42(42);
    ni::tF64 generated_values_seed42[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed42[i] = engine_seed42();
      // niDebugFmt(("Seed 42 - Value %d: %f", i, generated_values_seed42[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_CLOSE(expected_values_seed42[i], generated_values_seed42[i], niEpsilon3);
    }
  }

  {
    ni::sRandEngineTpl<tF64> engine_seed42(42);
    engine_seed42.discard(5);
    CHECK_CLOSE(0.440828, engine_seed42(), niEpsilon3);
  }

  {
    static const ni::tF64 expected_values_seed123[10] = {
      0.008484,
      0.335387,
      0.754434,
      0.786164,
      0.244527,
      0.809957,
      0.032432,
      0.191564,
      0.486658,
      0.025737,
    };

    tRandEngineF64 engine_seed123(123);
    ni::tF64 generated_values_seed123[10];
    for (int i = 0; i < 10; ++i) {
      generated_values_seed123[i] = engine_seed123();
      // niDebugFmt(("Seed 123 - Value %d: %f", i, generated_values_seed123[i]));
    }
    for (int i = 0; i < 10; ++i) {
      CHECK_CLOSE(expected_values_seed123[i], generated_values_seed123[i], niEpsilon3);
    }
  }
}

TEST_FIXTURE(ASTL_random_engine,shuffle) {
  const vector<tU32> original = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  const vector<tU32> expected = {8, 10, 2, 7, 3, 6, 4, 1, 5, 9};

  vector<tU32> shuffled = original;

  tRandEngineU32 randEngine(42);
  shuffle(shuffled.begin(), shuffled.end(), randEngine);
  for (int i = 0; i < shuffled.size(); ++i) {
    //niDebugFmt(("shuffled %d: %d", i, shuffled[i]));
    CHECK_EQUAL(expected[i], shuffled[i]);
  }

  CHECK(is_permutation(shuffled.begin(), shuffled.end(), original.begin()));
  CHECK(shuffled != original);
}
