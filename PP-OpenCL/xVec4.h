#pragma once

/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2019, ISO/IEC
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
 * int32_tERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

 // Original author: Jakub Stankowski, jakub.stankowski@put.poznan.pl,
 //                  Poznan University of Technology, Poznañ, Poland

#include "CommonDef.h"
#include <array>
#include <vector>

//===============================================================================================================================================================================================================
// Simplified Vec4
//===============================================================================================================================================================================================================

template<typename XXX> class xVec4
{
public:
  using tVec = xVec4<XXX>;

protected:
  xAligned(4 * sizeof(XXX)) XXX m_V[4];

public:
  inline xVec4() {}
  constexpr xVec4(const XXX x, const XXX y, const XXX z, const XXX a) : m_V{ x, y, z, a } {}

  template<typename YYY> explicit operator xVec4<YYY>() const { return xVec4<YYY>((YYY)m_V[0], (YYY)m_V[1], (YYY)m_V[2], (YYY)m_V[3]); }

  inline XXX   getSum    () const { return (m_V[0] + m_V[1] + m_V[2] + m_V[3]); }
  inline tVec  getVecPow2() const { return tVec(xPow2(m_V[0]), xPow2(m_V[1]), xPow2(m_V[2]), xPow2(m_V[3])); }

  inline const tVec& operator += (const tVec& Vec4)       { m_V[0]  += Vec4.m_V[0]; m_V[1]  += Vec4.m_V[1]; m_V[2]  += Vec4.m_V[2]; m_V[3]  += Vec4.m_V[3]; return *this;}  
  inline const tVec& operator -= (const tVec& Vec4)       { m_V[0]  -= Vec4.m_V[0]; m_V[1]  -= Vec4.m_V[1]; m_V[2]  -= Vec4.m_V[2]; m_V[3]  -= Vec4.m_V[3]; return *this;}
  inline const tVec& operator *= (const tVec& Vec4)       { m_V[0]  *= Vec4.m_V[0]; m_V[1]  *= Vec4.m_V[1]; m_V[2]  *= Vec4.m_V[2]; m_V[3]  *= Vec4.m_V[3]; return *this;}
  inline const tVec& operator /= (const tVec& Vec4)       { m_V[0]  /= Vec4.m_V[0]; m_V[1]  /= Vec4.m_V[1]; m_V[2]  /= Vec4.m_V[2]; m_V[3]  /= Vec4.m_V[3]; return *this;}

  inline const tVec& operator += (const XXX   i   )       { m_V[0]  += i;           m_V[1]  += i;           m_V[2]  += i;           m_V[3]  += i;           return *this;}
  inline const tVec& operator -= (const XXX   i   )       { m_V[0]  -= i;           m_V[1]  -= i;           m_V[2]  -= i;           m_V[3]  -= i;           return *this;}
  inline const tVec& operator *= (const XXX   i   )       { m_V[0]  *= i;           m_V[1]  *= i;           m_V[2]  *= i;           m_V[3]  *= i;           return *this;}
  inline const tVec& operator /= (const XXX   i   )       { m_V[0]  /= i;           m_V[1]  /= i;           m_V[2]  /= i;           m_V[3]  /= i;           return *this;}

  inline const tVec  operator +  (const tVec& Vec4) const { return xVec4(m_V[0]  + Vec4.m_V[0], m_V[1]  + Vec4.m_V[1], m_V[2]  + Vec4.m_V[2], m_V[3]  + Vec4.m_V[3]);}  
  inline const tVec  operator -  (const tVec& Vec4) const { return xVec4(m_V[0]  - Vec4.m_V[0], m_V[1]  - Vec4.m_V[1], m_V[2]  - Vec4.m_V[2], m_V[3]  - Vec4.m_V[3]);}
  inline const tVec  operator *  (const tVec& Vec4) const { return xVec4(m_V[0]  * Vec4.m_V[0], m_V[1]  * Vec4.m_V[1], m_V[2]  * Vec4.m_V[2], m_V[3]  * Vec4.m_V[3]);}
  inline const tVec  operator /  (const tVec& Vec4) const { return xVec4(m_V[0]  / Vec4.m_V[0], m_V[1]  / Vec4.m_V[1], m_V[2]  / Vec4.m_V[2], m_V[3]  / Vec4.m_V[3]);}

  inline const tVec  operator -  (                ) const { return xVec4(-m_V[0], -m_V[1], -m_V[2], -m_V[3]); }

  inline const tVec  operator +  (const XXX   i   ) const { return xVec4(m_V[0]  + i,           m_V[1]  + i,           m_V[2]  + i,           m_V[3]  + i          );}
  inline const tVec  operator -  (const XXX   i   ) const { return xVec4(m_V[0]  - i,           m_V[1]  - i,           m_V[2]  - i,           m_V[3]  - i          );}
  inline const tVec  operator *  (const XXX   i   ) const { return xVec4(m_V[0]  * i,           m_V[1]  * i,           m_V[2]  * i,           m_V[3]  * i          );}
  inline const tVec  operator /  (const XXX   i   ) const { return xVec4(m_V[0]  / i,           m_V[1]  / i,           m_V[2]  / i,           m_V[3]  / i          );}

  constexpr inline bool operator==  (const tVec& Vec4) const {return (m_V[0]==Vec4.m_V[0] && m_V[1]==Vec4.m_V[1] && m_V[2]==Vec4.m_V[2] && m_V[3]==Vec4.m_V[3]);}
  constexpr inline bool operator!=  (const tVec& Vec4) const {return (m_V[0]!=Vec4.m_V[0] || m_V[1]!=Vec4.m_V[1] || m_V[2]!=Vec4.m_V[2] || m_V[3]!=Vec4.m_V[3]);}


  inline XXX&        operator[]  (int32_t Idx)       {return m_V[Idx];}
  inline const XXX&  operator[]  (int32_t Idx) const {return m_V[Idx];}

  inline void  clip (const tVec& Min, const tVec& Max) { m_V[0] = xClip (m_V[0], Min.m_V[0], Max.m_V[0]); m_V[1] = xClip (m_V[1], Min.m_V[1], Max.m_V[1]); m_V[2] = xClip (m_V[2], Min.m_V[2], Max.m_V[2]); m_V[3] = xClip (m_V[3], Min.m_V[3], Max.m_V[3]); }
};

