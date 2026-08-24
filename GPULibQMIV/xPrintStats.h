#pragma once

#include "./lib_fmt/printf.h"
#include "CommonDef.h"

class xPrintStats
{
public:
    static void printPSNRTable(
        uint32 Frame,
        flt64 GpuLm,
        flt64 GpuCb,
        flt64 GpuCr,
        flt64 CpuLm,
        flt64 CpuCb,
        flt64 CpuCr);
    static void printTimeTable(
        uint32 Frame,
        flt64 GpuCopyBuff,
        flt64 GpuExecKernelSqrDiff,
        flt64 GpuExecKernelReduce,
        flt64 GpuReadBuff,
        flt64 GpuFillBuff,
        std::string kerneltype,
        tDuration CpuDuration);
};