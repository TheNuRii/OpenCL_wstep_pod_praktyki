#pragma once
#include "../../CommonDef.h"
#include "../../xOpenCL_Common.h"
#include <CL/opencl.hpp>
#include <array>
#include <cmath>
#include "../../xPic.h"

class xPSNR {
public:
    
    enum class colorSpace {
        LM = 0,
        CB = 1,
        CR = 2,
    };
    
protected:
    std::array<flt64, 3> PSNRResultCPU;
    std::array<flt64, 3> PSNRSumCPU = {0.0, 0.0, 0.0};
    tDuration DurationCpuCalcSSD = tDuration(0);

    int32 m_Width       = NOT_VALID;
    int32 m_Height      = NOT_VALID;
    int32 m_Margin      = NOT_VALID;
    int32 m_Stride      = NOT_VALID;
    int32 m_BitDepth    = NOT_VALID;

    int32        m_VerboseLevel  = 1;
    const int32  m_NumComponents = 3;

public:
    bool create(int32 Width, int32 Height, int32 Margin, int32 BitDepth);
    void  processFrame(xPic& Ref, xPic& Test);

    flt64 getAvgPSNR(int32 CmpIdx, int32 NumFrames) const { 
        return static_cast<flt64>(PSNRSumCPU[CmpIdx] / NumFrames); }
    
    flt64 getAvgTimeMs(int32 NumFrames) const {
    return std::chrono::duration_cast<std::chrono::duration<flt64, std::milli>>
    (DurationCpuCalcSSD).count() / NumFrames;}

    std::array<flt64, 3> getCurrentPSNRFrame() const {return PSNRResultCPU;}
};