#include "xPSNR_OpenCL.h"
#include "CommonDef.h"
#include "lib_fmt/printf.h"
#include "xFile.h"
#include "xPic.h"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>

bool xPSNR_OpenCL::create(int32 Width, int32 Height, int32 Margin, 
    const std::string& KernelFile, cl::Device& Device){
    
    m_Width  = Width;
    m_Height = Height;
    m_Margin = Margin;
    m_Stride = Width + (m_Margin << 1);

    m_BuffCmpNumBytes         = (size_t)m_Width * m_Height * sizeof(uint16);
    m_BuffSqrDiffNumBytes     = (size_t)m_Width * m_Height * sizeof(uint64);

    m_Device = Device;
    // crate context
    m_Context = cl::Context(m_Device);

    // compile kernel(s)
    cl_int Result = CL_SUCCESS;

    fmt::printf("Processing kernels.cl\n");
    if (!xFile::exist(KernelFile)) {
        fmt::printf("ERROR - kernel file does not exist - %s\n", KernelFile);
        return false; 
    }

    std::ifstream KernelSourceFile(KernelFile);
    // TO DO: przeanalizować 
    const std::string KernelSource(
        (std::istreambuf_iterator<char>(KernelSourceFile)), std::istreambuf_iterator<char>()
    );

    cl::Program::Sources Sources({KernelSource});
    m_Program = cl::Program(m_Context, Sources, &Result);
    Result = m_Program.build();

    //logi przy budowie
    fmt::printf("Build info --> CL_PRORAM_BUILD_LOG: \n");
    auto BuildLogLines = m_Program.getBuildInfo<CL_PROGRAM_BUILD_LOG>();
    for (auto BuildLogLine : BuildLogLines) {
        fmt::printf(BuildLogLine.second);
    }

    // status budowy programu
    fmt::printf("Build info --> CL_PROGRAM_BUILD_STATUS:\n");
    auto BuildStats = m_Program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>();
    for (auto BuidStatus : BuildStats) {
        fmt::printf("Status = %d\n", BuidStatus.second);
    }
    
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - program - %d\n", Result);
        return false;
    }


    m_Kernels[(int32)eKernelOp::SQRDIFF] = cl::Kernel(m_Program, "PsnrPartialSum", &Result);
    if (Result != CL_SUCCESS) { 
        fmt::printf("ERROR - kernel PsnrPartialSum - %d\n", Result);
        return false;
    }

    m_Kernels[(int32)eKernelOp::SDD]     = cl::Kernel(m_Program, "PsnrReduceSum", &Result);
    if (Result != CL_SUCCESS) { 
        fmt::printf("ERROR - kernel PsnrReduceSum - %d\n", Result);
        return false;
    }

    // create command queue
    m_Queue = cl::CommandQueue(m_Context, m_Device, 0, &Result);
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - queue = %d\n", Result);
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

    // crate buffer for accumulated pow2(ref - tets)
    for (uint32 c = 0; c < 3; c++) {
        m_BufferSqrDiff[c] = cl::Buffer(m_Context, CL_MEM_READ_WRITE, // wpisujemy do bufora i oczytyjemy przy ToTal PSNR
        (cl::size_type)m_BuffSqrDiffNumBytes , nullptr, &Result);

        if (Result != CL_SUCCESS) {
            fmt::printf("ERROR - buffer sqrdif - %d\n", Result);
            return false;
        }
    }
    
    // osteczne bufory wyjsciowe
    for (uint32 c = 0; c < 3;c++) {
        m_BufferTotalDiff[c] = cl::Buffer(m_Context, CL_MEM_READ_WRITE, sizeof(cl_ulong), nullptr, &Result);
        if (Result != CL_SUCCESS) {
            fmt::printf("ERROR - buffer totaldiff - %d\n", Result);
            return false;
        }
    }

    double sumSqaredErrorLm, sumSqaredErrorCr, sumSqaredErrorCb;
    sumSqaredErrorCb = sumSqaredErrorCr = sumSqaredErrorLm = 0.0;
    
    return true;
}

