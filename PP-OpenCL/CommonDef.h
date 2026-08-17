#pragma once

/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2020, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

// Original authors: Jakub Stankowski, jakub.stankowski@put.poznan.pl,
//                   Adrian Dziembowski, adrian.dziembowski@put.poznan.pl,
//                   Poznan University of Technology, Poznañ, Poland


//=============================================================================================================================================================================
// MSVC workaround
//=============================================================================================================================================================================
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <type_traits>
#include <algorithm>
#include <string>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cinttypes>
#include <cfloat>

#define FMT_HEADER_ONLY
#define FMT_USE_WINDOWS_H 0
#include "lib_fmt/format.h"
#include "lib_fmt/printf.h"
#include "lib_fmt/ostream.h"
#undef FMT_USE_WINDOWS_H
#undef FMT_HEADER_ONLY

//=============================================================================================================================================================================
// Compile time settings
//=============================================================================================================================================================================
#define USE_KBNS                1 // use Kahanand-Babuska-Neumaier floating point sumation algorithm
#define USE_FIXED_WEIGHTS       1 // use fixed (4, 1, 1) weights during NxN block scan
#define WSPSNR_PEAK_VALUE_8BIT  1 // use 1020 as peak value for 10-bps videos

//=============================================================================================================================================================================
// Hard coded constrains
//=============================================================================================================================================================================
#define NOT_VALID  -1

//=============================================================================================================================================================================
// System section
//=============================================================================================================================================================================
#if (defined(WIN32) || defined(_WIN32) || defined(_WIN64))
#define X_SYSTEM_WINDOWS 1
#elif defined(__linux__)
#define X_SYSTEM_LINUX 1
#else
#defiene X_SYSTEM_UNKNOWN 0
#endif

//=============================================================================================================================================================================
// Memmory and alignment section
//=============================================================================================================================================================================
#define X_Log2MemSizePage   12                //Memmory page size = 4kB
#define X_MemSizePage       (1<<X_Log2MemSizePage)
#define X_AlignmentPel      X_MemSizePage     //pel alignment

//Allocation with explicit alignment
#if defined(X_SYSTEM_WINDOWS)
#define xAlignedMalloc(size, alignment) _aligned_malloc((size), (alignment))
#define xAlignedFree(memmory)           _aligned_free((memmory))
#elif defined(X_SYSTEM_LINUX)
#define xAlignedMalloc(size, alignment) aligned_alloc((alignment), (size))
#define xAlignedFree(memmory)           free((memmory))
#endif

//Aligned variables/objects
#if defined(X_SYSTEM_WINDOWS)
#define xAligned(x) __declspec(align(x))
#elif defined(X_SYSTEM_LINUX)
#define xAligned(x) __attribute__ ((aligned(x)))
#else
#error "Unknown implementation"
#endif

//=============================================================================================================================================================================
// C99 restrict pointers support
//=============================================================================================================================================================================
#if defined(_MSC_VER) && _MSC_VER >= 1400 //MSVC
#define restrict __restrict
#elif defined(__GNUC__) && ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)) //GCC
#ifdef __cplusplus
#define restrict __restrict
#endif
#ifdef __STDC_VERSION__ //STANDARD_C_1990
#define restrict __restrict
#endif
#if (__STDC_VERSION__ >= 199409L) //STANDARD_C_1994
#define restrict __restrict
#endif
#if (__STDC_VERSION__ >= 199901L) //STANDARD_C_1999
//restrict defined
#endif
#elif defined(__STDC__) //STANDARD_C_1989
#define restrict
#endif

//=============================================================================================================================================================================
// Integers anf float types
//=============================================================================================================================================================================
typedef  int64_t   int64;
typedef  int32_t   int32;
typedef  int16_t   int16;
typedef  int8_t    int8;

typedef  uint64_t  uint64;
typedef  uint32_t  uint32;
typedef  uint16_t  uint16;
typedef  uint8_t   uint8;

typedef  uint8     byte;

typedef  float     flt32;
typedef  double    flt64;

typedef  uintptr_t uintPtr;
typedef  size_t    uintSize;

static const flt32 flt32_max = std::numeric_limits<flt32>::max    ();
static const flt32 flt32_min = std::numeric_limits<flt32>::lowest ();
static const flt32 flt32_eps = std::numeric_limits<flt32>::epsilon();

static const flt64 flt64_max = std::numeric_limits<flt64>::max    ();
static const flt64 flt64_min = std::numeric_limits<flt64>::lowest ();
static const flt64 flt64_eps = std::numeric_limits<flt64>::epsilon();

//=============================================================================================================================================================================
// Limits
//=============================================================================================================================================================================
static const int64  int64_max  = INT64_MAX;
static const int64  int64_min  = INT64_MIN;
static const int32  int32_max  = INT32_MAX;
static const int32  int32_min  = INT32_MIN;
static const int16  int16_max  = INT16_MAX;
static const int16  int16_min  = INT16_MIN;
static const int8   int8_max   = INT8_MAX ;
static const int8   int8_min   = INT8_MIN ;

