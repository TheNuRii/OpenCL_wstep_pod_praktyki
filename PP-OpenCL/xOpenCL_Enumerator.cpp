#include "xOpenCL_Enumerator.h"
#include <sstream>
#include <iostream>

using std::string;
using std::vector;


//=====================================================================================================================================================================================
// xOpenCL_Enumerator
//=====================================================================================================================================================================================
cl::Device xOpenCL_Enumerator::findMachingDevice(cl_device_type DeviceType, string Vendor, int32 Version)
{
  std::vector<cl::Platform> platforms;
  cl::Platform::get(&platforms);
  for (auto& p : platforms) {
    std::cout << "Vendor: " << p.getInfo<CL_PLATFORM_VENDOR>()
              << " | Version: " << p.getInfo<CL_PLATFORM_VERSION>() << "\n";
  }

  std::vector<cl::Platform> Platforms;
  cl_int Result = cl::Platform::get(&Platforms);
  if(Result != CL_SUCCESS) { fprintf(stderr, "xOpenCL::findMachingDeviceContext() failed %d\n", Result); return cl::Device(); }
  for(cl::Platform& Platform : Platforms)
  {
    string sCL_PLATFORM_VENDOR  = Platform.getInfo<CL_PLATFORM_VENDOR >();
    string sCL_PLATFORM_VERSION = Platform.getInfo<CL_PLATFORM_VERSION>();
    int32 MajorVersion = 0;
    int32 MinorVersion = 0;
    sscanf(sCL_PLATFORM_VERSION.c_str(), "OpenCL %d.%d", &MajorVersion, &MinorVersion);
    int32 FinalVersion = MajorVersion*100 + (MinorVersion<10 ? MinorVersion*10 : MinorVersion);    

    if((Vendor=="Any" || sCL_PLATFORM_VENDOR.find(Vendor) != string::npos) && FinalVersion>=Version)
    {
      vector<cl::Device> Devices;
      Result = Platform.getDevices(DeviceType, &Devices);
      if(Result != CL_SUCCESS) { fprintf(stderr, "xOpenCL::findMachingDeviceContext() failed %d\n", Result); return cl::Device(); }
      for(cl::Device& Device : Devices)
      {
        string sCL_DEVICE_VERSION = Device.getInfo<CL_DEVICE_VERSION>();
        int32 MajorVersion = 0;
        int32 MinorVersion = 0;
        sscanf(sCL_DEVICE_VERSION.c_str(), "OpenCL %d.%d", &MajorVersion, &MinorVersion);
        int32 FinalVersion = MajorVersion*100 + (MinorVersion<10 ? MinorVersion*10 : MinorVersion);
        if(FinalVersion>=Version)
        {
          printf("xOpenCL::findMachingDevice: Found %s - %s @ %s\n", Platform.getInfo<CL_PLATFORM_NAME>().c_str(), Device.getInfo<CL_DEVICE_NAME>().c_str(), sCL_DEVICE_VERSION.c_str());
          return Device;
        }
      }
    }
  }
  printf("xOpenCL::findMachingDevice: Cannot find maching device!\n");
  return cl::Device();
}
cl_int xOpenCL_Enumerator::queryAllData()
{
  std::vector<cl::Platform> Platforms;
  cl_int Result = cl::Platform::get(&Platforms);
  if(Result != CL_SUCCESS) { fprintf(stderr, "xOpenCL::queryAllData() failed %d\n", Result); return Result; }
  for(cl::Platform& P : Platforms)
  {
    xPlatform Platform;
    Platform.m_Platform = P;
    Platform.queryAllData();
    m_Platforms.push_back(Platform);
  }
  return CL_SUCCESS;
}
string xOpenCL_Enumerator::printAllData(bool ListImageFormats)
{
  string Result;
  Result += "Found " + std::to_string(m_Platforms.size()) + " OpenCL platforms\n";
  for(vector<xPlatform>::iterator it = m_Platforms.begin(); it != m_Platforms.end(); it++)
  {
    int32 Index = (int32)std::distance(m_Platforms.begin(), it);
    Result += "Platform " + std::to_string(Index) + "\n";
    Result += it->printAllData("  ", ListImageFormats);
  }
  return Result;
}

