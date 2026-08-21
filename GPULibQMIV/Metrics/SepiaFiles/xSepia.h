#pragma once
#include "../../CommonDef.h"
#include "../../xPic.h"
#include "../../xThreadPool.h"

//===============================================================================================================================================================================================================
// nie trzeba bo tylko PSNR wyliczam, a oba pliki są .yuv, więc nie ma potrzeby konwersji YUV --> RGB --> YUV
class xCoeffs
{
public:
  static constexpr inline flt32 RGB2YCbCr_F32[3][3] = //BT709
  {
    {(flt32)( 0.21260), (flt32)( 0.71520), (flt32)( 0.07220)}, //Y_R, Y_G, Y_B
    {(flt32)(-0.11457), (flt32)(-0.38543), (flt32)( 0.50000)}, //U_R, U_G ,U_B
    {(flt32)( 0.50000), (flt32)(-0.45415), (flt32)(-0.04585)}, //V_R, V_B, V_G
  };
  static constexpr inline flt32 YCbCr2RGB_F32[3][3] =  //BT709
  {
    {(flt32)(1.0), (flt32)( 0      ), (flt32)( 1.5748 )}, //R_Y, R_U, R_V
    {(flt32)(1.0), (flt32)(-0.18732), (flt32)(-0.46812)}, //G_Y, G_U, G_V
    {(flt32)(1.0), (flt32)( 1.8556 ), (flt32)( 0      )}, //B_Y, B_U, B_V
  };
  static constexpr inline flt32 SEPIA_F32[3][3] = //SEPIA
  {
    {(flt32)(0.393), (flt32)(0.769), (flt32)(0.189)},
    {(flt32)(0.349), (flt32)(0.686), (flt32)(0.168)},
    {(flt32)(0.272), (flt32)(0.534), (flt32)(0.131)},
  };
};

//===============================================================================================================================================================================================================

class xSepiaBase : public xCoeffs
{
public:
  enum class eMode : int32
  {
    SERIAL    =-1,
    PIXELS    = 0,
    ROWS      = 1,
    TILES4    = 2,
    TILES8    = 3,
    TILES16   = 4,
    TILES32   = 5,
    TILES64   = 6,
    TILES128  = 7,
    TILES256  = 8,
    TILES512  = 9,
    TILES1024 = 10,
  };

protected:
  xThreadPool* m_ThreadPool    = nullptr;
  int32        m_VerboseLevel  = 0;
  const int32  m_NumComponents = 3;

public:
  void  setThreadPool  (xThreadPool* ThreadPool) { m_ThreadPool   = ThreadPool;   }
  void  setVerboseLevel(int32      VerboseLevel) { m_VerboseLevel = VerboseLevel; }
};

//===============================================================================================================================================================================================================

class xSepia_STD : public xSepiaBase
{
public:
  void  testCopyContent      (xPic& Dst, const xPic& Src);
  void  testYUVtoRGBtoYUV_FLT(xPic& Dst, const xPic& Src, eMode ProcMode);
  void  applySepiaEffect_FLT (xPic& Dst, const xPic& Src, eMode ProcMode);

protected:
  static void  xTestYUVtoRGBtoYUV_FLT(xPic& Dst, const xPic& Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX);
  static void  xApplySepiaEffect_FLT (xPic& Dst, const xPic& Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX);
};

//===============================================================================================================================================================================================================