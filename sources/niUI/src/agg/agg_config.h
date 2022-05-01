#ifndef AGG_CONFIG_INCLUDED
#define AGG_CONFIG_INCLUDED

#pragma warning(disable:4305) // warning C4305: 'initializing' : truncation from 'agg_real' to 'const float'
#pragma warning(disable:4244) // warning C4244: 'return' : conversion from 'agg_real' to 'float', possible loss of data

// This file can be used to redefine the default basic types such as:
//
// AGG_INT8
// AGG_INT8U
// AGG_INT16
// AGG_INT16U
// AGG_INT32
// AGG_INT32U
// AGG_INT64
// AGG_INT64U
//
// Just replace this file with new defines if necessary.
// For example, if your compiler doesn't have a 64 bit integer type
// you can still use AGG if you define the follows:
//
// #define AGG_INT64  int
// #define AGG_INT64U unsigned
//
// It will result in overflow in 16 bit-per-component image/pattern resampling
// but it won't result any crash and the rest of the library will remain
// fully functional.

#endif
