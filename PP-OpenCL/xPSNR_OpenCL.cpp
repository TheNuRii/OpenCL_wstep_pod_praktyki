#include "xPSNR_OpenCL.h"
#include "CommonDef.h"
#include "lib_fmt/core.h"
#include "lib_fmt/printf.h"
#include "xFile.h"
#include "xPic.h"
#include <CL/cl.h>
#include <CL/cl_platform.h>
#include <CL/opencl.hpp>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <vector>

bool xPSNR_OpenCL::create(int32 Width, int32 Height, int32 Margin, int32 BitDepth,
    const std::string& KernelFile, cl::Device& Device){
    
    m_Width  = Width;
    m_Height = Height;
    m_Margin = Margin;
    m_Stride = Width + (m_Margin << 1);
    m_BitDepth = BitDepth;

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


    m_Kernels[(int32)eKernelOp::SSDSQRDIFF] = cl::Kernel(m_Program, "SSDPartialSum", &Result);
    if (Result != CL_SUCCESS) { 
        fmt::printf("ERROR - kernel PartialSum - %d\n", Result);
        return false;
    }

    m_Kernels[(int32)eKernelOp::SSDREDUCESUM]     = cl::Kernel(m_Program, "SSDReduceSum", &Result);
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
    
    return true;
}

