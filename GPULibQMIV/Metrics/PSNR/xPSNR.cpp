#include "xPSNR.h"
#include "../../xPixelOpsSTD.h"
#include <vector>
#include "../../lib_fmt/printf.h"

void xPSNR::cpuCalPSNR(xPic& Ref, xPic& Test)
{   
    m_Width    = Ref.getWidth();
    m_Height   = Ref.getHeight();
    m_Margin   = Ref.getMargin();
    m_Stride   = Ref.getStride();
    m_BitDepth = Ref.getBitDepth();

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

    std::vector<flt64> PSNR(3);

    for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
        uint64_t NumPoints = (uint64_t)m_Width * m_Height;
        uint64_t MaxVal    = (uint64_t)(pow(2.0, m_BitDepth) - 1);
        flt64 MAX          = NumPoints * (MaxVal * MaxVal);

        PSNR[CmpIdx] = (SSD[CmpIdx] > 0)
            ? 10.0 * std::log10(MAX / SSD[CmpIdx])
            : std::numeric_limits<double>::infinity();
    }

    fmt::printf(
        "| CPU: LM %8.4f dB | CB %8.4f dB | CR %8.4f dB\n",
        PSNR[0], PSNR[1], PSNR[2]
    );
    CpuResultPSNRLm += PSNR[0];
    CpuResultPSNRCb += PSNR[1];
    CpuResultPSNRCr += PSNR[2];

    if (m_VerboseLevel >= 1) {
        DurationCpuCalcSSD += T1 - T0;
    }
}

void xPSNR::cpuAvgPNSR(uint32 NumFrames) {
    CpuResultPSNRLm = CpuResultPSNRLm / NumFrames;
    CpuResultPSNRCb = CpuResultPSNRCb / NumFrames;
    CpuResultPSNRCr = CpuResultPSNRCr / NumFrames;
};