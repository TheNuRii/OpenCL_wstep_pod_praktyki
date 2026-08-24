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
    
    flt64 GpuResultPSNRLm = 0;
    flt64 GpuResultPSNRCb = 0;
    flt64 GpuResultPSNRCr = 0;
    
protected:
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
    flt64     TimeFillBuff          = 0; 


    tDuration DurationWriteBuff  = tDuration(0);
    tDuration DurationExecKernel = tDuration(0);
    tDuration DurationReadBuff   = tDuration(0);
public:
    bool create(int32 Width, int32 Height, int32 Margin, int32 BitDepth,
        const std::string& KernelsFile, cl::Device& Device);
    
    void printTimeStats(int32  NumFrames);
    void printPSNRStats(uint64 SSD, uint8 colorSpace);
    void printAvgPNSRStats(uint32 NumFrames);
    bool processFrame(xPic& Ref, xPic& Test);
    void gpuAvgPSNR(uint32 NumFrames);

    // Getery do analizy w glowny programie 
    flt64 getGpuResultPSNRLm()  { return GpuResultPSNRLm; }
    flt64 getGpuResultPSNRCb()  { return GpuResultPSNRCb; }
    flt64 getGpuResultPSNRCr()  { return GpuResultPSNRCr; }

    flt64 getTimeCopyBuff()    { return TimeCopyBuff; }
    flt64 getTimeExecKernelSqrDiff()   { return TimeExecKernelSqrDiff; }
    flt64 getTimeExecKernelReduce()   { return TimeExecKernelReduce; }
    flt64 getTimeReadBuff()     { return TimeReadBuff; }
    flt64 getTimeFillBuff()     { return TimeFillBuff; }

protected:
    bool xRunSquaredDiff(xPic& Ref, xPic& Test, uint8_t colorSpace);
    bool xRunReduceSum(uint64* SqrtDiff, uint8_t colorSpace);
};