// =================================================================================================================================================================================================================
bool xPSNR_OpenCL::xRunSquaredDiff(xPic& Ref, xPic& Test, uint8_t colorSpce) {
    cl_int Result = CL_SUCCESS;
    tTimePoint T0 = (m_VerboseLevel >= 1) ? tClock::now() : tTimePoint::min();

    Result = m_Queue.enqueueWriteBuffer(m_BufferRef[colorSpce], false, 0, m_BuffCmpNumBytes, Ref.getBuffer(colorSpce));
    if (Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueWriteBuffer Ref - %d\n", Result); return false; }

    Result = m_Queue.enqueueWriteBuffer(m_BufferTest[colorSpce], false, 0, m_BuffCmpNumBytes, Test.getBuffer(colorSpce));
    if (Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueWriteBuffer Test - %d\n", Result); return false; }

    tTimePoint T1 = (m_VerboseLevel >= 1) ? tClock::now() : tTimePoint::min();

    cl::Kernel& K = m_Kernels[(uint32_t)eKernelOp::SSDSQRDIFF];
    K.setArg(0, m_BufferRef[colorSpce]);
    K.setArg(1, m_BufferTest[colorSpce]);
    K.setArg(2, m_BufferSqrDiff[colorSpce]);
    K.setArg(3,  m_Width);

    Result = m_Queue.enqueueNDRangeKernel(K, cl::NullRange, cl::NDRange(m_Height), cl::NullRange);
    if (Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueNDRangeKernel SQRDIFF - %d\n", Result); return false; }

    tTimePoint T2 = (m_VerboseLevel >= 1) ? tClock::now() : tTimePoint::min();
    m_Queue.finish();

    if (m_VerboseLevel >= 1) {
        DurationWriteBuff  += T1 - T0;
        DurationExecKernel += T2 - T1;
    }
    return true;
}

bool xPSNR_OpenCL::xRunReduceSum(uint64* SqrDiff, uint8_t colorSpace) {
    cl_int Result = CL_SUCCESS;

    cl_ulong zero = 0;
    m_Queue.enqueueFillBuffer(m_BufferTotalDiff[colorSpace], zero, 0, sizeof(cl_ulong));


    cl::Kernel& K = m_Kernels[(int32)eKernelOp::SSDREDUCESUM];
    K.setArg(0, m_BufferSqrDiff[colorSpace]);
    K.setArg(1, m_BufferTotalDiff[colorSpace]);
    K.setArg(2, (uint32_t)m_Height);

    Result = m_Queue.enqueueNDRangeKernel(K, cl::NullRange, cl::NDRange(1), cl::NullRange);
    if (Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueNDRangeKernel SDD - %d\n", Result); return false; }

    tTimePoint T2 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    m_Queue.enqueueReadBuffer(m_BufferTotalDiff[colorSpace], true, 0, sizeof(cl_ulong), SqrDiff);

    tTimePoint T3 = (m_VerboseLevel >= 1) ? tClock::now() : tTimePoint::min();
    if (m_VerboseLevel >= 1) { DurationReadBuff += T3 - T2; }

    return true;
}
// =================================================================================================================================================================================================================

bool xPSNR_OpenCL::processFrame(xPic& Ref, xPic& Test) {
    if (!xRunSquaredDiff(Ref, Test, (uint8_t)colorSpace::LM)) {return false;}
    if (!xRunSquaredDiff(Ref, Test, (uint8_t)colorSpace::CB)) {return false;}
    if (!xRunSquaredDiff(Ref, Test, (uint8_t)colorSpace::CR)) {return false;}
    

    //uint64_t sumRef = 0, sumTest = 0;
    //const ushort* refData  = (const ushort*)Ref.getBuffer((uint8_t)colorSpace::LM);
    //const ushort* testData = (const ushort*)Test.getBuffer((uint8_t)colorSpace::LM);
    //for (size_t i = 0; i < m_Width * m_Height; i++) { sumRef += refData[i]; sumTest += testData[i]; }
    //fmt::printf("sumRef=%llu sumTest=%llu\n", sumRef, sumTest);
    //uint64* SqrDiffLm = 0;
    //uint64* SqrDiffCb = 0; 
    //uint64* SqrDiffCr = 0;
    //if(!xRunReduceSum(SqrDiffLm, (uint8_t)colorSpace::LM)) {return false;}

    std::vector<cl_ulong>  rowSumsLM(m_Height);
    std::vector<cl_ulong>  rowSumsCB(m_Height);
    std::vector<cl_ulong>  rowSumsCR(m_Height);
    m_Queue.enqueueReadBuffer(m_BufferSqrDiff[(uint8_t)colorSpace::LM], true, 0, 
                            sizeof(cl_long)*m_Height, rowSumsLM.data());

    m_Queue.enqueueReadBuffer(m_BufferSqrDiff[(uint8_t)colorSpace::CB], true, 0, 
                            sizeof(cl_long)*m_Height, rowSumsCB.data());

    m_Queue.enqueueReadBuffer(m_BufferSqrDiff[(uint8_t)colorSpace::CR], true, 0, 
                            sizeof(cl_long)*m_Height, rowSumsCR.data());
    

    uint64_t totalSqrDiffLm = 0;
    uint64_t totalSqrDiffCb = 0;
    uint64_t totalSqrDiffCr = 0;
    
    for (auto v : rowSumsLM) totalSqrDiffLm += v;
    for (auto v : rowSumsCB) totalSqrDiffCb += v;
    for (auto v : rowSumsCR) totalSqrDiffCr += v;

    
    fmt::print("PSNR: ");
    printPSNRStats(totalSqrDiffLm);
    fmt::print(" | ");
    printPSNRStats(totalSqrDiffCb);
    fmt::print(" | ");
    printPSNRStats(totalSqrDiffCr);
    fmt::print(" | ");


    AvgtotalSqrDiffLm += totalSqrDiffLm;
    AvgtotalSqrDiffCb += totalSqrDiffCb;
    AvgtotalSqrDiffCr += totalSqrDiffCr;

    // prep buff to acumulet Squared Diff another frame
    cl_ulong zero = 0;
    for (uint32 c = 0; c < 3; c++) {
        m_Queue.enqueueFillBuffer(m_BufferTotalDiff[c], zero, 0, sizeof(cl_ulong));
    }
    return true;
}

// =================================================================================================================================================================================================================
void xPSNR_OpenCL::printPSNRStats(uint64 SSD) {

    uint64_t NumPoints  = (uint64_t)m_Width * m_Height;
    uint64_t MaxVal     = (uint64_t)(pow(2.0, m_BitDepth) - 1);
    float MAX           = NumPoints * (MaxVal * MaxVal);
    float psnr          = (SSD > 0.0) ? 10.0 * std::log10(MAX / SSD) 
                                        : std::numeric_limits<double>::infinity();
    
    fmt::printf(" %f dB", psnr);
}

void xPSNR_OpenCL::printTimeStats(int32 NumFrames)
{
  fmt::printf("AvgTime WriteBuff  %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationWriteBuff ).count() / NumFrames);
  fmt::printf("AvgTime ExecKernel %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationExecKernel).count() / NumFrames);
  fmt::printf("AvgTime ReadBuff   %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationReadBuff  ).count() / NumFrames);
}

void xPSNR_OpenCL::printAvgPNSRStats(uint32_t NumFrames) {
    float LM = AvgtotalSqrDiffLm / NumFrames;
    float CB = AvgtotalSqrDiffCb / NumFrames;
    float CR = AvgtotalSqrDiffCr / NumFrames;

    fmt::printf("\n\nAvgPSNR: ");
    printPSNRStats(LM);
    fmt::printf(" | ");
    printPSNRStats(CB);
    fmt::printf(" | ");
    printPSNRStats(CR);
    fmt::printf("\n");     
}

// =================================================================================================================================================================================================================
