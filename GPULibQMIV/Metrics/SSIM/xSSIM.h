#pragma once
#include "../../CommonDef.h"
#include "../../xOpenCL_Common.h"
#include "../../xPic.h"
#include <array>
#include <cmath>
#include <vector>
#include "../../xPixelOps.h"
#include <mdspan>

class xSSIM {
public:
    enum class colorSpace {
        LM = 1,
        CB = 2,
        CR = 3,
    };

    struct SSIMStats {
        flt32 MeanX;
        flt32 MeanY;

        flt32 VarX;
        flt32 VarY;

        flt32 CovXY;
    };

protected:
    std::array<flt32, 3> SSIMResultCPU;               
    std::array<flt64, 3> SSIMSumCPU = {0.0, 0.0, 0.0}; 
    tDuration DurationCpuCalcSSIM = tDuration(0);

    int32 m_Width     = NOT_VALID;
    int32 m_Height    = NOT_VALID;
    int32 m_Margin    = NOT_VALID;
    int32 m_Stride    = NOT_VALID;
    int32 m_BitDepth  = NOT_VALID;
    int32 m_BlockSize = NOT_VALID;
    int32 m_VerboseLevel = 3;

    int32 m_Step            = NOT_VALID;
    int32 m_BlocksWidth     = NOT_VALID;
    int32 m_BlocksHeight    = NOT_VALID;

    int32 m_DynamicRange = NOT_VALID;
    flt32 m_C1           = NOT_VALID;
    flt32 m_C2           = NOT_VALID;

public:
    bool create(int32 Width, int32 Height, int32 Margin, 
        int32 BitDepth, int32 BlockSize, int32 Step);
    SSIMStats calcStats(const uint16* X, const uint16* Y);
    flt32     calcSSIM(const SSIMStats& s);
    void      processFrame(xPic& PicRef, xPic& PicTest);

    flt32 getAvgSSIM(int32 CmpIdx, int32 NumFrames) const { 
        return static_cast<flt32>(SSIMSumCPU[CmpIdx] / NumFrames); }

    flt64 getAvgTimeMs(int32 NumFrames) const {
    return std::chrono::duration_cast<std::chrono::duration<flt64, std::milli>>
    (DurationCpuCalcSSIM).count() / NumFrames;
}
};