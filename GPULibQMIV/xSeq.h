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
#include "xPic.h"
#include <iostream>
#include <fstream>
#include <string>

//===============================================================================================================================================================================================================

class xSeq
{
public:
  enum class eMode   : int32 { Unknown, Read, Write };
  enum class eResult : int32 { Correct, EndOfFile, Error };

protected:
  std::string m_FileName;
  eMode       m_FileMode;
  FILE*       m_FileHandle;

  int32   m_Width           = NOT_VALID;
  int32   m_Height          = NOT_VALID;

  int32   m_BitsPerSample   = NOT_VALID;
  int32   m_BytesPerSample  = NOT_VALID;
  int32   m_ChromaFormat    = NOT_VALID;

  int32   m_FileCmpNumPels  = NOT_VALID;
  int32   m_FileCmpNumBytes = NOT_VALID;
  int32   m_FileImgNumBytes = NOT_VALID;

  uint8*  m_FileBuffer;

public:
  xSeq() { m_FileBuffer = nullptr; };
  xSeq(uint32 Width, uint32 Height, uint32 BitDepth, uint32 ChromaFormat) { create(Width, Height, BitDepth, ChromaFormat); }
  ~xSeq() { destroy(); }

  void create    (uint32 Width, uint32 Height, uint32 BitDepth, uint32 ChromaFormat);
  void destroy   ();
  bool openFile  (const std::string& FileName, eMode FileMode);
  bool closeFile ();
  bool seekFrame (uint32 FrameNumber);
  bool readFrame (      xPic* Pic);
  bool writeFrame(const xPic* Pic);

public:
  inline int32 getWidth   () const { return m_Width;  }
  inline int32 getHeight  () const { return m_Height; }
  inline int32 getArea    () const { return m_Width * m_Height; }
  inline int32 getBitDepth() const { return m_BitsPerSample; }

protected:
  bool xUnpackFrame(      xPic* Pic);
  bool xPackFrame  (const xPic* Pic);

public:
  static uint32 calcNumFramesInFile(uint32 Width, uint32 Height, uint32 BitDepth, uint32 ChromaFormat, uint64 FileSize);
};

//===============================================================================================================================================================================================================
