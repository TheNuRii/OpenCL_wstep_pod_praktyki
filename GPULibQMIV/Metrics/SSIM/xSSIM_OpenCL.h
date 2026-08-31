#pragma once
#include "xSSIM.h"
#include <CL/opencl.hpp>
#include <array>
#include <string>
#include "../../xFile.h"
#include "filesystem"

class xSSIM_OpenCl : xSSIM {
public:
    std::array<flt32, 3> SSIMResultGPU;
protected:
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
    //double  TimeFillBuff                = 0;
public:

    bool create(int32 Width, int32 Height, int32 Margin, int32 BitDepth, int32 BlockSize,
        const std::string& KernelsFile, cl::Device& Device);

    bool processFrame(xPic& PicRef, xPic& PicTest);

    bool xRunProcesBlock(xPic& PicRef, xPic& PicTest, uint8 colorSpace);
    bool xRunProcesLine(uint8 colorSpace);
    bool xRunReduceSum(flt32* FrameSSIM, uint8 colorSpace);

    double getTimeCopyBuff() {return TimeCopyBuff;}
    double getTimeReadBuff() {return TimeReadBuff;}
    double getTimeExecKernelProcesBlock() {return TimeExecKernelProcesBlock;}
    double getTimeExecKernelProcesLine()  {return TimeExecKernelProcesLine;}
    double getTimeExecKernelReduceSum()    {return TimeExecKernelReduceSum;}
};