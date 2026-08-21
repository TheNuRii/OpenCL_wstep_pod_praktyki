#pragma once
#include "CommonDef.h"
#include "xOpenCL_Common.h"
#include <map>

//=====================================================================================================================================================================================

class xOpenCL_Enumerator
{
public:
  class xDevice;
  class xPlatform;

protected:
  std::vector<xPlatform> m_Platforms;

public:
  cl_int      queryAllData();
  std::string printAllData(bool ListImageFormats);

  static cl::Device   findMachingDevice  (cl_device_type DeviceType, std::string Vendor, int32 Version);  
  static cl_device_id findMachingDeviceId(cl_device_type DeviceType, std::string Vendor, int32 Version) { return findMachingDevice(DeviceType, Vendor, Version).get(); }
};

//=============================================================================================================================================================================

class xOpenCL_Enumerator::xPlatform
{
public:
  cl::Platform         m_Platform;
  std::vector<xDevice> m_Devices;

  std::string              m_CL_PLATFORM_PROFILE;
  std::string              m_CL_PLATFORM_VERSION; 
  std::string              m_CL_PLATFORM_NAME;
  std::string              m_CL_PLATFORM_VENDOR;
  std::vector<std::string> m_CL_PLATFORM_EXTENSIONS;     

public:
  cl_int      queryAllData();
  std::string printAllData(std::string Prefix, bool ListImageFormats);
};

//=============================================================================================================================================================================

class xOpenCL_Enumerator::xDevice
{
public:
  cl::Device m_Device;
  std::map<cl_mem_flags, std::vector<cl::ImageFormat> > m_ImageFormats;

