#pragma once

#include "./lib_fmt/printf.h"
#include "CommonDef.h"

class xPrintStats
{
public:
    static void printTablePSNR(
        flt64 GpuLm,
        flt64 GpuCb,
        flt64 GpuCr,
        flt64 CpuLm,
        flt64 CpuCb,
        flt64 CpuCr);

     static void printTableSSINM(
        flt64 GpuLm,
        flt64 GpuCb,
        flt64 GpuCr,
        flt64 CpuLm,
        flt64 CpuCb,
        flt64 CpuCr);

    static void printTimeTablePSNR(
        flt64 GpuCopyBuff,
        flt64 GpuExecKernelSqrDiff,
        flt64 GpuExecKernelReduce,
        flt64 GpuReadBuff,
        std::string kerneltype,
        flt64 CpuDuration);

    static void printTimeTableSSIM(
        flt64 GpuCopyBuff,
        flt64 GpuReadBuff,
        flt64 GpuExecKernelProcesBlock,
        flt64 GpuExecKernelProcesLine,
        flt64 GpuExecKernelReduceSum,
        std::string kerneltype,
        flt64 CpuDuration);
};