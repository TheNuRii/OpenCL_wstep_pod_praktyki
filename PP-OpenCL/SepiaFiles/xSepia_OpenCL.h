#pragma once
#include "../CommonDef.h"
#include "../xOpenCL_Common.h"
#include "xSepia.h"

//===============================================================================================================================================================================================================

class xSepia_OpenCL : public xSepiaBase
{
public:
  enum class eCopyMode
  {
    FullCopy,
    ZeroCopy,
  };

  enum class eKernelOp
  {
    Copy          = 0,
    YUVtoRGBtoYUV = 1,
    Sepia         = 2,
  };

protected:
  int32 m_Width  = NOT_VALID;
  int32 m_Height = NOT_VALID;
  int32 m_Margin = NOT_VALID;
  int32 m_Stride = NOT_VALID;

  int32 m_BuffCmpNumPels  = NOT_VALID;
  int32 m_BuffCmpNumBytes = NOT_VALID;  

  cl::Device       m_Device;
  cl::Context      m_Context;
  cl::Program      m_Program;
  cl::Kernel       m_Kernels[3];
  cl::CommandQueue m_Queue;

  eCopyMode   m_CopyMode;
  cl::Buffer  m_BufferSrc[3];
  cl::Buffer  m_BufferDst[3];

  tDuration DurationWriteBuff  = tDuration(0);
  tDuration DurationExecKernel = tDuration(0);
  tDuration DurationReadBuff   = tDuration(0);

public:
  bool create (int32 Width, int32 Height, int32 Margin, const std::string& KernelsFile, cl::Device& Device, eCopyMode CopyMode);

  bool testCopyContent  (xPic& Dst, const xPic& Src) { return xRunOneKernel(Dst, Src, eKernelOp::Copy         ); }
  bool testYUVtoRGBtoYUV(xPic& Dst, const xPic& Src) { return xRunOneKernel(Dst, Src, eKernelOp::YUVtoRGBtoYUV); }
  bool applySepiaEffect (xPic& Dst, const xPic& Src) { return xRunOneKernel(Dst, Src, eKernelOp::Sepia        ); }

  void printTimeStats(int32 NumFrames);

protected:
  bool xRunOneKernel(xPic& Dst, const xPic& Src, eKernelOp KernelOp);
};

//===============================================================================================================================================================================================================
