#pragma once

#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#define CL_HPP_TARGET_OPENCL_VERSION  120

#if __has_include("CL/opencl.hpp")
#  include "CL/opencl.hpp"
#else
#  include "CL/cl2.hpp"
#endif



