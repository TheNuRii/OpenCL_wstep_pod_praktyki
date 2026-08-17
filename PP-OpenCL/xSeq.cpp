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

#include "xSeq.h"
#include "xPixelOps.h"
#include "xFile.h"
#include <cassert>
#include <cstring>

//=============================================================================================================================================================================

void xSeq::create(uint32 Width, uint32 Height, uint32 BitDepth, uint32 ChromaFormat)
{
  m_Width  = Width;
  m_Height = Height;

  m_BitsPerSample  = BitDepth;
  m_BytesPerSample = m_BitsPerSample <= 8 ? 1 : 2;
  m_ChromaFormat   = ChromaFormat;

  m_FileCmpNumPels  = m_Width * m_Height;
  m_FileCmpNumBytes = m_Width * m_Height * m_BytesPerSample;

  switch(m_ChromaFormat)
  {
    case 444: m_FileImgNumBytes = 3 * m_FileCmpNumBytes; break;
    case 420: m_FileImgNumBytes = m_FileCmpNumBytes + (m_FileCmpNumBytes >> 1); break;
    case 400: m_FileImgNumBytes = m_FileCmpNumBytes; break;
    default: assert(0);
  }

  m_FileBuffer = (uint8*)xAlignedMalloc(m_FileImgNumBytes, X_AlignmentPel);
}
void xSeq::destroy()
{
  m_FileName.clear();
  m_FileHandle = nullptr;

  m_Width  = NOT_VALID;
  m_Height = NOT_VALID;

  m_BitsPerSample  = NOT_VALID;
  m_BytesPerSample = NOT_VALID;
  m_ChromaFormat   = NOT_VALID;

  m_FileCmpNumPels  = NOT_VALID;
  m_FileCmpNumBytes = NOT_VALID;

  if(m_FileBuffer) { xAlignedFree(m_FileBuffer); m_FileBuffer = nullptr; }
}
bool xSeq::openFile(const std::string& FileName, eMode FileMode)
{
  m_FileName   = FileName;
  m_FileMode   = FileMode;
  switch(FileMode)
  {
    case eMode::Read : m_FileHandle = fopen(m_FileName.c_str(), "rb"); break;
    case eMode::Write: m_FileHandle = fopen(m_FileName.c_str(), "wb"); break;
    default: return false;
  }  
  return m_FileHandle != nullptr;
}
bool xSeq::closeFile()
{
  fclose(m_FileHandle);
  return true;
}
bool xSeq::seekFrame(uint32 FrameNumber)
{
  //seek frame
  uintSize Offset = (uintSize)m_FileImgNumBytes * (uintSize)FrameNumber;
  xFseek64(m_FileHandle, Offset, SEEK_SET);
  return true;
}
bool xSeq::readFrame(xPic* Pic)
{
  //read frame
  uintSize Read = fread(m_FileBuffer, 1, m_FileImgNumBytes, m_FileHandle);  
  if(Read != (uintSize)m_FileImgNumBytes) { return false; }

  //unpack frame
  bool Unpacked = xUnpackFrame(Pic);
  if(!Unpacked) { return false; }

  return true;
}
bool xSeq::writeFrame(const xPic* Pic)
{
  //pack frame
  bool Packed = xPackFrame(Pic);
  if(!Packed) { return false; }

  //write frame
  uintSize Written = fwrite(m_FileBuffer, 1, m_FileImgNumBytes, m_FileHandle);
  if(Written != (uintSize)m_FileImgNumBytes) { return false; }
  fflush(m_FileHandle);

  return true;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool xSeq::xUnpackFrame(xPic* Pic)
{
  uint16* PtrLm      = Pic->getAddr  (0);
  uint16* PtrCb      = Pic->getAddr  (1);
  uint16* PtrCr      = Pic->getAddr  (2);
  const int32 Stride = Pic->getStride( );

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (PtrLm, m_FileBuffer           , Stride, m_Width, m_Width, m_Height); }
  else                      { xPixelOps::Copy(PtrLm, (uint16*)(m_FileBuffer), Stride, m_Width, m_Width, m_Height); }

  //process chroma
  const uint8* ChromaPtr = m_FileBuffer + m_FileCmpNumBytes;
  if(m_ChromaFormat == 420)
  {
    const int32 ChromaFileCmpNumBytes = m_FileCmpNumBytes >> 2;
    const int32 ChromaFileStride      = m_Width >> 1;
    if(m_BytesPerSample == 1)
    {
      xPixelOps::CvtUpsample(PtrCb, ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
      ChromaPtr += ChromaFileCmpNumBytes;
      xPixelOps::CvtUpsample(PtrCr, ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
    }
    else
    {
      xPixelOps::Upsample(PtrCb, (uint16*)ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
      ChromaPtr += ChromaFileCmpNumBytes;
      xPixelOps::Upsample(PtrCr, (uint16*)ChromaPtr, Stride, ChromaFileStride, m_Width, m_Height);
    }
  }
  else if(m_ChromaFormat == 444)
  {
    if(m_BytesPerSample == 1)
    { 
      xPixelOps::Cvt(PtrCb, ChromaPtr, Stride, m_Width, m_Width, m_Height);
      ChromaPtr += m_FileCmpNumBytes;
      xPixelOps::Cvt(PtrCr, ChromaPtr, Stride, m_Width, m_Width, m_Height);
    }
    else
    { 
      xPixelOps::Copy(PtrCb, (uint16*)ChromaPtr, Stride, m_Width, m_Width, m_Height);
      ChromaPtr += m_FileCmpNumBytes;
      xPixelOps::Copy(PtrCr, (uint16*)ChromaPtr, Stride, m_Width, m_Width, m_Height);
    }
  }
  else { return false; }
  return true;
}
bool xSeq::xPackFrame(const xPic* Pic)
{
  const uint16* PtrLm  = Pic->getAddr  (0);
  const uint16* PtrCb  = Pic->getAddr  (1);
  const uint16* PtrCr  = Pic->getAddr  (2);
  const int32   Stride = Pic->getStride( );

  //process luma
  if(m_BytesPerSample == 1) { xPixelOps::Cvt (m_FileBuffer           , PtrLm, m_Width, Stride, m_Width, m_Height); }
  else                      { xPixelOps::Copy((uint16*)(m_FileBuffer), PtrLm, m_Width, Stride, m_Width, m_Height); }

  //process chroma
  uint8* ChromaPtr = m_FileBuffer + m_FileCmpNumBytes;
  if(m_ChromaFormat == 420)
  {
    const int32 ChromaFileCmpNumBytes = m_FileCmpNumBytes >> 2;
    const int32 ChromaFileStride      = m_Width >> 1;
    if(m_BytesPerSample == 1)
    {
      xPixelOps::CvtDownsample(ChromaPtr, PtrCb, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
      ChromaPtr += ChromaFileCmpNumBytes;
      xPixelOps::CvtDownsample(ChromaPtr, PtrCr, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
    }
    else
    {
      xPixelOps::Downsample((uint16*)ChromaPtr, PtrCb, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
      ChromaPtr += ChromaFileCmpNumBytes;
      xPixelOps::Downsample((uint16*)ChromaPtr, PtrCr, ChromaFileStride, Stride, m_Width>>1, m_Height>>1);
    }
  }
  else if(m_ChromaFormat == 444)
  {
    if(m_BytesPerSample == 1)
    { 
      xPixelOps::Cvt(ChromaPtr, PtrCb, m_Width, Stride, m_Width, m_Height);
      ChromaPtr += m_FileCmpNumBytes;
      xPixelOps::Cvt(ChromaPtr, PtrCr, m_Width, Stride, m_Width, m_Height);
    }
    else
    { 
      xPixelOps::Copy((uint16*)ChromaPtr, PtrCb, m_Width, Stride, m_Width, m_Height);
      ChromaPtr += m_FileCmpNumBytes;
      xPixelOps::Copy((uint16*)ChromaPtr, PtrCr, m_Width, Stride, m_Width, m_Height);
    }
  }
  else { return false; }
  return true;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

uint32 xSeq::calcNumFramesInFile(uint32 Width, uint32 Height, uint32 BitDepth, uint32 ChromaFormat, uint64 FileSize)
{
  int32 BytesPerSample  = BitDepth <= 8 ? 1 : 2;
  int32 FileCmpNumPels  = Width * Height;
  int32 FileCmpNumBytes = FileCmpNumPels * BytesPerSample;

  int32 FileImgNumBytes = NOT_VALID;
  switch(ChromaFormat)
  {
    case 444: FileImgNumBytes = 3 * FileCmpNumBytes; break;
    case 420: FileImgNumBytes = FileCmpNumBytes + (FileCmpNumBytes >> 1); break;
    case 400: FileImgNumBytes = FileCmpNumBytes; break;
    default: assert(0);
  }

  return (int32)(FileSize / FileImgNumBytes);
}

//=============================================================================================================================================================================
