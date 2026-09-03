#pragma once
#include "../../CommonDef.h"
#include "../../xOpenCL_Common.h"
#include <CL/opencl.hpp>
#include <array>
#include "../../xPic.h"
#include "xPSNR.h"
//===============================================================================================================================================================================================================

class xPSNR_OpenCL : xPSNR{
public:
    enum class eCopyMode {
        FullCopy,
    };

    enum class eKernelOp {
        SSDSQRDIFF = 0,
        SSDREDUCESUM = 1,
    };

    enum class colorSpace {
        LM = 0,
        CB = 1,
        CR = 2,
    };
        
protected:
    std::array<flt64, 3> PSNRResultGPU;
    std::array<flt64, 3> PSNRSumGPU = {0.0, 0.0, 0.0};

    int64 m_BuffSqrDiffNumBytes  = NOT_VALID;
    int32 m_BuffCmpNumBytes = NOT_VALID;
    int32 m_BuffCmpNumPels = NOT_VALID;

    cl::Device       m_Device;
    cl::Context      m_Context;
    cl::Program      m_Program;
    cl::Kernel       m_Kernels[2];
    cl::CommandQueue m_Queue;

    cl::Buffer m_BufferRef[3];
    cl::Buffer m_BufferTest[3];
    cl::Buffer m_BufferSqrDiff[3];
    cl::Buffer m_BufferTotalDiff[3];

    flt64     TimeCopyBuff          = 0;
    flt64     TimeExecKernelSqrDiff = 0;
    flt64     TimeExecKernelReduce  = 0;
    flt64     TimeReadBuff          = 0;
    //flt64     TimeFillBuff          = 0; 


    tDuration DurationWriteBuff  = tDuration(0);
    tDuration DurationExecKernel = tDuration(0);
    tDuration DurationReadBuff   = tDuration(0);
public:
    bool create(int32 Width, int32 Height, int32 Margin, int32 BitDepth,
        const std::string& KernelsFile, cl::Device& Device);

    bool processFrame(xPic& Ref, xPic& Test);

    // Getery do analizy w glowny programie 
    flt64 getAvgPSNR(int32 CmpIdx, int32 NumFrames) const { 
        return static_cast<flt64>(PSNRSumGPU[CmpIdx] / NumFrames); }

    flt64 getAvgTimeCopyBuff(int32 NumFrames)              const { return TimeCopyBuff / NumFrames; }
    flt64 getAvgTimeReadBuff(int32 NumFrames)              const { return TimeReadBuff / NumFrames; }
    flt64 getAvgTimeExecKernelReduce (int32 NumFrames)     const { return TimeExecKernelReduce  / NumFrames; }
    flt64 getAvgTimeExecKernelSqrDiff(int32 NumFrames)     const { return TimeExecKernelSqrDiff / NumFrames; }

protected:
    bool xRunSquaredDiff(xPic& Ref, xPic& Test, uint8_t colorSpace);
    bool xRunReduceSum(uint64* SqrtDiff, uint8_t colorSpace);
};