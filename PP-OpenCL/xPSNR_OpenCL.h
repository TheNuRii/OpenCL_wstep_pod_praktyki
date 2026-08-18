#pragma once
#include "CommonDef.h"
#include "xOpenCL_Common.h"
#include <CL/opencl.hpp>
#include "xPic.h"
//===============================================================================================================================================================================================================

class xPSNR_OpenCL {
public:
    enum class eCopyMode {
        FullCopy,
    };

    enum class eKernelOp {
        SSDSQRDIFF = 1,
        SSDREDUCESUM = 2,
    };

    enum class colorSpace {
        LM = 0,
        CB = 1,
        CR = 2,
    };

protected:
    int32 m_Width  = NOT_VALID;
    int32 m_Height = NOT_VALID;
    int32 m_Margin = NOT_VALID;
    int32 m_Stride = NOT_VALID;
    int32 m_BitDepth = NOT_VALID;

    int64 m_BuffSqrDiffNumBytes  = NOT_VALID;
    int32 m_BuffCmpNumBytes = NOT_VALID;

    cl::Device       m_Device;
    cl::Context      m_Context;
    cl::Program      m_Program;
    cl::Kernel       m_Kernels[2];
    cl::CommandQueue m_Queue;

    //eCopyMode  m_CopyMode;
    cl::Buffer m_BufferRef[3];
    cl::Buffer m_BufferTest[3];
    cl::Buffer m_BufferSqrDiff[3];
    cl::Buffer m_BufferTotalDiff[3];

    float AvgtotalSqrDiffLm = 0;
    float AvgtotalSqrDiffCb = 0;
    float AvgtotalSqrDiffCr = 0;

    tDuration DurationWriteBuff  = tDuration(0);
    tDuration DurationExecKernel = tDuration(0);
    tDuration DurationReadBuff   = tDuration(0);

protected:
    int32        m_VerboseLevel  = 0;
    const int32  m_NumComponents = 3;

public:
    bool create(int32 Width, int32 Height, int32 Margin, int32 BitDepth,
        const std::string& KernelsFile, cl::Device& Device);
    
    void printTimeStats(int32  NumFrames);
    void printPSNRStats(uint64 SqrDiff);
    void printAvgPNSRStats(uint32_t NumFrames);
    bool processFrame(xPic& Ref, xPic& Test);
    
protected:
    //bool xRunKernel(xPic& Ref, xPic& Test, uint64& SqrDiff, eKernelOp KernelOp);
    bool xRunSquaredDiff(xPic& Ref, xPic& Test, uint8_t colorSpace);
    bool xRunReduceSum(uint64* SqrtDiff, uint8_t colorSpace);
};