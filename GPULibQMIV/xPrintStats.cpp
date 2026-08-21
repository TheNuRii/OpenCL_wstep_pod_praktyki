#include "./lib_fmt/printf.h"
#include "CommonDef.h"
#include "xPrintStats.h"

void xPrintStats::printPSNRTable(
    uint32 Frame,
    flt64 GpuLm,
    flt64 GpuCb,
    flt64 GpuCr,
    flt64 CpuLm,
    flt64 CpuCb,
    flt64 CpuCr)
{
    fmt::printf("Frames %08d\n", Frame);

    fmt::printf(
        "---------------------------------------------------------------------------\n");

    fmt::printf(
        "Average PSNR Comparison GPU vs CPU\n");

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