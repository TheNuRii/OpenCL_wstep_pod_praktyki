#include "./lib_fmt/printf.h"
#include "CommonDef.h"
#include "xPrintStats.h"

using tDurationF64 = std::chrono::duration<flt64, std::milli>;

void xPrintStats::printTablePSNR(
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

void xPrintStats::printTableSSINM(
    uint32 Frame,
    flt64 GpuLm,
    flt64 GpuCb,
    flt64 GpuCr,
    flt64 CpuLm,
    flt64 CpuCb,
    flt64 CpuCr)
{
    fmt::printf(
        "Average SSIM Comparison GPU vs CPU\n");

    fmt::printf(
        "---------------------------------------------------------------------------\n");

    

    fmt::printf(
        "             %10s            %10s            %10s\n",
        "LM", "CB", "CR");

    fmt::printf(
        "---------------------------------------------------------------------------\n");

    fmt::printf(
        "GPU PSNR   %10.4f            %10.4f             %10.4f \n",
        GpuLm / Frame,
        GpuCb / Frame,
        GpuCr / Frame);

    fmt::printf(
        "CPU PSNR   %10.4f            %10.4f             %10.4f \n",
        CpuLm / Frame,
        CpuCb / Frame,
        CpuCr / Frame);

    fmt::printf(
        "---------------------------------------------------------------------------\n");
}

// ------------------------------------------------------------------------------------------------------------

void xPrintStats::printTimeTablePSNR(
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
        "GPU TIME ALLOCATION AND EXECTUITION FOR (Singel Farme): %s\n", kerneltype);

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

void xPrintStats::printTimeTableSSIM(
    uint32 NumFrames,
    flt64 GpuCopyBuff,
    flt64 GpuReadBuff,
    flt64 GpuExecKernelProcesBlock,
    flt64 GpuExecKernelProcesLine,
    flt64 GpuExecKernelReduceSum,
    std::string kerneltype,
    tDuration CpuDuration)
{
    flt64 total = static_cast<flt64>(GpuCopyBuff + GpuCopyBuff + GpuExecKernelProcesBlock 
        + GpuExecKernelProcesLine + GpuExecKernelReduceSum) / NumFrames;
    fmt::printf(
        "---------------------------------------------------------------------------\n");

    

    fmt::printf(
        "GPU TIME ALLOCATION AND EXECTUITION FOR (Singel Farme): %s\n", kerneltype);

    fmt::printf(
        "---------------------------------------------------------------------------\n");

    fmt::printf("Copy from CPU(host) to Buffor GPU      %10.4f ms\n",GpuCopyBuff / NumFrames);
    fmt::printf("Read form a Buffer                     %10.4f ms\n", GpuReadBuff / NumFrames);
    fmt::printf("Execution ProcesBlock Kernel           %10.4f ms\n",GpuExecKernelProcesBlock / NumFrames);
    fmt::printf("Execution ProcesLine Kernel            %10.4f ms\n",GpuExecKernelProcesLine / NumFrames);
    fmt::printf("Execution ReduceSum Kernel             %10.4f ms\n",GpuExecKernelReduceSum / NumFrames);
   
    fmt::printf(
        "---------------------------------------------------------------------------\n");

    fmt::printf("Total GPU avg time:                    %10.4f ms\n", total);
    fmt::printf("Total CPU avg time:                    %10.4f ms\n", 
        std::chrono::duration_cast<tDurationF64>(CpuDuration).count() / NumFrames);
   
    fmt::printf(
        "---------------------------------------------------------------------------\n");
}