//===============================================================================================================================================================================================================

typedef xVec4<  bool>   boolV4;
typedef xVec4< int16>  int16V4;
typedef xVec4<uint16> uint16V4;
typedef xVec4< int32>  int32V4;
typedef xVec4<uint32> uint32V4;
typedef xVec4< int64>  int64V4;
typedef xVec4<uint64> uint64V4;
typedef xVec4< flt32>  flt32V4;
typedef xVec4< flt64>  flt64V4;

//=============================================================================================================================================================================

template<typename XXX> static inline xVec4<XXX> xMakeVec4(XXX Value) { return xVec4<XXX>{Value, Value, Value, Value}; }

//=============================================================================================================================================================================

template <class XXX> static inline int32V4 xRoundFltToInt32(const xVec4<XXX>& FltV);
template <> inline int32V4 xRoundFltToInt32(const xVec4< float>& FltV) { return { xRoundFloatToInt32 (FltV[0]), xRoundFloatToInt32 (FltV[1]), xRoundFloatToInt32 (FltV[2]), xRoundFloatToInt32 (FltV[3])}; }
template <> inline int32V4 xRoundFltToInt32(const xVec4<double>& FltV) { return { xRoundDoubleToInt32(FltV[0]), xRoundDoubleToInt32(FltV[1]), xRoundDoubleToInt32(FltV[2]), xRoundDoubleToInt32(FltV[3])}; }

//=============================================================================================================================================================================
