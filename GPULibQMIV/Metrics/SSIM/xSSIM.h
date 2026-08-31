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

    std::array<flt32, 3> SSIMResultCPU;
    tDuration DurationCpuCalcSSIM = tDuration(0);
protected:
    int32 m_Width     = NOT_VALID;
    int32 m_Height    = NOT_VALID;
    int32 m_Margin    = NOT_VALID;
    int32 m_Stride    = NOT_VALID;
    int32 m_BitDepth  = NOT_VALID;
    int32 m_BlockSize = NOT_VALID;
    int32 m_VerboseLevel = 3;

    int32 m_DynamicRange = NOT_VALID;
    flt32 m_C1           = NOT_VALID;
    flt32 m_C2           = NOT_VALID;

    

public:
    bool      create(xPic& PicRef, xPic& PicTest);
    SSIMStats calcStats(const uint16* X, const uint16* Y);
    flt32     calcSSIM(const SSIMStats& s);
    void      processFrame(xPic& PicRef, xPic& PicTest);
};