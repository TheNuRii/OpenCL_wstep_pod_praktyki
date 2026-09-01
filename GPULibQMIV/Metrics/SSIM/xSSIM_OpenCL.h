#pragma once
#include "xSSIM.h"
#include <CL/opencl.hpp>
#include <array>
#include <string>
#include "../../xFile.h"
#include "filesystem"

class xSSIM_OpenCl : xSSIM {
protected:
    std::array<flt32, 3> SSIMResultGPU;
    std::array<flt64, 3> SSIMSumGPU = {0.0, 0.0, 0.0}; 

    uint32 m_BuffCmpNumBytes         = NOT_VALID;
    uint32 m_BuffCmpNumPels          = NOT_VALID;
    uint32 m_BuffProcesLineNumBytes  = NOT_VALID;
    uint32 m_BuffProcesBlockNumBytes = NOT_VALID;

    cl::Device       m_Device;
    cl::Context      m_Context;
    cl::Program      m_Program;
    cl::Kernel       m_ProcesBlockKernel;
    cl::Kernel       m_ProcesLineKernel;
    cl::Kernel       m_ReduceSumKernel;
    cl::CommandQueue m_Queue;

    cl::Buffer m_BufferRef[3];
    cl::Buffer m_BufferTest[3];
    cl::Buffer m_BufferProcesBlock[3];
    cl::Buffer m_BufferProcesLine[3];
    cl::Buffer m_BufferReduceSum[3];

    double  TimeCopyBuff                = 0;
    double  TimeExecKernelProcesBlock   = 0;
    double  TimeExecKernelProcesLine    = 0;
    double  TimeExecKernelReduceSum     = 0;
    double  TimeReadBuff                = 0;

public:

    bool create(int32 Width, int32 Height, int32 Margin, 
        int32 BitDepth, int32 BlockSize, int32 Step,
        const std::string& KernelsFile, cl::Device& Device);

    bool processFrame(xPic& PicRef, xPic& PicTest);

    bool xRunProcesBlock(xPic& PicRef, xPic& PicTest, uint8 colorSpace);
    bool xRunProcesLine(uint8 colorSpace);
    bool xRunReduceSum(flt32* FrameSSIM, uint8 colorSpace);

    flt32 getAvgSSIM(int32 CmpIdx, int32 NumFrames) const { 
        return static_cast<flt32>(SSIMSumGPU[CmpIdx] / NumFrames); }

    flt64 getAvgTimeCopyBuff(int32 NumFrames)              const { return TimeCopyBuff / NumFrames; }
    flt64 getAvgTimeReadBuff(int32 NumFrames)              const { return TimeReadBuff / NumFrames; }
    flt64 getAvgTimeExecKernelProcesBlock(int32 NumFrames) const { return TimeExecKernelProcesBlock / NumFrames; }
    flt64 getAvgTimeExecKernelProcesLine(int32 NumFrames)  const { return TimeExecKernelProcesLine / NumFrames; }
    flt64 getAvgTimeExecKernelReduceSum(int32 NumFrames)   const { return TimeExecKernelReduceSum / NumFrames; }
};