#include "./lib_fmt/printf.h"
#include "CommonDef.h"
#include "xPrintStats.h"

using tDurationF64 = std::chrono::duration<flt64, std::milli>;

void xPrintStats::printPSNRTable(
    uint32 Frame,
    flt64 GpuLm,
    flt64 GpuCb,
    flt64 GpuCr,
    flt64 CpuLm,
    flt64 CpuCb,
    flt64 CpuCr)
{
    fmt::printf(
        "Average PSNR Comparison GPU vs CPU\n");

    fmt::printf(
        "---------------------------------------------------------------------------\n");

    

    fmt::printf(
        "             %10s            %10s            %10s\n",
        "LM", "CB", "CR");

    fmt::printf(
        "---------------------------------------------------------------------------\n");

    fmt::printf(
        "GPU PSNR   %10.4f dB           %10.4f dB            %10.4f dB\n",
        GpuLm,
        GpuCb,
        GpuCr);

    fmt::printf(
        "CPU PSNR   %10.4f dB           %10.4f dB            %10.4f dB\n",
        CpuLm,
        CpuCb,
        CpuCr);

    fmt::printf(
        "---------------------------------------------------------------------------\n");
}

void xPrintStats::printTimeTable(
    uint32 NumFrames,
    flt64 GpuCopyBuff,
    flt64 GpuExecKernelSqrDiff,
    flt64 GpuExecKernelReduce,
    flt64 GpuReadBuff,
    flt64 GpuFillBuff,
    std::string kerneltype,
    tDuration CpuDuration)
{
    flt64 total = static_cast<flt64>(GpuCopyBuff + GpuCopyBuff + GpuExecKernelSqrDiff + GpuExecKernelReduce + GpuFillBuff) / NumFrames;
    fmt::printf(
        "---------------------------------------------------------------------------\n");

    

    fmt::printf(
        "GPU TIME ALLOCATION AND EXECTUITION FOR: %s\n", kerneltype);

    fmt::printf(
        "---------------------------------------------------------------------------\n");

    fmt::printf("Copy from CPU(host) to Buffor GPU      %10.4f ms\n",GpuCopyBuff / NumFrames);
    fmt::printf("Read form a Buffer                     %10.4f ms\n", GpuReadBuff / NumFrames);
    fmt::printf("Execution SqrDiff Kernel               %10.4f ms\n",GpuExecKernelSqrDiff / NumFrames);
    fmt::printf("Execution SqrDiff Kernel               %10.4f ms\n",GpuExecKernelReduce / NumFrames);
    fmt::printf("Fill GPU buffor with a val             %10.4f ms\n", GpuFillBuff / NumFrames);
   
    fmt::printf(
        "---------------------------------------------------------------------------\n");

    fmt::printf("Total GPU avg time:                    %10.4f ms\n", total);
    fmt::printf("Total CPU avg time:                    %10.4f ms\n", 
        std::chrono::duration_cast<tDurationF64>(CpuDuration).count() / NumFrames);
   
    fmt::printf(
        "---------------------------------------------------------------------------\n");
}