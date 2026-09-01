#pragma once
#include "../../CommonDef.h"
#include "../../xOpenCL_Common.h"
#include <CL/opencl.hpp>
#include <cmath>
#include "../../xPic.h"

class xPSNR {
public:
    
    enum class colorSpace {
        LM = 0,
        CB = 1,
        CR = 2,
    };
    flt64 CpuResultPSNRLm = 0;
    flt64 CpuResultPSNRCb = 0;
    flt64 CpuResultPSNRCr = 0;

    tDuration DurationCpuCalcSSD = tDuration(0);

protected:
    int32 m_Width       = NOT_VALID;
    int32 m_Height      = NOT_VALID;
    int32 m_Margin      = NOT_VALID;
    int32 m_Stride      = NOT_VALID;
    int32 m_BitDepth    = NOT_VALID;

    int32        m_VerboseLevel  = 1;
    const int32  m_NumComponents = 3;

public: 
    void  processFrame(xPic& Ref, xPic& Test);
    void  cpuAvgPNSR(uint32 NumFrames);

    flt64 getCpuResultPSNRLm() { return CpuResultPSNRLm; }
    flt64 getCpuResultPSNRCb() { return CpuResultPSNRCb; }
    flt64 getCpuResultPSNRCr() { return CpuResultPSNRCr; }
};