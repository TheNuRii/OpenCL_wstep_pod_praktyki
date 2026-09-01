#include "xSSIM_OpenCL.h"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

bool xSSIM_OpenCl::create(int32 Width, int32 Height, int32 Margin, 
    int32 BitDepth, int32 BlockSize, int Step,
    const std::string& KernelFile, cl::Device& Device)
{
    m_Width     = Width;
    m_Height    = Height;
    m_Margin    = Margin;
    m_Stride    = Width + (m_Margin << 1);
    m_BitDepth  = BitDepth;
    m_BlockSize = BlockSize;
    m_Step      = Step;

    m_BlocksWidth  = static_cast<int32>(m_Width  + m_Step - 1) / m_Step;
    m_BlocksHeight = static_cast<int32>(m_Height + m_Step - 1) / m_Step;

    m_BuffCmpNumPels          = (m_Width + (m_Margin << 1)) * (m_Height + (m_Margin << 1));
    m_BuffCmpNumBytes         = m_BuffCmpNumPels * sizeof(uint16);
    m_BuffProcesBlockNumBytes = m_Height * Width * sizeof(flt32);
    m_BuffProcesLineNumBytes  = m_Height * sizeof(flt32);

    m_DynamicRange = (1 << m_BitDepth) - 1;
    m_C1 = (0.01 * m_DynamicRange)*(0.01 * m_DynamicRange); // 0.01 is param k1 form wikipedia
    m_C2 = (0.03 * m_DynamicRange)*(0.03 * m_DynamicRange); // 0.03 is param k2 form wikipedia
    //m_C3 = static_cast<flt32>(m_C2) / 2;  

    m_Device = Device;
    m_Context = cl::Context(m_Device);
    cl_int Result = CL_SUCCESS;

    fmt::printf("Processing kernels.cl\n");
    if (!xFile::exist(KernelFile)) {
        std::filesystem::path CurrentPath = std::filesystem::current_path();
        fmt::printf("ERRORR - kernel file does not exist in this dir: %s | %s",
        CurrentPath, KernelFile);

        return false;
    }

    std::ifstream KernelSourceFile(KernelFile);
    const std::string KernelSource(
        (std::istreambuf_iterator<char>(KernelSourceFile)), std::istreambuf_iterator<char>()
    );

    cl::Program::Sources Sources({KernelSource});
    m_Program = cl::Program(m_Context, Sources, &Result);
    Result = m_Program.build();

    fmt::printf("Build info --> CL_PROGRAM_BUILD_LOG: \n");
    auto BuildLogLines = m_Program.getBuildInfo<CL_PROGRAM_BUILD_LOG>();
    for (auto BuildLogLine : BuildLogLines) {
        fmt::printf(BuildLogLine.second);
    }

    fmt::printf("Build info --> CL_PROGRAM_BUILD_STATUS: \n");
    auto BuildStats = m_Program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>();
    for (auto BuildStatus : BuildStats) {
        fmt::printf("Status = %d\n",BuildStatus.second);
    }

    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - program %d\n", Result);
        return false;
    }

    m_ProcesBlockKernel = cl::Kernel(m_Program, "ProcesBlock", &Result);
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - Kernel ProcesBlock - %d\n", Result);
        return false;
    }

    m_ProcesLineKernel  = cl::Kernel(m_Program, "ProcesLine", &Result);
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - Kernel Proces Line - %d\n", Result);
        return false;
    }

    m_ReduceSumKernel   = cl::Kernel(m_Program, "ReduceSum", &Result);
    if (Result != CL_SUCCESS) {

        fmt::printf("ERROR - Kernel Reduce Sum - %d\n", Result);
        return false;
    }

    m_Queue = cl::CommandQueue(m_Context, m_Device, CL_QUEUE_PROFILING_ENABLE, &Result);
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - ComandQueue - %d\n", Result);
        return false;
    }

    // crate buffer for refernec pic
    for (uint32 c = 0; c < 3; c++) {
        m_BufferRef[c] = cl::Buffer(m_Context, (cl_mem_flags)(CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY), 
        (cl::size_type)m_BuffCmpNumBytes, nullptr, &Result);

        if (Result != CL_SUCCESS) {
            fmt::printf("ERROR - buffer ref - %d\n", Result);
            return false;
        }
    }

    // create buffer for test pic
    for (uint32 c = 0; c < 3; c++) {
        m_BufferTest[c] = cl::Buffer(m_Context, (cl_mem_flags)(CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY),
        (cl::size_type)m_BuffCmpNumBytes, nullptr, &Result);

        if (Result != CL_SUCCESS) {
            fmt::printf("ERROR - buffer test - %d\n", Result);
            return false;
        }
    }

    // Buffors for Accumylating processign Block 
    for (uint32 c = 0; c < 3; c++) {
        m_BufferProcesBlock[c] = cl::Buffer(m_Context, CL_MEM_READ_WRITE, 
        (cl::size_type)m_BuffProcesBlockNumBytes, nullptr, &Result);

        if (Result != CL_SUCCESS) {
            fmt::printf("ERROR - buff proces block - %d\n", Result);
            return false;
        }
    }

    // Buffor for Acumulate procesing line
    for (uint32 c = 0; c < 3; c++) {
        m_BufferProcesLine[c] = cl::Buffer(m_Context, CL_MEM_READ_WRITE,
        (cl::size_type)m_BuffProcesLineNumBytes, nullptr, &Result);
        
        if (Result != CL_SUCCESS) {
            fmt::printf("ERROR - buff proces line - %d\n", Result);
            return false;
        }
    }
    
    for (uint32 c = 0; c < 3; c++) {
        m_BufferReduceSum[c] = cl::Buffer(m_Context, CL_MEM_READ_WRITE,
        sizeof(flt32), nullptr, &Result);

        if (Result != CL_SUCCESS) {
            fmt::printf("ERROR - buffer reduction sum - %d\n", Result);
            return false;
        }
    }

    return true;
}

