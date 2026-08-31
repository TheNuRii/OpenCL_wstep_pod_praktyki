#include "xSepia_OpenCL.h"
#include "../../xFile.h"

//===============================================================================================================================================================================================================

bool xSepia_OpenCL::create(int32 Width, int32 Height, int32 Margin, const std::string& KernelsFile, cl::Device& Device, eCopyMode CopyMode)
{
  m_Width  = Width;
  m_Height = Height;
  m_Margin = Margin;
  m_Stride = Width + (m_Margin << 1);

  m_BuffCmpNumPels  = (m_Width + (m_Margin << 1)) * (m_Height + (m_Margin << 1));
  m_BuffCmpNumBytes = m_BuffCmpNumPels * sizeof(uint16);

  m_Device  = Device;

  //create context
  m_Context = cl::Context(m_Device);

  //compile kernel(s)
  cl_int Result = CL_SUCCESS;

  fmt::printf("Processing kernels.cl\n");
  if(!xFile::exist(KernelsFile)) { fmt::printf("ERROR - kernel file does not exist - %s\n", KernelsFile); return false; }
  std::ifstream KernelSourceFile(KernelsFile);
  const std::string KernelSource((std::istreambuf_iterator<char>(KernelSourceFile)), std::istreambuf_iterator<char>());

  cl::Program::Sources Sources({ KernelSource });
  m_Program = cl::Program(m_Context, Sources, &Result);
  Result = m_Program.build();

  fmt::printf("Build info --> CL_PROGRAM_BUILD_LOG:\n");
  auto BuildLogLines = m_Program.getBuildInfo<CL_PROGRAM_BUILD_LOG>();
  for(auto BuildLogLine : BuildLogLines) { fmt::printf(BuildLogLine.second); }
  fmt::printf("Build info --> CL_PROGRAM_BUILD_STATUS:\n");
  auto BuildStats = m_Program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>();
  for(auto BuildStatus : BuildStats) { fmt::printf("Status = %d\n", BuildStatus.second); }
  if(Result != CL_SUCCESS) { fmt::printf("ERROR - program - %d\n", Result); return false; }

  m_Kernels[(int32)eKernelOp::Copy         ] = cl::Kernel(m_Program, "Copy"         , &Result); if(Result != CL_SUCCESS) { fmt::printf("ERROR - kernel Copy - %d\n"         , Result); return false; }
  m_Kernels[(int32)eKernelOp::YUVtoRGBtoYUV] = cl::Kernel(m_Program, "YUVtoRGBtoYUV", &Result); if(Result != CL_SUCCESS) { fmt::printf("ERROR - kernel YUVtoRGBtoYUV - %d\n", Result); return false; }
  m_Kernels[(int32)eKernelOp::Sepia        ] = cl::Kernel(m_Program, "Sepia"        , &Result); if(Result != CL_SUCCESS) { fmt::printf("ERROR - kernel Sepia - %d\n"        , Result); return false; }

  //create command queue
  m_Queue = cl::CommandQueue(m_Context, m_Device, 0, &Result);
  if(Result != CL_SUCCESS) { fmt::printf("ERROR - queue - %d\n", Result); return false; }

  m_CopyMode = CopyMode;
  if(m_CopyMode == eCopyMode::FullCopy)
  {
    //create device buffers
    for(uint32 c = 0; c < 3; c++)
    {
      m_BufferSrc[c] = cl::Buffer(m_Context, (cl_mem_flags)(CL_MEM_READ_ONLY | CL_MEM_HOST_WRITE_ONLY), (cl::size_type)m_BuffCmpNumBytes, nullptr, &Result);
      if(Result != CL_SUCCESS) { fmt::printf("ERROR - buffer src - %d\n", Result); return false; }
    }
    for(uint32 c = 0; c < 3; c++)
    {
      m_BufferDst[c] = cl::Buffer(m_Context, (cl_mem_flags)(CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY), (cl::size_type)m_BuffCmpNumBytes, nullptr, &Result);
      if(Result != CL_SUCCESS) { fmt::printf("ERROR - buffer dst - %d\n", Result); return false; }
    }
  }

  return true;
}
bool xSepia_OpenCL::xRunOneKernel(xPic& Dst, const xPic& Src, eKernelOp KernelOp)
{
  cl_int Result = CL_SUCCESS;

  tTimePoint T0 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

  if(m_CopyMode == eCopyMode::FullCopy)
  {
    //copy data
    for(uint32 c = 0; c < 3; c++)
    {
      Result = m_Queue.enqueueWriteBuffer(m_BufferSrc[c], false, 0, m_BuffCmpNumBytes, Src.getBuffer(c));
      if(Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueWriteBuffer - %d\n", Result); return false; }
    }
    Result = m_Queue.finish();
    if(Result != CL_SUCCESS) { fmt::printf("ERROR - finish - %d\n", Result); return false; }
  }
  else if(m_CopyMode == eCopyMode::ZeroCopy)
  {
    //create bufers from host mem
    for(uint32 c = 0; c < 3; c++)
    {
      m_BufferSrc[c] = cl::Buffer(m_Context, (cl_mem_flags)(CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR), (cl::size_type)m_BuffCmpNumBytes, (void*)(Src.getBuffer(c)), &Result);
      if(Result != CL_SUCCESS) { fmt::printf("ERROR - buffer src - %d\n", Result); return false; }
      m_BufferDst[c] = cl::Buffer(m_Context, (cl_mem_flags)(CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR), (cl::size_type)m_BuffCmpNumBytes, (void*)(Dst.getBuffer(c)), &Result);
      if(Result != CL_SUCCESS) { fmt::printf("ERROR - buffer src - %d\n", Result); return false; }
    }
  }

  tTimePoint T1 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

  cl::Kernel& SelectedKernel = m_Kernels[(int32)KernelOp];

  //set kernel arguments
  Result = SelectedKernel.setArg(0, m_BufferDst[0]); //__global       uint16* restrict DstLm
  Result = SelectedKernel.setArg(1, m_BufferDst[1]); //__global       uint16* restrict DstCb
  Result = SelectedKernel.setArg(2, m_BufferDst[2]); //__global       uint16* restrict DstCr
  Result = SelectedKernel.setArg(3, m_BufferSrc[0]); //__global const uint16* restrict SrcLm
  Result = SelectedKernel.setArg(4, m_BufferSrc[1]); //__global const uint16* restrict SrcCb
  Result = SelectedKernel.setArg(5, m_BufferSrc[2]); //__global const uint16* restrict SrcCr
  Result = SelectedKernel.setArg(6, m_Width       ); //const int Width
  Result = SelectedKernel.setArg(7, m_Height      ); //const int Height
  Result = SelectedKernel.setArg(8, m_Margin      ); //const int Margin
  Result = SelectedKernel.setArg(9, m_Stride      ); //const int Stride

  //execute
  Result = m_Queue.enqueueNDRangeKernel(SelectedKernel, cl::NullRange, cl::NDRange(m_Width, m_Height), cl::NullRange, nullptr, nullptr);
  if(Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueNDRangeKernel - %d\n", Result); return false; }
  Result = m_Queue.finish();
  if(Result != CL_SUCCESS) { fmt::printf("ERROR - finish - %d\n", Result); return false; }

  tTimePoint T2 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

  if(m_CopyMode == eCopyMode::FullCopy)
  {
    //copy data
    for(uint32 c = 0; c < 3; c++)
    {
      Result = m_Queue.enqueueReadBuffer(m_BufferDst[c], false, 0, m_BuffCmpNumBytes, Dst.getBuffer(c));
      if(Result != CL_SUCCESS) { fmt::printf("ERROR - enqueueReadBuffer - %d\n", Result); return false; }
    }
    Result = m_Queue.finish();
    if(Result != CL_SUCCESS) { fmt::printf("ERROR - finish - %d\n", Result); return false; }
  }
  else if(m_CopyMode == eCopyMode::ZeroCopy)
  {
    //release buffers
    for(uint32 c = 0; c < 3; c++)
    {
      m_BufferSrc[c] = cl::Buffer();
      m_BufferDst[c] = cl::Buffer();
    }
  }  

  tTimePoint T3 = (m_VerboseLevel >= 3) ? tClock::now() : tTimePoint::min();

  DurationWriteBuff  += (T1 - T0);
  DurationExecKernel += (T2 - T1);
  DurationReadBuff   += (T3 - T2);

  return true;
}
void xSepia_OpenCL::printTimeStats(int32 NumFrames)
{
  fmt::printf("AvgTime WriteBuff  %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationWriteBuff ).count() / NumFrames);
  fmt::printf("AvgTime ExecKernel %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationExecKernel).count() / NumFrames);
  fmt::printf("AvgTime ReadBuff   %9.2f ms\n", std::chrono::duration_cast<tDurationMS>(DurationReadBuff  ).count() / NumFrames);
}

//===============================================================================================================================================================================================================
