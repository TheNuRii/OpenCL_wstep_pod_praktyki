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
 //                   Poznan University of Technology, Poznań, Poland

//=============================================================================================================================================================================

#include "CommonDef.h"
#include "xVec4.h"
#include <iostream>
#include <fstream>
#include <string>

//===============================================================================================================================================================================================================

class xPic
{
protected:
  static constexpr int32 c_DefMargin = 4;

protected:
  int32   m_Width           = NOT_VALID;
  int32   m_Height          = NOT_VALID;
  int32   m_Margin          = NOT_VALID;
  int32   m_Stride          = NOT_VALID;

  int32   m_BitsPerSample   = NOT_VALID;
  int32   m_BytesPerSample  = NOT_VALID;

  int32   m_BuffCmpNumPels  = NOT_VALID;
  int32   m_BuffCmpNumBytes = NOT_VALID;

  uint16* m_BufferPlanar[4] = { nullptr, nullptr, nullptr, nullptr };
  uint16* m_OriginPlanar[4] = { nullptr, nullptr, nullptr, nullptr };

  uint16* m_BufferInterleaved = nullptr;
  uint16* m_OriginInterleaved = nullptr;

public:
  xPic () { };
  xPic (uint32 Width, uint32 Height, uint32 BitDepth, bool UseInterleaved) { create(Width, Height, BitDepth, UseInterleaved); }
  ~xPic() { destroy(); }

  void   create (uint32 Width, uint32 Height, uint32 BitDepth, bool UseInterleaved);
  void   destroy();

  bool   check     (const std::string& Name);
  void   extend    ();
  void   interleave();

public:
  inline int32 getWidth   () const { return m_Width           ; }
  inline int32 getHeight  () const { return m_Height          ; }
  inline int32 getArea    () const { return m_Width * m_Height; }
  inline int32 getMargin  () const { return m_Margin          ; }
  inline int32 getStride  () const { return m_Stride          ; }
  inline int32 getBitDepth() const { return m_BitsPerSample   ; }

  inline uint16 getMaxPelValue() const { return (uint16)xBitDepth2MaxValue(m_BitsPerSample); }

  //planar access
  inline uint16*       getAddr  (uint32 CmpIdx   )       { return m_OriginPlanar[CmpIdx]; }
  inline const uint16* getAddr  (uint32 CmpIdx   ) const { return m_OriginPlanar[CmpIdx]; }  
  inline int32         getOffset(int32 x, int32 y) const { return y * m_Stride + x; }

  //interleaved access
  inline uint16V4*       getAddrVec()       { return (uint16V4*)m_OriginInterleaved; }
  inline const uint16V4* getAddrVec() const { return (uint16V4*)m_OriginInterleaved; }

  //buffer access
  inline uint16*       getBuffer(uint32 CmpIdx)       { return m_BufferPlanar[CmpIdx]; }
  inline const uint16* getBuffer(uint32 CmpIdx) const { return m_BufferPlanar[CmpIdx]; }
};

//===============================================================================================================================================================================================================