// -----------------------------------------------------------------------------------------------------------------------
bool xSSIM_OpenCl::xRunReduceSum(flt32* FrameSSIM, uint8 colorSpace) {
    cl_int Result = CL_SUCCESS;
    //cl::Event eventFillBuff;
    cl::Event eventKernelReduceSum;
    cl::Event eventReadBuff;
    cl_long start = 0, end = 0;
    
    //cl_long zero = 0; w sumie nie ma potrzeby zerowania buffora

    cl::Kernel& KernelReduceSum =  m_ReduceSumKernel;
    KernelReduceSum.setArg(0, m_BufferProcesLine[colorSpace]);
    KernelReduceSum.setArg(1, m_BufferReduceSum[colorSpace]);
    KernelReduceSum.setArg(2, static_cast<uint32>(m_BlocksHeight));

    Result = m_Queue.enqueueNDRangeKernel(KernelReduceSum, cl::NullRange, cl::NDRange(1),
            cl::NullRange, nullptr, &eventKernelReduceSum);
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - NDRange Kernel ReduceSum");
        return false;
    }

    eventKernelReduceSum.wait();
    if (Result != CL_SUCCESS) {fmt::printf("ERROR - wait Reduce Sum - %d\n"); return false;}

    start = 0, end = 0;
    eventKernelReduceSum.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    eventKernelReduceSum.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    TimeExecKernelReduceSum += static_cast<double>(end - start) /  1000000.0; // z ns do ms

    Result = m_Queue.enqueueReadBuffer(m_BufferReduceSum[colorSpace], true, 0, 
        sizeof(flt32), FrameSSIM, nullptr, &eventReadBuff);
    eventReadBuff.wait();
    if (Result != CL_SUCCESS) {fmt::printf("ERROR - wait Read buf - %d\n"); return false;}

    eventReadBuff.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    eventReadBuff.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    TimeReadBuff += static_cast<double>(end - start) / 1000000.0; // z ns do ms

    return true;
}

bool xSSIM_OpenCl::xRunProcesLine(uint8 colorSpace) {
    cl_int Result = CL_SUCCESS;
    cl::Event eventKernelProcesLine;
    cl_long start = 0, end = 0;

    cl::Kernel& KernelProcesLine = m_ProcesLineKernel;
    KernelProcesLine.setArg(0, m_BufferProcesBlock[colorSpace]);
    KernelProcesLine.setArg(1, m_BufferProcesLine[colorSpace]);
    KernelProcesLine.setArg(2, static_cast<uint32>(m_BlocksWidth));
    KernelProcesLine.setArg(3, static_cast<uint32>(m_BlocksHeight));

    Result = m_Queue.enqueueNDRangeKernel(KernelProcesLine, cl::NullRange,
    cl::NDRange(m_BlocksHeight), cl::NullRange, nullptr, &eventKernelProcesLine);
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - NDRangeKernel ProceLine - %d\n", Result);
        return false;
    }

    eventKernelProcesLine.wait();
    if (Result != CL_SUCCESS) { fmt::printf("ERROR - wait Proces Line - %d\n", Result); return false;}

    start = 0, end = 0;
    eventKernelProcesLine.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    eventKernelProcesLine.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    TimeExecKernelProcesLine += static_cast<double>(end - start) / 1000000.0; // z ns do ms

    return true;
}

