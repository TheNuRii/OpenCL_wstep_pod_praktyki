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

//===============================================================================================================================================================================================================

#include "xPixelOpsSTD.h"
#if X_USE_SSE && X_SSE_ALL
#include "xPixelOpsSSE.h"
#endif //X_USE_SSE && X_SSE_ALL

//===============================================================================================================================================================================================================

class xPixelOps
{
public:  
  static inline void Copy         (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSTD::Copy         (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void Downsample   (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::Downsample   (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void CvtDownsample(uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::CvtDownsample(Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline bool FindBroken   (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsSTD::FindBroken(Src, SrcStride, Width, Height, BitDepth); }
  static inline void ExtendMargin (uint16* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin) { xPixelOpsSTD::ExtendMargin(Addr, Stride, Width, Height, Margin); }

#if X_USE_SSE && X_SSE_ALL

  static inline void Cvt          (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSSE::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void Cvt          (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSSE::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void Upsample     (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::Upsample     (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void CvtUpsample  (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSSE::CvtUpsample  (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  
  static inline bool Check        (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsSSE::Check(Src, SrcStride, Width, Height, BitDepth); }
  static inline void Interleave   (uint16* DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsSSE::Interleave(DstABCD, SrcA, SrcB, SrcC, ValueD, DstStride, SrcStride, Width, Height); }

  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xPixelOpsSSE::CalcSSD(Org, Dist,                   Area          ); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xPixelOpsSSE::CalcSSD(Org, Dist, OStride, DStride, Width,  Height); }

#else //X_USE_SSE && X_SSE_ALL

  static inline void Cvt          (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSTD::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void Cvt          (uint8*  Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   ) { xPixelOpsSTD::Cvt          (Dst, Src, DstStride, SrcStride, Width   , Height   ); }
  static inline void Upsample     (uint16* Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::Upsample     (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  static inline void CvtUpsample  (uint16* Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight) { xPixelOpsSTD::CvtUpsample  (Dst, Src, DstStride, SrcStride, DstWidth, DstHeight); }
  
  static inline bool Check        (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth) { return xPixelOpsSTD::Check(Src, SrcStride, Width, Height, BitDepth); }
  static inline void Interleave   (uint16* DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height) { xPixelOpsSTD::Interleave(DstABCD, SrcA, SrcB, SrcC, ValueD, DstStride, SrcStride, Width, Height); }

  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist,                               int32 Area               ) { return xPixelOpsSTD::CalcSSD(Org, Dist,                   Area          ); }
  static inline uint64 CalcSSD(const uint16* Org, const uint16* Dist, int32 OStride, int32 DStride, int32 Width, int32 Height) { return xPixelOpsSTD::CalcSSD(Org, Dist, OStride, DStride, Width,  Height); }

#endif //X_USE_SSE && X_SSE_ALL

};

//===============================================================================================================================================================================================================
