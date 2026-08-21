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
};