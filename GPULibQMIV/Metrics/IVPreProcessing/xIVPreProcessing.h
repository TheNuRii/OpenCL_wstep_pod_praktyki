#pragma once
#include "../../CommonDef.h"
#include "../../xPic.h"
#include <array>

class xIVPreProcessing {
public:
    flt64 IVMetricResultCPU;
    flt64 IVMetricSumCPU = 0;
    flt64 IVMetricResultGPU;
    flt64 IVMetricSumGPU = 0;

    enum eCmp {
        LM = 0,
        CB = 1,
        CR = 2,
    };

protected:
    std::array<flt32, 3> m_GCD = {0.0, 0.0, 0.0};
    std::array<flt64, 3> m_CCW;
    tDuration DurationCpuCalcSSIM = tDuration(0);

    int32 m_Width     = NOT_VALID;
    int32 m_Height    = NOT_VALID;
    int32 m_Margin    = NOT_VALID;
    int32 m_Stride    = NOT_VALID;
    int32 m_BitDepth  = NOT_VALID;
    int32 m_VerboseLevel = 3;
    int32 m_BlockSize = NOT_VALID;

    int32 m_CPS       = NOT_VALID;
    flt64 m_MUD       = NOT_VALID;
    
public:
    bool    create(int32 Width, int32 Height, int32 Margin, 
        int32 BitDepth, int32 CPS, flt32 MUD, flt64 CWW_Lm, flt64 CWW_Cb, flt64 CWW_Cr);
    
    void    processShiftingFrames(const xPic& PicRef, const xPic& PicTest, xPic& ShiffedPic);

    flt64   calcGlobalColorDiff(int32 colorComp, const xPic& PicI, const xPic& PicJ);
    int32V4 pixelShiftSearch(const int32V4& TstPel, const xPic* Ref);
    flt64   calcWieghtedMetric(std::array<flt64, 3> PSNRSiffedRef, std::array<flt64, 3> PSNRSiffedTest);
};