//=============================================================================================================================================================================
// xOpenCL_Enumerator::xPlatform
//=====================================================================================================================================================================================
cl_int xOpenCL_Enumerator::xPlatform::queryAllData()
{
  cl_int Result;
  Result = m_Platform.getInfo(CL_PLATFORM_PROFILE,    &m_CL_PLATFORM_PROFILE);
  Result = m_Platform.getInfo(CL_PLATFORM_VERSION,    &m_CL_PLATFORM_VERSION);
  Result = m_Platform.getInfo(CL_PLATFORM_NAME,       &m_CL_PLATFORM_NAME);
  Result = m_Platform.getInfo(CL_PLATFORM_VENDOR,     &m_CL_PLATFORM_VENDOR);
  string PlatformExtensions;
  Result = m_Platform.getInfo(CL_PLATFORM_EXTENSIONS, &PlatformExtensions);
  std::istringstream StringStream(PlatformExtensions);
  std::copy(std::istream_iterator<string>{StringStream}, std::istream_iterator<string>(), back_inserter(m_CL_PLATFORM_EXTENSIONS));  

  vector<cl::Device> Devices;
  Result = m_Platform.getDevices(CL_DEVICE_TYPE_ALL, &Devices);
  if(Result != CL_SUCCESS) { fprintf(stderr, "xOpenCL::xPlatform::queryAllData() failed %d\n", Result); return Result; }
  for(cl::Device& D : Devices)
  {
    xDevice Device;
    Device.m_Device = D;
    Device.queryAllData();
    m_Devices.push_back(Device);
  }
  return CL_SUCCESS;
}
string xOpenCL_Enumerator::xPlatform::printAllData(string Prefix, bool ListImageFormats)
{
  string Result;
  Result += Prefix + "CL_PLATFORM_PROFILE:    " + m_CL_PLATFORM_PROFILE + "\n";
  Result += Prefix + "CL_PLATFORM_VERSION:    " + m_CL_PLATFORM_VERSION + "\n";
  Result += Prefix + "CL_PLATFORM_NAME:       " + m_CL_PLATFORM_NAME    + "\n";
  Result += Prefix + "CL_PLATFORM_VENDOR:     " + m_CL_PLATFORM_VENDOR  + "\n";
  Result += Prefix + "CL_PLATFORM_EXTENSIONS: "; for(string Extension : m_CL_PLATFORM_EXTENSIONS) { Result += Extension + " "; }; Result += "\n";

  Result += Prefix + "Found " + std::to_string(m_Devices.size()) + " OpenCL devices within a platforms\n";
  for(vector<xDevice>::iterator it = m_Devices.begin(); it != m_Devices.end(); it++)
  {
    int32 Index = (int32)std::distance(m_Devices.begin(), it);
    Result += Prefix + "Device " + std::to_string(Index) + "\n";
    Result += it->printAllData(Prefix + "  ", ListImageFormats);
  }
  return Result;
}