// =================================================================================================================================================================================================================
bool xPSNR_OpenCL::xRunSquaredDiff(xPic& Ref, xPic& Test) {
    cl_int Result = CL_SUCCESS;
    tTimePoint T0 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    for (uint32 c = 0; c < 3; c++) {
        Result = m_Queue.enqueueWriteBuffer(m_BufferRef[c], false, 0, m_BuffCmpNumBytes, Ref.getBuffer(c));
        if (Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueWriteBuffer Ref - %d\n", Result); return false; }
    }
    for (uint32 c = 0; c < 3; c++) {
        Result = m_Queue.enqueueWriteBuffer(m_BufferTest[c], false, 0, m_BuffCmpNumBytes, Test.getBuffer(c));
        if (Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueWriteBuffer Test - %d\n", Result); return false; }
    }

    tTimePoint T1 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    cl::Kernel& K = m_Kernels[(int32)eKernelOp::SQRDIFF];
    K.setArg(0, m_BufferRef[0]);
    K.setArg(1, m_BufferRef[1]);
    K.setArg(2, m_BufferRef[2]);
    K.setArg(3, m_BufferTest[0]);
    K.setArg(4, m_BufferTest[1]);
    K.setArg(5, m_BufferTest[2]);
    K.setArg(6, m_BufferSqrDiff[0]);
    K.setArg(7, m_BufferSqrDiff[1]);
    K.setArg(8, m_BufferSqrDiff[2]);
    K.setArg(9,  m_Width);
    K.setArg(10, m_Height);
    K.setArg(11, m_Margin);
    K.setArg(12, m_Stride);

    Result = m_Queue.enqueueNDRangeKernel(K, cl::NullRange, cl::NDRange(m_Width, m_Height), cl::NullRange);
    if (Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueNDRangeKernel SQRDIFF - %d\n", Result); return false; }

    tTimePoint T2 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();
    m_Queue.finish();

    if (m_VerboseLevel >= 3) {
        DurationWriteBuff  += T1 - T0;
        DurationExecKernel += T2 - T1;
    }
    return true;
}

bool xPSNR_OpenCL::xRunReduceSum(uint64* SqrDiffLm, uint64* SqrDiffCb, uint64* SqrDiffCr) {
    cl_int Result = CL_SUCCESS;

    cl_ulong zero = 0;
    for (uint32 c = 0; c < 3; c++) {
        m_Queue.enqueueFillBuffer(m_BufferTotalDiff[c], zero, 0, sizeof(cl_ulong));
    }

    cl::Kernel& K = m_Kernels[(int32)eKernelOp::SDD];
    K.setArg(0, m_BufferSqrDiff[0]);
    K.setArg(1, m_BufferSqrDiff[1]);
    K.setArg(2, m_BufferSqrDiff[2]);
    K.setArg(3, m_BufferTotalDiff[0]);
    K.setArg(4, m_BufferTotalDiff[1]);
    K.setArg(5, m_BufferTotalDiff[2]);
    K.setArg(6, m_Width);
    K.setArg(7, m_Height);
    K.setArg(8, m_Margin);
    K.setArg(9, m_Stride);

    Result = m_Queue.enqueueNDRangeKernel(K, cl::NullRange, cl::NDRange(m_Width, m_Height), cl::NullRange);
    if (Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueNDRangeKernel SDD - %d\n", Result); return false; }

    tTimePoint T2 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    m_Queue.enqueueReadBuffer(m_BufferTotalDiff[0], true, 0, sizeof(cl_ulong), SqrDiffLm);
    m_Queue.enqueueReadBuffer(m_BufferTotalDiff[1], true, 0, sizeof(cl_ulong), SqrDiffCb);
    m_Queue.enqueueReadBuffer(m_BufferTotalDiff[2], true, 0, sizeof(cl_ulong), SqrDiffCr);

    tTimePoint T3 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();
    if (m_VerboseLevel >= 3) { DurationReadBuff += T3 - T2; }

    return true;
}
// =================================================================================================================================================================================================================

bool xPSNR_OpenCL::processFrame(xPic& Ref, xPic& Test) {
    if (!xRunSquaredDiff(Ref, Test)) {return false;}

    uint64* SqrDiffLm = 0;
    uint64* SqrDiffCb = 0; 
    uint64* SqrDiffCr = 0;
    if(!xRunReduceSum(SqrDiffLm, SqrDiffCb, SqrDiffCr)) {return false;}

    printPSNRStats(SqrDiffLm, 1);
    printPSNRStats(SqrDiffCb, 1);
    printPSNRStats(SqrDiffCr, 1);

    // prep buff to acumulet Squared Diff another frame
    cl_ulong zero = 0;
    for (uint32 c = 0; c < 3; c++) {
        m_Queue.enqueueFillBuffer(m_BufferTotalDiff[c], zero, 0, sizeof(cl_ulong));
    }

    return true;
}

// =================================================================================================================================================================================================================
void xPSNR_OpenCL::printPSNRStats(uint64* SqrDiff, int32 NumFrames) {
    const double mse    = (double)*SqrDiff /  (double)((uint64)m_Width * m_Height * NumFrames);
    const double maxVal = (double)((1u << 10) - 1);
    const double psnr   = (mse > 0.0) ? 10.0 * std::log10((maxVal * maxVal) / mse) 
                                        : std::numeric_limits<double>::infinity();
    
    fmt::printf("PSNR = %f dB\n", psnr);
}

void xPSNR_OpenCL::printTimeStats(int32 NumFrames)
{
  fmt::printf("AvgTime WriteBuff  %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationWriteBuff ).count() / NumFrames);
  fmt::printf("AvgTime ExecKernel %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationExecKernel).count() / NumFrames);
  fmt::printf("AvgTime ReadBuff   %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationReadBuff  ).count() / NumFrames);
}

// =================================================================================================================================================================================================================
/*
bool xPSNR_OpenCL::xRunKernel(xPic& Ref, xPic& Test, uint64& SqrDiff, eKernelOp KernelOp) {
    cl_int Result = CL_SUCCESS;

    tTimePoint T0 = (m_VerboseLevel >=3) ? tClock::now() : tTimePoint::min();

    // TO DO: impement copy datda to buffors
    for (uint32 c = 0; c < 3; c++) {
        Result = m_Queue.enqueueWriteBuffer(m_BufferRef[c], false, 0, 
            m_BuffCmpNumBytes, Ref.getBuffer(c));
        
        if (Result != CL_SUCCESS) {
            fmt::printf("ERROR - enqueueWriteBuffer Ref - %d\n", Result);
            return false;
        }
    }

    for (uint32 c = 0; c < 3; c++) {
        Result = m_Queue.enqueueWriteBuffer(m_BufferTest[c], false, 0, 
            m_BuffCmpNumBytes, Test.getBuffer(c));
        
        if (Result != CL_SUCCESS) {
            fmt::printf("ERROR - enqueueWriteBuffer Test - %d\n", Result);
            return false;
        }
    }

    tTimePoint T1 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();
    
    cl::Kernel& SelectedKernel = m_Kernels[(int32)KernelOp];

    // seting kernel args
    Result = SelectedKernel.setArg(0, m_BufferRef[0]);      // __global const uint16* restrict RefLm
    Result = SelectedKernel.setArg(1, m_BufferRef[1]);      // __global const uint16* restrict RefCb   
    Result = SelectedKernel.setArg(2, m_BufferRef[2]);      // __global const uint16* restrict RefCr
    
    Result = SelectedKernel.setArg(3, m_BufferTest[0]);     // __global const uint16* restrict TestLm
    Result = SelectedKernel.setArg(4, m_BufferTest[1]);     // __global const uint16* restrict TestCb
    Result = SelectedKernel.setArg(5, m_BufferTest[2]);     // __global const uint16* restrict TestCr
    
    Result = SelectedKernel.setArg(6, m_BufferSqrDiff[0]);  // __global       unit64*  restrict SqrDiffLm
    Result = SelectedKernel.setArg(7, m_BufferSqrDiff[1]);  // __global       unit64*  restrict SqrDiffCb
    Result = SelectedKernel.setArg(8, m_BufferSqrDiff[2]);  // __global       unit64*  restrict SqrDiffCr

    Result = SelectedKernel.setArg(9, m_Width);             // const int Width
    Result = SelectedKernel.setArg(10, m_Height);           // const int Height
    Result = SelectedKernel.setArg(11, m_Margin);           // const int Margin
    Result = SelectedKernel.setArg(12, m_Stride);           // const int Stride

    // execute kernel
    Result = m_Queue.enqueueNDRangeKernel(SelectedKernel, cl::NullRange, 
        cl::NDRange(m_Width, m_Height), cl::NullRange, nullptr, nullptr);
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - enqueueNDRangeKernel - %d\n", Result);
        return false;
    }

    Result = m_Queue.finish();
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - finsh - %d\n", Result);
        return false;
    }

    tTimePoint T2 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    // Reading form buffers
    for (uint32 c = 0; c < 3; c++) {
        Result = m_Queue.enqueueReadBuffer(m_BufferSqrDiff[c], false, 0, 
            m_BuffCmpNumBytes, &SqrDiff);
    }

    Result = m_Queue.finish();
    if (Result != CL_SUCCESS) {
        fmt::printf("ERROR - finsh - %d\n", Result);
        return false;
    }
    return true;
}

*/