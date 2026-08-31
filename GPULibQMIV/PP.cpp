/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2020, ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

 // Original authors: Jakub Stankowski, jakub.stankowski@put.poznan.pl,
 //                   Adrian Dziembowski, adrian.dziembowski@put.poznan.pl,
 //                   Poznan University of Technology, Poznań, Poland

//===============================================================================================================================================================================================================

#include "CommonDef.h"
#include "Metrics/SSIM/xSSIM.h"
#include "Metrics/SSIM/xSSIM_OpenCL.h"
#include "lib_fmt/core.h"
#include "lib_fmt/printf.h"
#include "xFile.h"
#include "xPic.h"
#include <math.h>
#include <cassert>
#include <string>
#include "xOpenCL_Enumerator.h"
#include "./Metrics/PSNR/xPSNR_OpenCL.h"
#include "xSeq.h"
#include "./Metrics/PSNR/xPSNR.h"
#include "xPrintStats.h"

//===============================================================================================================================================================================================================
int32 main(int argc, char *argv[]) {
  xOpenCL_Enumerator OpenCL;

  OpenCL.queryAllData();
  std::string ResultStr = OpenCL.printAllData(false);
  fmt::printf(ResultStr);
  //return EXIT_SUCCESS;

  //readed from commandline/config 
  std::string RefFile        = "../../Materials/A97_SL_QP1_TT_v0_1920x1080_yuv420p10le.yuv";
  std::string TestFile       = "../../Materials/A97_SL_QP2_TT_v0_1920x1080_yuv420p10le.yuv";
  //std::string OutputFile      = "../../Materials/Poznan_Street/Poznan_Street_00_1920x1088_tex_cam03_test.yuv";
  std::string PSNRKernelsFile     = "../Metrics/PSNR/kernels.cl";
  std::string SSIMKernelsFile    = "../Metrics/SSIM/SSIMkernels.cl";
  int32       PictureWidth    = 1920;
  int32       PictureHeight   = 1080;
  int32       BitDepth        = 8   ;  
  int32       ChromaFormat    = 420 ;
  int32       NumberOfFrames  = -1 ;  
  int32       NumberOfThreads = -1  ;
  int32       VerboseLevel    = 4   ;
  int32       BlockSize       = 11;

/*
//check OpenMP
#if defined(_OPENMP)
  int32 FinalNumberOfThreads = NumberOfThreads < 0 ? std::thread::hardware_concurrency() : std::min<int32>(NumberOfThreads, std::thread::hardware_concurrency());
#else
  int32 FinalNumberOfThreads = 0;
#endif
*/
//
  //print config
  if(VerboseLevel >= 1)
  {
    fmt::printf("Configuration:\n");
    fmt::printf("InputFile       = %s\n", RefFile        );
    fmt::printf("OutputFile      = %s\n", TestFile        );
    fmt::printf("KernelsFile     = %s\n", PSNRKernelsFile      );
    fmt::printf("PictureWidth    = %d\n", PictureWidth     );
    fmt::printf("PictureHeight   = %d\n", PictureHeight    );
    fmt::printf("BitDepth        = %d\n", BitDepth         );
    fmt::printf("ChromaFormat    = %d\n", ChromaFormat     );
    fmt::printf("NumberOfFrames  = %d%s\n", NumberOfFrames, NumberOfFrames==NOT_VALID ? "  (all)" : "");
    //fmt::printf("NumberOfThreads = %d%s\n", FinalNumberOfThreads, NumberOfThreads == NOT_VALID ? "  (all)" : "");
    fmt::printf("VerboseLevel    = %d\n", VerboseLevel     );
    fmt::printf("Block Size SSIM = %d\n", BlockSize);
    fmt::printf("\n");
  }

  //==============================================================================
  //operation
  if(VerboseLevel >= 2) { fmt::printf("Initializing:\n"); }

  // Ref file load
  if(!xFile::exist(RefFile)) { fmt::printf("ERROR --> InputFile does not exist (%s)", RefFile); return EXIT_FAILURE; }
  int64 SizeOfRefFile = xFile::filesize(RefFile);
  if(VerboseLevel >= 1) { fmt::printf("SizeOfRefFile = %d\n", SizeOfRefFile); }
  
  // Test file load
  if(!xFile::exist(TestFile)) { fmt::printf("ERROR --> InputFile does not exist (%s)", TestFile); return EXIT_FAILURE; }
  int64 SizeOfTestFile = xFile::filesize(TestFile);
  if(VerboseLevel >= 1) { fmt::printf("SizeOfTestFile = %d\n", SizeOfTestFile); }
  
  int32 NumOfFrames = xSeq::calcNumFramesInFile(PictureWidth, PictureHeight, BitDepth, ChromaFormat, SizeOfRefFile);
  if(VerboseLevel >= 1) { fmt::printf("DetectedFrames  = %d\n", NumOfFrames); }

  int32 MinSeqNumFrames = NumOfFrames;
  int32 NumFrames       = NumberOfFrames > 0 ? NumberOfFrames : MinSeqNumFrames;
  if(VerboseLevel >= 1) { fmt::printf("FramesToProcess = %d\n", NumFrames); }
  fmt::printf("\n");

  xSeq SequenceRef(PictureWidth, PictureHeight, BitDepth, ChromaFormat);
  xSeq SequenceTest(PictureWidth, PictureHeight, BitDepth, ChromaFormat);

  SequenceRef.openFile(RefFile , xSeq::eMode::Read );
  SequenceTest.openFile(TestFile, xSeq::eMode::Read);

  xPic PictureRefYUV(PictureWidth, PictureHeight, BitDepth, false);
  xPic PictureTestYUV(PictureWidth, PictureHeight, BitDepth, false);

  cl::Device Device = OpenCL.findMachingDevice(CL_DEVICE_TYPE_GPU, "Any", 120);
  if (Device() == nullptr) {
    fmt::printf("ERROR: %d no matching OpenCL device found\n", CL_DEVICE_TYPE_GPU);
    return EXIT_FAILURE;
  }

  xPSNR_OpenCL GPU;
  xPSNR        CPU;
  xSSIM        CPUSSIM;
  xSSIM_OpenCl GPUSSIM;
  //Processor.setVerboseLevel(VerboseLevel);
  //bool Created = GPU.create(PictureWidth, PictureHeight, PictureRefYUV.getMargin(), BitDepth, PSNRKernelsFile, Device);
bool CreatedCPU = CPUSSIM.create(PictureRefYUV, PictureTestYUV);
  if(!CreatedCPU) { return EXIT_FAILURE; }
  if(!CreatedCPU) { return EXIT_FAILURE; }

bool CreatedGPU = GPUSSIM.create(PictureWidth, PictureHeight, 
  PictureRefYUV.getMargin(), BitDepth, BlockSize,SSIMKernelsFile, Device);
  if(!CreatedGPU) { return EXIT_FAILURE; }
  if(!CreatedGPU) { return EXIT_FAILURE; }

  //==============================================================================
  //running
  
  if(VerboseLevel >= 2) { fmt::printf("Running:\n"); }

  tDuration DurationLoad = tDuration(0);
  tDuration DurationProc = tDuration(0);
  tDuration DurationStor = tDuration(0);

  for(int32 f = 0; f < NumFrames; f++)
  {
    
    tTimePoint T0 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    //LOAD
    bool ReadOKRef = SequenceRef.readFrame(&PictureRefYUV);
    if(!ReadOKRef) { fmt::printf("ERROR --> InputFile read error (%s)", RefFile); return EXIT_FAILURE; }

    bool ReadOKTest = SequenceTest.readFrame(&PictureTestYUV);
    if(!ReadOKTest) { fmt::printf("ERROR --> InputFile read error (%s)", TestFile); return EXIT_FAILURE; }

    tTimePoint T1 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();
    
    //PROCESS
    if(VerboseLevel >= 2) { fmt::printf("Frame %08d\n", f); }
    //GPU.processFrame(PictureRefYUV, PictureTestYUV);
    //CPU.cpuCalPSNR(PictureRefYUV, PictureTestYUV);
    //uint64 test = xPixelOpsSTD::CalcSSD(PictureRefYUV.getAddr(0), PictureTestYUV.getAddr(0), PictureRefYUV.getStride(), PictureTestYUV.getStride(), PictureWidth, PictureHeight);
    //fmt::printf("CPU SSD[0] = %llu\n", (unsigned long long)test);
    CPUSSIM.processFrame(PictureRefYUV, PictureTestYUV);
    GPUSSIM.processFrame(PictureRefYUV, PictureTestYUV);
    fmt::print("_______________________________________________\n");
    
    tTimePoint T2 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();
    
    //tTimePoint T3 = (VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

    DurationLoad += (T1 - T0);
    DurationProc += (T2 - T1);
    //DurationStor += (T3 - T2);    
  }
  //Processor.printAvgPNSRStats(NumFrames);
  //========================================NumFrames======================================
  //GPU.gpuAvgPSNR(NumFrames);
  //CPU.cpuAvgPNSR(NumFrames);
  
  //cleanup
  SequenceRef.closeFile();
  SequenceTest.closeFile();

  //==============================================================================
  //printout results

  xPrintStats PrintStats;
/*
  PrintStats.printTablePSNR(
    NumFrames,
    GPU.getGpuResultPSNRLm(),
    GPU.getGpuResultPSNRCb(),
    GPU.getGpuResultPSNRCr(),
    CPU.getCpuResultPSNRLm(),
    CPU.getCpuResultPSNRCb(),
    CPU.getCpuResultPSNRCr()
  );

  PrintStats.printTimeTablePSNR(
    NumFrames, 
    GPU.getTimeCopyBuff(), 
    GPU.getTimeExecKernelSqrDiff(), 
    GPU.getTimeExecKernelReduce(), 
    GPU.getTimeReadBuff(),
    GPU.getTimeFillBuff(),
    "PSNR 2 simple (for loop) kernels",
    CPU.DurationCpuCalcSSD);
  fmt::printf("\n\n");
  
  */
  PrintStats.printTableSSINM(
    NumFrames, 
    GPUSSIM.SSIMResultGPU[0], 
    GPUSSIM.SSIMResultGPU[1], 
    GPUSSIM.SSIMResultGPU[2],
    CPUSSIM.SSIMResultCPU[0], 
    CPUSSIM.SSIMResultCPU[1], 
    CPUSSIM.SSIMResultCPU[2]
  );

  PrintStats.printTimeTableSSIM(
    NumFrames, 
    GPUSSIM.getTimeCopyBuff(), 
    GPUSSIM.getTimeReadBuff(),
    GPUSSIM.getTimeExecKernelProcesBlock(), 
    GPUSSIM.getTimeExecKernelProcesLine(), 
    GPUSSIM.getTimeExecKernelReduceSum(), 
    "SSIM", 
    CPUSSIM.DurationCpuCalcSSIM 
  );

  /*
  if(VerboseLevel >= 3)
  {
    fmt::printf("AvgTime LOAD %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationLoad).count() / NumFrames); 
    fmt::printf("AvgTime PROC %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationProc).count() / NumFrames);
    fmt::printf("AvgTime STOR %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationStor).count() / NumFrames);
    GPU.printTimeStats(NumFrames);
  }
  */

  fmt::printf("\n");
  fmt::printf("NumFrames %d\n", NumFrames);
  fmt::printf("END-OF-LOG\n");

  return EXIT_SUCCESS;
}

//===============================================================================================================================================================================================================