  cl_device_type              m_CL_DEVICE_TYPE;
  cl_uint                     m_CL_DEVICE_VENDOR_ID;
  cl_uint                     m_CL_DEVICE_MAX_COMPUTE_UNITS;
  cl_uint                     m_CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS;
  size_t                      m_CL_DEVICE_MAX_WORK_GROUP_SIZE;
  std::vector<size_t>         m_CL_DEVICE_MAX_WORK_ITEM_SIZES;
  cl_uint                     m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR;
  cl_uint                     m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT;
  cl_uint                     m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT;
  cl_uint                     m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG;
  cl_uint                     m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT;
  cl_uint                     m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE;
  cl_uint                     m_CL_DEVICE_MAX_CLOCK_FREQUENCY;
  cl_uint                     m_CL_DEVICE_ADDRESS_BITS;
  cl_uint                     m_CL_DEVICE_MAX_READ_IMAGE_ARGS;
  cl_uint                     m_CL_DEVICE_MAX_WRITE_IMAGE_ARGS;
  cl_ulong                    m_CL_DEVICE_MAX_MEM_ALLOC_SIZE;
  size_t                      m_CL_DEVICE_IMAGE2D_MAX_WIDTH;
  size_t                      m_CL_DEVICE_IMAGE2D_MAX_HEIGHT;
  size_t                      m_CL_DEVICE_IMAGE3D_MAX_WIDTH;
  size_t                      m_CL_DEVICE_IMAGE3D_MAX_HEIGHT;  
  size_t                      m_CL_DEVICE_IMAGE3D_MAX_DEPTH;
  cl_bool                     m_CL_DEVICE_IMAGE_SUPPORT;
  size_t                      m_CL_DEVICE_MAX_PARAMETER_SIZE;
  cl_uint                     m_CL_DEVICE_MAX_SAMPLERS;
  cl_uint                     m_CL_DEVICE_MEM_BASE_ADDR_ALIGN;
  cl_device_fp_config         m_CL_DEVICE_SINGLE_FP_CONFIG;
  cl_device_mem_cache_type    m_CL_DEVICE_GLOBAL_MEM_CACHE_TYPE;
  cl_uint                     m_CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE;
  cl_ulong                    m_CL_DEVICE_GLOBAL_MEM_CACHE_SIZE;
  cl_ulong                    m_CL_DEVICE_GLOBAL_MEM_SIZE;
  cl_ulong                    m_CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE;
  cl_uint                     m_CL_DEVICE_MAX_CONSTANT_ARGS;
  cl_device_local_mem_type    m_CL_DEVICE_LOCAL_MEM_TYPE;
  cl_ulong                    m_CL_DEVICE_LOCAL_MEM_SIZE;
  cl_bool                     m_CL_DEVICE_ERROR_CORRECTION_SUPPORT;
//#define CL_DEVICE_PROFILING_TIMER_RESOLUTION            0x1025
  cl_bool                     m_CL_DEVICE_ENDIAN_LITTLE;
  cl_bool                     m_CL_DEVICE_AVAILABLE;
  cl_bool                     m_CL_DEVICE_COMPILER_AVAILABLE;
  cl_device_exec_capabilities m_CL_DEVICE_EXECUTION_CAPABILITIES;
//#define CL_DEVICE_QUEUE_ON_HOST_PROPERTIES              0x102A
  std::string                 m_CL_DEVICE_NAME;
  std::string                 m_CL_DEVICE_VENDOR;
  std::string                 m_CL_DRIVER_VERSION;
  std::string                 m_CL_DEVICE_PROFILE;
  std::string                 m_CL_DEVICE_VERSION;
  std::vector<std::string>    m_CL_DEVICE_EXTENSIONS;
  cl_platform_id              m_CL_DEVICE_PLATFORM;
  cl_device_fp_config         m_CL_DEVICE_DOUBLE_FP_CONFIG;
  //  /* 0x1033 reserved for CL_DEVICE_HALF_FP_CONFIG */
  cl_uint                     m_CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF;
//#define CL_DEVICE_HOST_UNIFIED_MEMORY                   0x1035   /* deprecated */
  cl_uint                     m_CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR;
  cl_uint                     m_CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT;
  cl_uint                     m_CL_DEVICE_NATIVE_VECTOR_WIDTH_INT;
  cl_uint                     m_CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG;
  cl_uint                     m_CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT;
  cl_uint                     m_CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE;
  cl_uint                     m_CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF;
  std::string                 m_CL_DEVICE_OPENCL_C_VERSION;
  cl_bool                     m_CL_DEVICE_LINKER_AVAILABLE;
  std::string                 m_CL_DEVICE_BUILT_IN_KERNELS;
  size_t                      m_CL_DEVICE_IMAGE_MAX_BUFFER_SIZE;
  size_t                      m_CL_DEVICE_IMAGE_MAX_ARRAY_SIZE;
//#define CL_DEVICE_PARENT_DEVICE                         0x1042
//#define CL_DEVICE_PARTITION_MAX_SUB_DEVICES             0x1043
//#define CL_DEVICE_PARTITION_PROPERTIES                  0x1044
//#define CL_DEVICE_PARTITION_AFFINITY_DOMAIN             0x1045
//#define CL_DEVICE_PARTITION_TYPE                        0x1046
//#define CL_DEVICE_REFERENCE_COUNT                       0x1047
//#define CL_DEVICE_PREFERRED_INTEROP_USER_SYNC           0x1048
//#define CL_DEVICE_PRINTF_BUFFER_SIZE                    0x1049
  cl_uint                     m_CL_DEVICE_IMAGE_PITCH_ALIGNMENT;
  cl_uint                     m_CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT;
  cl_uint                     m_CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS;
  size_t                      m_CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE;
//#define CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES            0x104E
//#define CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE        0x104F
//#define CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE              0x1050
//#define CL_DEVICE_MAX_ON_DEVICE_QUEUES                  0x1051
  cl_uint                     m_CL_DEVICE_MAX_ON_DEVICE_EVENTS;  
//cl_device_svm_capabilities  m_CL_DEVICE_SVM_CAPABILITIES;
  size_t                      m_CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE;
//#define CL_DEVICE_MAX_PIPE_ARGS                         0x1055
//#define CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS          0x1056
//#define CL_DEVICE_PIPE_MAX_PACKET_SIZE                  0x1057
//#define CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT   0x1058
//#define CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT     0x1059
//#define CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT      0x105A

public:
  cl_int      queryAllData();
  std::string printAllData(std::string Prefix, bool ListImageFormats);

public:
  static std::string cl_device_typeToString(cl_device_type DeviceType);
  static std::string cl_device_exec_capabilitiesToString(cl_device_exec_capabilities DeviceExecCapabilities);
  static std::string cl_channel_orderToString(cl_channel_order ChanellOrder);
  static std::string cl_channel_typeToString(cl_channel_type ChanellType);
};

//=====================================================================================================================================================================================