bool xSSIM_OpenCl::xRunProcesBlock(xPic& PicRef, xPic& PicTest, uint8 colorSpace) {
    cl_int Result = CL_SUCCESS;
    cl::Event eventWriteRefBuff;
    cl::Event eventWrietTestBuff;
    cl::Event eventKernelProcesBlock;
    cl_ulong start = 0, end = 0;

    Result = m_Queue.enqueueWriteBuffer(m_BufferRef[colorSpace], false, 0, 
        m_BuffCmpNumBytes, PicRef.getBuffer(colorSpace), nullptr, &eventWriteRefBuff);
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - enequue Wriet Buffer Ref - %d\n", Result);
        return false;
    }

    eventWriteRefBuff.wait();
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - wait - %d\n", Result); return false;}

    eventWriteRefBuff.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    eventWriteRefBuff.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    TimeCopyBuff += static_cast<double>(end - start) / 1000000.0; // z ns do ms
    
    Result = m_Queue.enqueueWriteBuffer(m_BufferTest[colorSpace], false, 0, m_BuffCmpNumBytes,
    PicTest.getBuffer(colorSpace), nullptr, &eventWrietTestBuff);
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - enqueue Write Buffer Test - %d\n", Result);
        return false;
    }

    eventWrietTestBuff.wait();
    if (Result != CL_SUCCESS) { fmt::printf("ERROR - wait - %d\n", Result); return false;}

    start = 0, end = 0;
    eventWrietTestBuff.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    eventWrietTestBuff.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    TimeCopyBuff += static_cast<double>(end - start) / 1000000.0; // z ns do ms

    cl::Kernel& ProcesBlockKernel = m_ProcesBlockKernel;
    ProcesBlockKernel.setArg(0, m_BufferRef[colorSpace]);
    ProcesBlockKernel.setArg(1, m_BufferTest[colorSpace]);
    ProcesBlockKernel.setArg(2, m_BufferProcesBlock[colorSpace]);
    ProcesBlockKernel.setArg(3, static_cast<uint32>(m_BlockSize));
    ProcesBlockKernel.setArg(4, static_cast<uint32>(m_Step));
    ProcesBlockKernel.setArg(5, static_cast<uint32>(m_Width));
    ProcesBlockKernel.setArg(6, static_cast<uint32>(m_Height));
    ProcesBlockKernel.setArg(7, static_cast<uint32>(m_Margin));
    ProcesBlockKernel.setArg(8, static_cast<uint32>(m_Stride));
    ProcesBlockKernel.setArg(9, static_cast<flt32>(m_C1));
    ProcesBlockKernel.setArg(10, static_cast<flt32>(m_C2));
    

    Result = m_Queue.enqueueNDRangeKernel(ProcesBlockKernel, cl::NullRange,
    cl::NDRange(m_BlocksWidth, m_BlocksHeight), cl::NullRange,
    nullptr, &eventKernelProcesBlock);
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - enequeue NDRange Kernel Proces Block - %d\n", Result);
        return false;
    }

    eventKernelProcesBlock.wait();
    if (Result != CL_SUCCESS) { fmt::printf("ERROR - wait Kernel Proces Block - %d\n", Result); return false;}

    start = 0, end = 0;
    eventKernelProcesBlock.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
    eventKernelProcesBlock.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
    TimeExecKernelProcesBlock += static_cast<double>(end - start) / 1000000.0; // z ns do ms

    return true;
}

// -----------------------------------------------------------------------------------------------------------------------

bool xSSIM_OpenCl::processFrame(xPic& PicRef, xPic& PicTest) {
    const int32 Radius = m_BlockSize / 2;

    if (m_Margin < Radius) { fmt::printf("ERROR - Margin < Radius \n"); return false;}
    for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
        flt32 FrameSSIM = 0;

        if (!xRunProcesBlock(PicRef, PicTest, CmpIdx)) {return false;}
 
        if (!xRunProcesLine(CmpIdx)) {return false;}

        if (!xRunReduceSum(&FrameSSIM, CmpIdx)) {return false;}

        SSIMResultGPU[CmpIdx] = FrameSSIM / static_cast<flt32>(m_BlocksWidth * m_BlocksHeight);
        SSIMSumGPU[CmpIdx]   += SSIMResultGPU[CmpIdx];
    }

    fmt::printf(
        "| GPU: LM %8.4f | CB %8.4f | CR %8.4f\n",
        SSIMResultGPU[0],
        SSIMResultGPU[1],
        SSIMResultGPU[2]
    );

    return true;
}
