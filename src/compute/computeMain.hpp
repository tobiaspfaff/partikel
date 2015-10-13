// Init OpenCL

#ifndef COMPUTE_COMPUTEMAIN_HPP
#define COMPUTE_COMPUTEMAIN_HPP

#include "compute/opencl.hpp"
#include <vector>
#include <map>
#include <memory>

class CLMaster
{
public:
	CLMaster();

	std::vector<cl_device_id> devices;
	cl_context context;
	cl_command_queue cpuQueue;
	cl_command_queue gpuQueue;
protected:
	void initialize();
};

class CLProgram
{
public:
	static CLProgram& get(CLMaster& master, const std::string& filename);

	cl_program handle;

protected:
	static std::map<std::string, CLProgram> instances;
};

class CLKernel
{
public:
	CLKernel(CLMaster& master, const std::string& filename, const std::string& kernel);

	CLProgram& program;
	cl_kernel handle;
};

class CLVertexBuffer
{
	
};

#endif