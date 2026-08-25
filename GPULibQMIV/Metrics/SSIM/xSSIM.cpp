#include "xSSIM.h"

bool xSSIM::create(xPic& PicRef, xPic& PicTest){

    m_BlockSize = 11;
    m_Width     = PicRef.getWidth();
    m_Height    = PicRef.getHeight();
    m_Margin    = 8; // tyle by zabezpieczycz blok 11 x 11 na krancu obrazu
    m_Stride    = m_Width + (m_Margin << 1);
    m_BitDepth  = PicRef.getBitDepth();

    m_DynamicRange = (1 << m_BitDepth) - 1;
    m_C1 = (0.01 * m_DynamicRange)*(0.01 * m_DynamicRange); // 0.01 is param k1 form wikipedia
    m_C2 = (0.03 * m_DynamicRange)*(0.03 * m_DynamicRange); // 0.03 is param k2 form wikipedia
    m_C3 = static_cast<flt32>(m_C2) / 2;  

    // RESIZEING: chandge margins (pic buffors), for block size of compute
   
    uint16* addresRefPic  = nullptr;
    uint16* addresTestPic = nullptr;
    uint64 average;
    // w zasadzie tutaj nie musilem tego robić w xPic zmienilem m_Margin z 4 na 8
    /*
    for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++){
        addresRefPic = PicRef.getAddr(CmpIdx);

        xPixelOps::ExtendMargin(addresRefPic, m_Stride, m_Width, m_Height, m_Margin);

        addresTestPic = PicTest.getAddr(CmpIdx);
        xPixelOps::ExtendMargin(addresTestPic, m_Stride, m_Width, m_Height, m_Margin);
    }
    */
    return true;
}

xSSIM::SSIMStats xSSIM::calcStats(const uint16* X, const uint16* Y){
    uint64 sumX      = 0;
    uint64 sumY      = 0;
    uint64 sumX2     = 0;
    uint64 sumY2     = 0;
    uint64 sumXY     = 0;

    const uint32 N = m_BlockSize * m_BlockSize;

    for (uint32 row = 0; row < m_BlockSize; row++)
    {
        const uint16* XRow = X + row * m_Stride;

        const uint16* YRow = Y + row * m_Stride;

        for (uint32 col = 0; col < m_BlockSize; col++)
        {
            const uint64 x = XRow[col];
            const uint64 y = YRow[col];

            sumX += x;
            sumY += y;

            sumX2 += x * x;
            sumY2 += y * y;

            sumXY += x * y;
        }
    }

    const flt64 meanX =
        static_cast<flt64>(sumX) / N;

    const flt64 meanY =
        static_cast<flt64>(sumY) / N;

    const flt64 varX =
        (static_cast<flt64>(sumX2) - static_cast<flt64>(sumX) * meanX)
        / (N - 1);

    const flt64 varY =
        (static_cast<flt64>(sumY2) - static_cast<flt64>(sumY) * meanY)
        / (N - 1);

    const flt64 covXY =
        (static_cast<flt64>(sumXY) - static_cast<flt64>(sumX) * meanY)
        / (N - 1);

    return {
        meanX,
        meanY,
        varX,
        varY,
        covXY
    };
}
flt64 xSSIM::calcSSIM(const SSIMStats& s)
{
    const flt64 numerator =
        (2.0 * s.MeanX * s.MeanY + m_C1) *
        (2.0 * s.CovXY + m_C2);

    const flt64 denominator =
        (s.MeanX * s.MeanX + s.MeanY * s.MeanY + m_C1) *
        (s.VarX + s.VarY + m_C2);

    return numerator / denominator;
}

void xSSIM::processFrame(xPic& PicRef, xPic& PicTest)
{
    const int32 Radius = m_BlockSize / 2;

    for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++)
    {
        const uint16* RefPic =
            PicRef.getAddr(CmpIdx);

        const uint16* TestPic =
            PicTest.getAddr(CmpIdx);

        flt64 FrameSSIM = 0.0;

        for (int32 y = 0; y < m_Height; y++)
        {
            for (int32 x = 0; x < m_Width; x++)
            {
                const uint16* RefBlock = RefPic +
                    (y - Radius) * m_Stride +
                    (x - Radius);

                const uint16* TestBlock = TestPic +
                    (y - Radius) * m_Stride +
                    (x - Radius);

                const SSIMStats s = calcStats(RefBlock, TestBlock);

                FrameSSIM += calcSSIM(s);
            }
        }

        SSIMResultCPU[CmpIdx] =
            FrameSSIM /
            (static_cast<flt64>(m_Width) * m_Height);
    }

    fmt::printf(
        "| CPU: LM %8.4f | CB %8.4f | CR %8.4f\n",
        SSIMResultCPU[0],
        SSIMResultCPU[1],
        SSIMResultCPU[2]
    );
}