//=============================================================================================================================================================================
// xOpenCL_Enumerator::xDevice
//=====================================================================================================================================================================================
string xOpenCL_Enumerator::xDevice::cl_device_typeToString(cl_device_type DeviceType)
{
  string Result;
  if(DeviceType & CL_DEVICE_TYPE_DEFAULT)     { Result += "CL_DEVICE_TYPE_DEFAULT "; }
  if(DeviceType & CL_DEVICE_TYPE_CPU)         { Result += "CL_DEVICE_TYPE_CPU "; }
  if(DeviceType & CL_DEVICE_TYPE_GPU)         { Result += "CL_DEVICE_TYPE_GPU "; }
  if(DeviceType & CL_DEVICE_TYPE_ACCELERATOR) { Result += "CL_DEVICE_TYPE_ACCELERATOR "; }
  if(DeviceType & CL_DEVICE_TYPE_CUSTOM)      { Result += "CL_DEVICE_TYPE_CUSTOM "; }
  return Result;
}
string xOpenCL_Enumerator::xDevice::cl_device_exec_capabilitiesToString(cl_device_exec_capabilities DeviceExecCapabilities)
{
  string Result;
  if(DeviceExecCapabilities & CL_EXEC_KERNEL)        { Result += "CL_EXEC_KERNEL "; }
  if(DeviceExecCapabilities & CL_EXEC_NATIVE_KERNEL) { Result += "CL_EXEC_NATIVE_KERNEL "; }
  return Result;
}
string xOpenCL_Enumerator::xDevice::cl_channel_orderToString(cl_channel_order ChanellOrder)
{
  switch(ChanellOrder)
  {
    case CL_R:             return "CL_R            ";
    case CL_A:             return "CL_A            ";
    case CL_RG:            return "CL_RG           ";
    case CL_RA:            return "CL_RA           ";
    case CL_RGB:           return "CL_RGB          ";
    case CL_RGBA:          return "CL_RGBA         ";
    case CL_BGRA:          return "CL_BGRA         ";
    case CL_ARGB:          return "CL_ARGB         ";
    case CL_INTENSITY:     return "CL_INTENSITY    ";
    case CL_LUMINANCE:     return "CL_LUMINANCE    ";
    case CL_Rx:            return "CL_Rx           ";
    case CL_RGx:           return "CL_RGx          ";
    case CL_RGBx:          return "CL_RGBx         ";
    case CL_DEPTH:         return "CL_DEPTH        ";
    case CL_DEPTH_STENCIL: return "CL_DEPTH_STENCIL";
    default:               return "Unknown cl_channel_order";
  }
}
string xOpenCL_Enumerator::xDevice::cl_channel_typeToString(cl_channel_type ChanellType)
{
  switch(ChanellType)
  {
    case CL_SNORM_INT8:       return "CL_SNORM_INT8      ";
    case CL_SNORM_INT16:      return "CL_SNORM_INT16     ";
    case CL_UNORM_INT8:       return "CL_UNORM_INT8      ";
    case CL_UNORM_INT16:      return "CL_UNORM_INT16     ";
    case CL_UNORM_SHORT_565:  return "CL_UNORM_SHORT_565 ";
    case CL_UNORM_SHORT_555:  return "CL_UNORM_SHORT_555 ";
    case CL_UNORM_INT_101010: return "CL_UNORM_INT_101010";
    case CL_SIGNED_INT8:      return "CL_SIGNED_INT8     ";
    case CL_SIGNED_INT16:     return "CL_SIGNED_INT16    ";
    case CL_SIGNED_INT32:     return "CL_SIGNED_INT32    ";
    case CL_UNSIGNED_INT8:    return "CL_UNSIGNED_INT8   ";
    case CL_UNSIGNED_INT16:   return "CL_UNSIGNED_INT16  ";
    case CL_UNSIGNED_INT32:   return "CL_UNSIGNED_INT32  ";
    case CL_HALF_FLOAT:       return "CL_HALF_FLOAT      ";
    case CL_FLOAT:            return "CL_FLOAT           ";
    case CL_UNORM_INT24:      return "CL_UNORM_INT24     ";
    default:                  return "Unknown cl_channel_type";
  }
}
cl_int xOpenCL_Enumerator::xDevice::queryAllData()
{
  cl_int Result;
  Result = m_Device.getInfo(CL_DEVICE_TYPE,                                 &m_CL_DEVICE_TYPE);
  Result = m_Device.getInfo(CL_DEVICE_VENDOR_ID,                            &m_CL_DEVICE_VENDOR_ID);
  Result = m_Device.getInfo(CL_DEVICE_MAX_COMPUTE_UNITS,                    &m_CL_DEVICE_MAX_COMPUTE_UNITS);
  Result = m_Device.getInfo(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,             &m_CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
  Result = m_Device.getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE,                  &m_CL_DEVICE_MAX_WORK_GROUP_SIZE);
  Result = m_Device.getInfo(CL_DEVICE_MAX_WORK_ITEM_SIZES,                  &m_CL_DEVICE_MAX_WORK_ITEM_SIZES);
  Result = m_Device.getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,          &m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR);
  Result = m_Device.getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,         &m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT);
  Result = m_Device.getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,           &m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT);
  Result = m_Device.getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG,          &m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG);
  Result = m_Device.getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,         &m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT);
  Result = m_Device.getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,        &m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE);
  Result = m_Device.getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY,                  &m_CL_DEVICE_MAX_CLOCK_FREQUENCY);
  Result = m_Device.getInfo(CL_DEVICE_ADDRESS_BITS,                         &m_CL_DEVICE_ADDRESS_BITS);
  Result = m_Device.getInfo(CL_DEVICE_MAX_READ_IMAGE_ARGS,                  &m_CL_DEVICE_MAX_READ_IMAGE_ARGS);
  Result = m_Device.getInfo(CL_DEVICE_MAX_WRITE_IMAGE_ARGS,                 &m_CL_DEVICE_MAX_WRITE_IMAGE_ARGS);
  Result = m_Device.getInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE,                   &m_CL_DEVICE_MAX_MEM_ALLOC_SIZE);
  Result = m_Device.getInfo(CL_DEVICE_IMAGE2D_MAX_WIDTH,                    &m_CL_DEVICE_IMAGE2D_MAX_WIDTH);
  Result = m_Device.getInfo(CL_DEVICE_IMAGE2D_MAX_HEIGHT,                   &m_CL_DEVICE_IMAGE2D_MAX_HEIGHT);
  Result = m_Device.getInfo(CL_DEVICE_IMAGE3D_MAX_WIDTH,                    &m_CL_DEVICE_IMAGE3D_MAX_WIDTH);
  Result = m_Device.getInfo(CL_DEVICE_IMAGE3D_MAX_HEIGHT,                   &m_CL_DEVICE_IMAGE3D_MAX_HEIGHT);
  Result = m_Device.getInfo(CL_DEVICE_IMAGE3D_MAX_DEPTH,                    &m_CL_DEVICE_IMAGE3D_MAX_DEPTH);
  Result = m_Device.getInfo(CL_DEVICE_IMAGE_SUPPORT,                        &m_CL_DEVICE_IMAGE_SUPPORT);
  Result = m_Device.getInfo(CL_DEVICE_MAX_PARAMETER_SIZE,                   &m_CL_DEVICE_MAX_PARAMETER_SIZE);
  Result = m_Device.getInfo(CL_DEVICE_MAX_SAMPLERS,                         &m_CL_DEVICE_MAX_SAMPLERS);
  Result = m_Device.getInfo(CL_DEVICE_MEM_BASE_ADDR_ALIGN,                  &m_CL_DEVICE_MEM_BASE_ADDR_ALIGN);
  Result = m_Device.getInfo(CL_DEVICE_SINGLE_FP_CONFIG,                     &m_CL_DEVICE_SINGLE_FP_CONFIG);
  Result = m_Device.getInfo(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,                &m_CL_DEVICE_GLOBAL_MEM_CACHE_TYPE);
  Result = m_Device.getInfo(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE,            &m_CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
  Result = m_Device.getInfo(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,                &m_CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);
  Result = m_Device.getInfo(CL_DEVICE_GLOBAL_MEM_SIZE,                      &m_CL_DEVICE_GLOBAL_MEM_SIZE);
  Result = m_Device.getInfo(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,             &m_CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
  Result = m_Device.getInfo(CL_DEVICE_MAX_CONSTANT_ARGS,                    &m_CL_DEVICE_MAX_CONSTANT_ARGS);
  Result = m_Device.getInfo(CL_DEVICE_LOCAL_MEM_TYPE,                       &m_CL_DEVICE_LOCAL_MEM_TYPE);
  Result = m_Device.getInfo(CL_DEVICE_LOCAL_MEM_SIZE,                       &m_CL_DEVICE_LOCAL_MEM_SIZE);
  Result = m_Device.getInfo(CL_DEVICE_ERROR_CORRECTION_SUPPORT,             &m_CL_DEVICE_ERROR_CORRECTION_SUPPORT);
  Result = m_Device.getInfo(CL_DEVICE_ENDIAN_LITTLE,                        &m_CL_DEVICE_ENDIAN_LITTLE);
  Result = m_Device.getInfo(CL_DEVICE_AVAILABLE,                            &m_CL_DEVICE_AVAILABLE);
  Result = m_Device.getInfo(CL_DEVICE_COMPILER_AVAILABLE,                   &m_CL_DEVICE_COMPILER_AVAILABLE);
  Result = m_Device.getInfo(CL_DEVICE_EXECUTION_CAPABILITIES,               &m_CL_DEVICE_EXECUTION_CAPABILITIES);
  Result = m_Device.getInfo(CL_DEVICE_NAME,                                 &m_CL_DEVICE_NAME);
  Result = m_Device.getInfo(CL_DEVICE_VENDOR,                               &m_CL_DEVICE_VENDOR);
  Result = m_Device.getInfo(CL_DRIVER_VERSION,                              &m_CL_DRIVER_VERSION);
  Result = m_Device.getInfo(CL_DEVICE_PROFILE,                              &m_CL_DEVICE_PROFILE);
  Result = m_Device.getInfo(CL_DEVICE_VERSION,                              &m_CL_DEVICE_VERSION);
  string DeviceExtensions;
  Result = m_Device.getInfo(CL_DEVICE_EXTENSIONS, &DeviceExtensions);
  std::istringstream StringStream(DeviceExtensions);
  copy(std::istream_iterator<string>{StringStream}, std::istream_iterator<string>(), back_inserter(m_CL_DEVICE_EXTENSIONS));  
  Result = m_Device.getInfo(CL_DEVICE_PLATFORM,                             &m_CL_DEVICE_PLATFORM);
  Result = m_Device.getInfo(CL_DEVICE_DOUBLE_FP_CONFIG,                     &m_CL_DEVICE_DOUBLE_FP_CONFIG);
  Result = m_Device.getInfo(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF,          &m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF);
  //Result = m_Device.getInfo(CL_DEVICE_HOST_UNIFIED_MEMORY,                  &m_CL_DEVICE_HOST_UNIFIED_MEMORY);
  Result = m_Device.getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR,             &m_CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR);
  Result = m_Device.getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT,            &m_CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT);
  Result = m_Device.getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT,              &m_CL_DEVICE_NATIVE_VECTOR_WIDTH_INT);
  Result = m_Device.getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG,             &m_CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG);
  Result = m_Device.getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,            &m_CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT);
  Result = m_Device.getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE,           &m_CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE);
  Result = m_Device.getInfo(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF,             &m_CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF);
  Result = m_Device.getInfo(CL_DEVICE_OPENCL_C_VERSION,                     &m_CL_DEVICE_OPENCL_C_VERSION);
  Result = m_Device.getInfo(CL_DEVICE_LINKER_AVAILABLE,                     &m_CL_DEVICE_LINKER_AVAILABLE);
  Result = m_Device.getInfo(CL_DEVICE_BUILT_IN_KERNELS,                     &m_CL_DEVICE_BUILT_IN_KERNELS);
  Result = m_Device.getInfo(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE,                &m_CL_DEVICE_IMAGE_MAX_BUFFER_SIZE);
  Result = m_Device.getInfo(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE,                 &m_CL_DEVICE_IMAGE_MAX_ARRAY_SIZE);

  cl::Context Context(m_Device);
  Result = Context.getSupportedImageFormats(CL_MEM_READ_WRITE,     CL_MEM_OBJECT_IMAGE2D, &(m_ImageFormats[CL_MEM_READ_WRITE    ]));
  Result = Context.getSupportedImageFormats(CL_MEM_WRITE_ONLY,     CL_MEM_OBJECT_IMAGE2D, &(m_ImageFormats[CL_MEM_WRITE_ONLY    ]));
  Result = Context.getSupportedImageFormats(CL_MEM_READ_ONLY,      CL_MEM_OBJECT_IMAGE2D, &(m_ImageFormats[CL_MEM_READ_ONLY     ]));
  Result = Context.getSupportedImageFormats(CL_MEM_USE_HOST_PTR,   CL_MEM_OBJECT_IMAGE2D, &(m_ImageFormats[CL_MEM_USE_HOST_PTR  ]));
  Result = Context.getSupportedImageFormats(CL_MEM_ALLOC_HOST_PTR, CL_MEM_OBJECT_IMAGE2D, &(m_ImageFormats[CL_MEM_ALLOC_HOST_PTR]));
  Result = Context.getSupportedImageFormats(CL_MEM_COPY_HOST_PTR,  CL_MEM_OBJECT_IMAGE2D, &(m_ImageFormats[CL_MEM_COPY_HOST_PTR ]));
  return CL_SUCCESS;
}
string xOpenCL_Enumerator::xDevice::printAllData(string Prefix, bool ListImageFormats)
{
  string Result;

  Result += Prefix + "Device info - general:\n";
  Result += Prefix + "CL_DEVICE_PROFILE:                      " + m_CL_DEVICE_PROFILE + "\n";
  Result += Prefix + "CL_DEVICE_VERSION:                      " + m_CL_DEVICE_VERSION + "\n";
  Result += Prefix + "CL_DEVICE_NAME:                         " + m_CL_DEVICE_NAME + "\n";
  Result += Prefix + "CL_DEVICE_TYPE:                         " + cl_device_typeToString(m_CL_DEVICE_TYPE) + "\n";
  Result += Prefix + "CL_DEVICE_VENDOR:                       " + m_CL_DEVICE_VENDOR + "\n";
  Result += Prefix + "CL_DEVICE_VENDOR_ID:                    " + std::to_string(m_CL_DEVICE_VENDOR_ID) + "\n";
  Result += Prefix + "CL_DEVICE_EXTENSIONS:                   "; for(string Extension : m_CL_DEVICE_EXTENSIONS) { Result += Extension + " "; }; Result += "\n";
  Result += Prefix + "CL_DRIVER_VERSION:                      " + m_CL_DEVICE_PROFILE + "\n";
  //m_CL_DEVICE_PLATFORM
  Result += Prefix + "CL_DEVICE_OPENCL_C_VERSION:             " + m_CL_DEVICE_OPENCL_C_VERSION + "\n";
  Result += Prefix + "CL_DEVICE_BUILT_IN_KERNELS:             " + m_CL_DEVICE_BUILT_IN_KERNELS + "\n";
  Result += Prefix + "CL_DEVICE_EXECUTION_CAPABILITIES:       " + cl_device_exec_capabilitiesToString(m_CL_DEVICE_EXECUTION_CAPABILITIES) + "\n";

  Result += Prefix + "Device info - availability:\n";
  Result += Prefix + "CL_DEVICE_AVAILABLE:                    " + std::to_string(m_CL_DEVICE_AVAILABLE) + "\n";
  Result += Prefix + "CL_DEVICE_COMPILER_AVAILABLE:           " + std::to_string(m_CL_DEVICE_COMPILER_AVAILABLE) + "\n";
  Result += Prefix + "CL_DEVICE_LINKER_AVAILABLE:             " + std::to_string(m_CL_DEVICE_LINKER_AVAILABLE) + "\n";

  Result += Prefix + "Device info - performance:\n";
  Result += Prefix + "CL_DEVICE_ADDRESS_BITS:                 " + std::to_string(m_CL_DEVICE_ADDRESS_BITS) + "\n";
  Result += Prefix + "CL_DEVICE_MAX_CLOCK_FREQUENCY:          " + std::to_string(m_CL_DEVICE_MAX_CLOCK_FREQUENCY) + "\n";
  Result += Prefix + "CL_DEVICE_MAX_COMPUTE_UNITS:            " + std::to_string(m_CL_DEVICE_MAX_COMPUTE_UNITS) + "\n";
  Result += Prefix + "CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:     " + std::to_string(m_CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS) + "\n";
  Result += Prefix + "CL_DEVICE_MAX_WORK_GROUP_SIZE:          " + std::to_string(m_CL_DEVICE_MAX_WORK_GROUP_SIZE) + "\n";
  Result += Prefix + "CL_DEVICE_MAX_WORK_ITEM_SIZES:          "; for(size_t Size : m_CL_DEVICE_MAX_WORK_ITEM_SIZES) { Result += std::to_string(Size) + " "; } Result += "\n";

  Result += Prefix + "Device info - image:\n";
  Result += Prefix + "CL_DEVICE_IMAGE_SUPPORT:                " + std::to_string(m_CL_DEVICE_IMAGE_SUPPORT) + "\n";
  Result += Prefix + "CL_DEVICE_IMAGE2D_MAX_:                 " + "WIDTH=" + std::to_string(m_CL_DEVICE_IMAGE2D_MAX_WIDTH) + " HEIGHT=" + std::to_string(m_CL_DEVICE_IMAGE2D_MAX_HEIGHT) + "\n";
  Result += Prefix + "CL_DEVICE_IMAGE3D_MAX_:                 " + "WIDTH=" + std::to_string(m_CL_DEVICE_IMAGE3D_MAX_WIDTH) + " HEIGHT=" + std::to_string(m_CL_DEVICE_IMAGE3D_MAX_HEIGHT) + " DEPTH=" + std::to_string(m_CL_DEVICE_IMAGE3D_MAX_DEPTH) + "\n";
  Result += Prefix + "CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:        " + std::to_string(m_CL_DEVICE_IMAGE_MAX_BUFFER_SIZE       ) + "\n";
  Result += Prefix + "CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:         " + std::to_string(m_CL_DEVICE_IMAGE_MAX_ARRAY_SIZE        ) + "\n";
  Result += Prefix + "CL_DEVICE_IMAGE_PITCH_ALIGNMENT:        " + std::to_string(m_CL_DEVICE_IMAGE_PITCH_ALIGNMENT       ) + "\n";
  Result += Prefix + "CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT: " + std::to_string(m_CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT) + "\n";

  Result += Prefix + "Device info - vector width:\n";
  Result += Prefix + "DEVICE_NATIVE_VECTOR_WIDTH_:            " + "CHAR=" + std::to_string(m_CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR) + " SHORT=" + std::to_string(m_CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT) + " INT=" + std::to_string(m_CL_DEVICE_NATIVE_VECTOR_WIDTH_INT) + " LONG=" + std::to_string(m_CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG) + " HALF=" + std::to_string(m_CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF) + " FLOAT=" + std::to_string(m_CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT) + " DOUBLE=" + std::to_string(m_CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE) + "\n";
  Result += Prefix + "DEVICE_PREFERRED_VECTOR_WIDTH_:         " + "CHAR=" + std::to_string(m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR) + " SHORT=" + std::to_string(m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT) + " INT=" + std::to_string(m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT) + " LONG=" + std::to_string(m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG) + " HALF=" + std::to_string(m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF) + " FLOAT=" + std::to_string(m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT) + " DOUBLE=" + std::to_string(m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE) + "\n";
  
  if(ListImageFormats)
  {
    Result += Prefix + "Device context - Image formats:\n";
    Result += Prefix + "Image formats CL_MEM_READ_WRITE:\n";
    for(cl::ImageFormat ImageFormat : m_ImageFormats[CL_MEM_READ_WRITE]) { Result += Prefix +  " ChanellOrder: " + cl_channel_orderToString(ImageFormat.image_channel_order) + "     ChanellType: " + cl_channel_typeToString(ImageFormat.image_channel_data_type) + "\n"; }
    Result += Prefix + "Image formats CL_MEM_WRITE_ONLY:\n";
    for(cl::ImageFormat ImageFormat : m_ImageFormats[CL_MEM_WRITE_ONLY]) { Result += Prefix +  " ChanellOrder: " + cl_channel_orderToString(ImageFormat.image_channel_order) + "     ChanellType: " + cl_channel_typeToString(ImageFormat.image_channel_data_type) + "\n"; }
    Result += Prefix + "Image formats CL_MEM_READ_ONLY:\n";
    for(cl::ImageFormat ImageFormat : m_ImageFormats[CL_MEM_READ_ONLY ]) { Result += Prefix +  " ChanellOrder: " + cl_channel_orderToString(ImageFormat.image_channel_order) + "     ChanellType: " + cl_channel_typeToString(ImageFormat.image_channel_data_type) + "\n"; }
    
    Result += Prefix + "Image formats CL_MEM_USE_HOST_PTR:\n";
    for(cl::ImageFormat ImageFormat : m_ImageFormats[CL_MEM_USE_HOST_PTR  ]) { Result += Prefix +  " ChanellOrder: " + cl_channel_orderToString(ImageFormat.image_channel_order) + "     ChanellType: " + cl_channel_typeToString(ImageFormat.image_channel_data_type) + "\n"; }
    Result += Prefix + "Image formats CL_MEM_ALLOC_HOST_PTR:\n";
    for(cl::ImageFormat ImageFormat : m_ImageFormats[CL_MEM_ALLOC_HOST_PTR]) { Result += Prefix +  " ChanellOrder: " + cl_channel_orderToString(ImageFormat.image_channel_order) + "     ChanellType: " + cl_channel_typeToString(ImageFormat.image_channel_data_type) + "\n"; }
    Result += Prefix + "Image formats CL_MEM_COPY_HOST_PTR:\n";
    for(cl::ImageFormat ImageFormat : m_ImageFormats[CL_MEM_COPY_HOST_PTR ]) { Result += Prefix +  " ChanellOrder: " + cl_channel_orderToString(ImageFormat.image_channel_order) + "     ChanellType: " + cl_channel_typeToString(ImageFormat.image_channel_data_type) + "\n"; }
  }

  return Result;
}

//=====================================================================================================================================================================================

