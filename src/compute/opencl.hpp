// Platform-dependent OpenCL includes

#ifndef COMPUTE_OPENCL_HPP
#define COMPUTE_OPENCL_HPP

#if defined(__APPLE__)
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif // !__APPLE__

#endif