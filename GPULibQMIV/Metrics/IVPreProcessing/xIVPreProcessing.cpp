#include "xIVPreProcessing.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include "../../xVec4.h"

bool xIVPreProcessing::create(int32 Width, int32 Height, int32 Margin, 
    int32 BitDepth, int32 CPS, flt32 MUD, flt64 CWW_Lm, flt64 CWW_Cb, flt64 CWW_Cr)
{
    m_Width     = Width;
    m_Height    = Height;
    m_Margin    = Margin;
    m_BitDepth  = BitDepth;
    m_Stride    = Width + (m_Margin << 1);
    m_CPS       = CPS;
    m_MUD       = MUD;
    m_BlockSize = 2 * CPS + 1;
    m_CCW[0]    = CWW_Lm;
    m_CCW[1]    = CWW_Cb;
    m_CCW[2]    = CWW_Cr;  

    return true;
}

flt64 xIVPreProcessing::calcGlobalColorDiff(int32 colorComp, const xPic& PicRef, const xPic& PicTest){
    const uint16* restrict addressPicRef = PicRef.getAddr(colorComp);
    const uint16* restrict addressPicTest = PicTest.getAddr(colorComp);

    int64 AccumAvg = 0;
    for (int32 y = 0; y < m_Height; y++) {
        for (int32 x = 0; x < m_Width; x++){
            AccumAvg += (addressPicRef[x] - addressPicTest[x]);
        }
        addressPicRef  += PicRef.getStride();
        addressPicTest += PicTest.getStride();
    }
    flt64 result;
    result = std::max(static_cast<flt64>(AccumAvg) / (m_Width * m_Height), m_MUD);
    return result;
}

int32V4 xIVPreProcessing::pixelShiftSearch(const int32V4& TstPixVec, const xPic* Ref){
    const uint16* RefPtrLm = Ref->getAddr  (eCmp::LM);
    const uint16* RefPtrCb = Ref->getAddr  (eCmp::CB);
    const uint16* RefPtrCr = Ref->getAddr  (eCmp::CR);
    
    int32V4 MinPixVec = {INT32_MAX, INT32_MAX, INT32_MAX, 0};
    int32   CurrDiff;
    int32   CurrPix;
    int32   MinDiff = INT32_MAX;
    int32   TstPix  = TstPixVec[0] + TstPixVec[1] + TstPixVec[2];

    for (int32 yShift = -m_CPS; yShift <= m_CPS; yShift++){
        const int32 RefOffset = (yShift + m_Margin) * Ref->getStride();
        for (int32 xShift = -m_CPS; xShift <= m_CPS; xShift++){

            // Sum val in each color component
            CurrPix  = RefPtrLm[RefOffset + xShift] + (int32)RefPtrCb[RefOffset + xShift] + (int32)RefPtrCr[RefOffset + xShift];
            CurrDiff = std::abs(TstPix - CurrPix);
            if (CurrDiff < MinDiff){
                MinDiff =  CurrDiff;
                MinPixVec = int32V4((int32)(RefPtrLm[RefOffset + xShift]), (int32)RefPtrCb[RefOffset + xShift], (int32)RefPtrCr[RefOffset + xShift], 0);
            }
        }
    }
    return MinPixVec;
}

void xIVPreProcessing::processShiftingFrames(const xPic& PicRef,const xPic& PicTest, xPic& ShiffedPic){
    // Block Global Component Diffrences
    for (int32 CmpIdx = 0; CmpIdx < 3; CmpIdx++) {
        m_GCD[CmpIdx] = calcGlobalColorDiff(CmpIdx, PicRef, PicTest);
    }
    
    // troche brute force implementation (wraper) - TODO 
    flt32V4 fltGlobalColorShift = {m_GCD[0], m_GCD[1], m_GCD[2], 0.0};
    int32V4 GlobalColorShift    = xRoundFltToInt32(fltGlobalColorShift);

    // Pixel Shift Search
    int32V4 shiffedPixel;
    for (int32 y = 0; y < m_Height; y++){
        const int32  TstOffset = y * PicTest.getStride();

        const uint16* TstPtrLm = PicTest.getAddr(eCmp::LM) + TstOffset;
        const uint16* TstPtrCb = PicTest.getAddr(eCmp::CB) + TstOffset;
        const uint16* TstPtrCr = PicTest.getAddr(eCmp::CR) + TstOffset;

        for (int32 x = 0; x < m_Width; x++){
            const int32V4 CurrTstValue  = int32V4((int32)(TstPtrLm[x]), (int32)(TstPtrCb[x]), (int32)(TstPtrCr[x]), 0) + GlobalColorShift; 
            shiffedPixel = pixelShiftSearch(CurrTstValue, &PicRef);
            
            ShiffedPic.getAddr(eCmp::LM)[TstOffset + x] = (uint16)shiffedPixel[0];
            ShiffedPic.getAddr(eCmp::CB)[TstOffset + x] = (uint16)shiffedPixel[1];
            ShiffedPic.getAddr(eCmp::CR)[TstOffset + x] = (uint16)shiffedPixel[2];
        }
    }
}

flt64  xIVPreProcessing::calcWieghtedMetric(std::array<flt64, 3> PSNRSiffedRef, std::array<flt64, 3> PSNRSiffedTest){
    flt64 WeighedPSNRSiffedRef = 
    (PSNRSiffedRef[0] * m_CCW[0] + PSNRSiffedRef[1]*m_CCW[1] + PSNRSiffedRef[2] * m_CCW[2]) / (m_CCW[0] + m_CCW[1] + m_CCW[2]);

    flt64 WeighedPSNRSiffedTest = 
    (PSNRSiffedTest[0] * m_CCW[0] + PSNRSiffedTest[1]*m_CCW[1] + PSNRSiffedTest[2] * m_CCW[2]) / (m_CCW[0] + m_CCW[1] + m_CCW[2]);

    return std::min(WeighedPSNRSiffedRef, WeighedPSNRSiffedTest);
}

