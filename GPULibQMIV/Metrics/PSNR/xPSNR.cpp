#include "xPSNR.h"
#include "../../xPixelOpsSTD.h"
#include <vector>
#include "../../lib_fmt/printf.h"

bool xPSNR::create(int32 Width, int32 Height, int32 Margin, int32 BitDepth)
{
    m_Width     = Width;
    m_Height    = Height;
    m_Margin    = Margin;
    m_Stride    = Width + (m_Margin << 1);
    m_BitDepth  = BitDepth;

    return true;
}

void xPSNR::processFrame(xPic& Ref, xPic& Test)
{   
    std::vector<uint64_t> SSD(3);

    tTimePoint T0 = (m_VerboseLevel >= 1) ? tClock::now() : tTimePoint::min();

    for (uint32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
        SSD[CmpIdx] = xPixelOpsSTD::CalcSSD(
            Ref.getAddr(CmpIdx),
            Test.getAddr(CmpIdx),
            Ref.getStride(),
            Test.getStride(),
            m_Width,
            m_Height
        );
    }

    tTimePoint T1 = (m_VerboseLevel >= 1) ? tClock::now() : tTimePoint::min();

    for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
        uint64_t NumPoints = (uint64_t)m_Width * m_Height;
        uint64_t MaxVal    = (uint64_t)(pow(2.0, m_BitDepth) - 1);
        flt64 MAX          = NumPoints * (MaxVal * MaxVal);

        PSNRResultCPU[CmpIdx] = (SSD[CmpIdx] > 0)
            ? 10.0 * std::log10(MAX / SSD[CmpIdx])
            : std::numeric_limits<double>::infinity();
    }

    fmt::printf(
        "| CPU: LM %8.4f dB | CB %8.4f dB | CR %8.4f dB\n",
        PSNRResultCPU[0], PSNRResultCPU[1], PSNRResultCPU[2]
    );
    
    for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) {PSNRSumCPU[CmpIdx] += PSNRResultCPU[CmpIdx];}

    if (m_VerboseLevel >= 1) {
        DurationCpuCalcSSD += T1 - T0;
    }
}