static const uint64 uint64_max = UINT64_MAX;
static const uint64 uint64_min = 0         ;
static const uint32 uint32_max = UINT32_MAX;
static const uint32 uint32_min = 0         ;
static const uint16 uint16_max = UINT16_MAX;
static const uint16 uint16_min = 0         ;
static const uint8  uint8_max  = UINT8_MAX ;
static const uint8  uint8_min  = 0         ;

//=============================================================================================================================================================================
// Basic ops
//=============================================================================================================================================================================
template <class XXX> static inline XXX  xMin     (XXX a, XXX b) { return std::min(a, b); }
template <class XXX> static inline XXX  xMax     (XXX a, XXX b) { return std::max(a, b); }
template <class XXX> static inline XXX  xClip    (XXX x, XXX min, XXX max) { return xMax(min, xMin(x, max)); }
template <class XXX> static inline XXX  xClipU8  (XXX x) {return xMax((XXX)0,xMin(x,(XXX)255));}
template <class XXX> static inline XXX  xClipS8  (XXX x) {return xMax((XXX)-128,xMin(x,(XXX)127));}
template <class XXX> static inline XXX  xClipU16 (XXX x) {return xMax((XXX)0,xMin(x,(XXX)65536));}
template <class XXX> static inline XXX  xClipS16 (XXX x) {return xMax((XXX)-32768,xMin(x,(XXX)32767));}
template <class XXX> static inline XXX  xAbs     (XXX a) { return (XXX)std::abs(a); }
template <class XXX> static inline XXX  xPow2    (XXX x) { return x * x; }

static inline int32 xRoundFloatToInt32 (float  Float) { return (int32)(std::round(Float)); }
static inline int32 xRoundDoubleToInt32(double Float) { return (int32)(std::round(Float)); }

template <class XXX> static inline XXX  xBitDepth2MidValue(XXX BitDepth) { return (1 << (BitDepth - 1)); }
template <class XXX> static inline XXX  xBitDepth2MaxValue(XXX BitDepth) { return ((1 << BitDepth) - 1); }
template <class XXX> static inline XXX  xBitDepth2NumLevel(XXX BitDepth) { return (1 << BitDepth); }

//=============================================================================================================================================================================
// time is money
//=============================================================================================================================================================================
typedef std::chrono::high_resolution_clock        tClock;
typedef tClock::time_point                        tTimePoint;
typedef tClock::duration                          tDuration;
typedef std::chrono::duration<double, std::milli> tDurationMS;
typedef std::chrono::duration<double            > tDurationS;

//=============================================================================================================================================================================
// Math constants
//=============================================================================================================================================================================
template<class XXX> constexpr XXX xc_PI       = XXX(3.1415926535897932385L);
template<class XXX> constexpr XXX xc_DegToRad = xc_PI<XXX> / XXX(180);
template<class XXX> constexpr XXX xc_RadToDeg = XXX(180) / xc_PI<XXX>;

//=============================================================================================================================================================================
// Multiple and remainder
//=============================================================================================================================================================================
static const uint32 c_MultipleMask4    = 0xFFFFFFFC;
static const uint32 c_MultipleMask8    = 0xFFFFFFF8;
static const uint32 c_MultipleMask16   = 0xFFFFFFF0;
static const uint32 c_MultipleMask32   = 0xFFFFFFE0;
static const uint32 c_MultipleMask64   = 0xFFFFFFC0;
static const uint32 c_MultipleMask128  = 0xFFFFFF80;

static const uint32 c_RemainderMask4   = 0x00000003;
static const uint32 c_RemainderMask8   = 0x00000007;
static const uint32 c_RemainderMask16  = 0x0000000F;
static const uint32 c_RemainderMask32  = 0x0000001F;
static const uint32 c_RemainderMask64  = 0x0000003F;
static const uint32 c_RemainderMask128 = 0x0000007F;

//=============================================================================================================================================================================
// SSE SIMD section
//=============================================================================================================================================================================
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86))
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

//MSVC does not define __SSEn__ macros. Assuming all extensions present.
#if defined(_MSC_VER)
#define __SSE__    1
#define __SSE2__   1
#define __SSE3__   1
#define __SSSE3__  1
#define __SSE4_1__ 1
#define __SSE4_2__ 1
#endif

#define X_SSE1    __SSE__    //Pentium III
#define X_SSE2    __SSE2__   //Pentium 4    (Willamette, Northwood, Gallatin)
#define X_SSE3    __SSE3__   //Pentium 4    (Prescott, Cedar Mill)
#define X_SSSE3   __SSSE3__  //Core 2       (Conroe, Merom)
#define X_SSE4_1  __SSE4_1__ //Core 2       (Penryn, Wolfdale)
#define X_SSE4_2  __SSE4_2__ //Core iX nnn  (Nehalem, Westmere)
#define X_SSE_ALL X_SSE1 && X_SSE2 && X_SSE3 && X_SSSE3 && X_SSE4_1 && X_SSE4_2

//=============================================================================================================================================================================
