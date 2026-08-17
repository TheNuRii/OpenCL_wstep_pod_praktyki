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

#include "xPic.h"
#include "xPixelOps.h"
#include <cassert>
#include <cstring>

//=============================================================================================================================================================================

void xPic::create(uint32 Width, uint32 Height, uint32 BitDepth, bool UseInterleaved)
{
  m_Width  = Width;
  m_Height = Height;
  m_Margin = c_DefMargin;
  m_Stride = Width + (m_Margin << 1);

  m_BitsPerSample  = BitDepth;
  m_BytesPerSample = m_BitsPerSample <= 8 ? 1 : 2;

  m_BuffCmpNumPels  = (m_Width + (m_Margin << 1)) * (m_Height + (m_Margin << 1));
  m_BuffCmpNumBytes = m_BuffCmpNumPels * sizeof(uint16);

  for(uint32 c = 0; c < 3; c++)
  {
    m_BufferPlanar[c] = (uint16*)xAlignedMalloc(m_BuffCmpNumBytes, X_AlignmentPel);
    m_OriginPlanar[c] = m_BufferPlanar[c] + (m_Margin * m_Stride) + m_Margin;
  }
  if(UseInterleaved)
  {
    m_BufferInterleaved = (uint16*)xAlignedMalloc(m_BuffCmpNumBytes << 2, X_AlignmentPel);
    m_OriginInterleaved = m_BufferInterleaved + (m_Margin * (m_Stride << 2)) + (m_Margin << 2);
  }
}
void xPic::destroy()
{
  m_Width  = NOT_VALID;
  m_Height = NOT_VALID;
  m_Margin = NOT_VALID;
  m_Stride = NOT_VALID;

  m_BitsPerSample  = NOT_VALID;
  m_BytesPerSample = NOT_VALID;

  m_BuffCmpNumPels  = NOT_VALID;
  m_BuffCmpNumBytes = NOT_VALID;

  for(uint32 c = 0; c < 3; c++)
  {
    if(m_BufferPlanar[c] != nullptr) { xAlignedFree(m_BufferPlanar[c]); m_BufferPlanar[c] = nullptr; }
    m_OriginPlanar[c] = nullptr;
  }

  if(m_BufferInterleaved) { xAlignedFree(m_BufferInterleaved); m_BufferInterleaved = nullptr; }
  m_OriginInterleaved = nullptr;
}
bool xPic::check(const std::string& Name)
{
  boolV4 Correct = xMakeVec4(true);
  for(uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  { 
    Correct[CmpIdx] = xPixelOps::Check(m_OriginPlanar[CmpIdx], m_Stride, m_Width, m_Height, m_BitsPerSample);
  }

  for(uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
  {
    if(!Correct[CmpIdx])
    {
      fmt::printf("FILE BROKEN " + Name + " (CMP=%d)\n", CmpIdx);
      xPixelOps::FindBroken(m_OriginPlanar[CmpIdx], m_Stride, m_Width, m_Height, m_BitsPerSample);
      return false;
    }
  }

  return true;
}
void xPic::extend()
{
  for(uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) { xPixelOps::ExtendMargin(m_OriginPlanar[CmpIdx], m_Stride, m_Width, m_Height, m_Margin); }
}
void xPic::interleave()
{
  const int32 ExtWidth  = m_Width  + (m_Margin << 1);
  const int32 ExtHeight = m_Height + (m_Margin << 1);
  xPixelOps::Interleave(m_BufferInterleaved, m_BufferPlanar[0], m_BufferPlanar[1], m_BufferPlanar[2], 0, m_Stride << 2, m_Stride, ExtWidth, ExtHeight);
}

//=============================================================================================================================================================================
