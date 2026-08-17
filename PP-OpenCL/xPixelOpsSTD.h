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

#include "CommonDef.h"

//===============================================================================================================================================================================================================

class xPixelOpsSTD
{
public:
  //Image
  static void Copy         (uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   );
  static void Cvt          (uint16* restrict Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   );
  static void Cvt          (uint8*  restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 Width   , int32 Height   );
  static void Upsample     (uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight);
  static void Downsample   (uint16* restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight);
  static void CvtUpsample  (uint16* restrict Dst, const uint8*  Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight);
  static void CvtDownsample(uint8*  restrict Dst, const uint16* Src, int32 DstStride, int32 SrcStride, int32 DstWidth, int32 DstHeight);
  static bool Check        (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth);
  static bool FindBroken   (const uint16* Src, int32 SrcStride, int32 Width, int32 Height, int32 BitDepth);
  static void ExtendMargin (uint16* Addr, int32 Stride, int32 Width, int32 Height, int32 Margin);
  static void Interleave   (uint16* restrict DstABCD, const uint16* SrcA, const uint16* SrcB, const uint16* SrcC, uint16 ValueD, int32 DstStride, int32 SrcStride, int32 Width, int32 Height);

  //SSD
  static uint64 CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 Area);
  static uint64 CalcSSD(const uint16* restrict Org, const uint16* restrict Dist, int32 OStride, int32 DStride, int32 Width, int32 Height);
};

//===============================================================================================================================================================================================================
