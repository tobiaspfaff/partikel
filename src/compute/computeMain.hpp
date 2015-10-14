// Init OpenCL

#ifndef COMPUTE_COMPUTEMAIN_HPP
#define COMPUTE_COMPUTEMAIN_HPP

#include <vector>
#include <map>
#include <memory>
#include "render/vertexArray.hpp"
#include "compute/opencl.hpp"
#include "tools/log.hpp"

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
	template<class T> void setArg(int idx, const T& value);
	void enqueue(cl_command_queue& queue, size_t problem_size);

	CLProgram& program;
	cl_kernel handle;	
};

class CLBuffer
{
public:
	CLBuffer(CLMaster& master) {}

	cl_mem handle;
};

template<class T>
class CLVertexBuffer : public CLBuffer
{
public:
	CLVertexBuffer(CLMaster& master, SingleVertexArray<T>& va);

	void acquire(cl_command_queue& queue);
	void release(cl_command_queue& queue);
};


// ------------------------------------
// IMPLEMENTATION
// ------------------------------------

template<class T>
CLVertexBuffer<T>::CLVertexBuffer(CLMaster& master, SingleVertexArray<T>& va) :
	CLBuffer(master)
{
	cl_int err;
	handle = clCreateFromGLBuffer(master.context, CL_MEM_WRITE_ONLY, va.buffer.handle, &err);
	if(err != CL_SUCCESS)
		fatalError("OpenCL error: create cl/gl buffer");
}

template<class T>
void CLKernel::setArg(int idx, const T& value) 
{
	cl_int err = clSetKernelArg(handle, idx, sizeof(T), &value);
	if (err != CL_SUCCESS)
		fatalError("Can't set OpenCL kernel argument");
}

template<class T>
void CLVertexBuffer<T>::acquire(cl_command_queue& queue) 
{
	cl_int err = clEnqueueAcquireGLObjects(queue, 1, &handle, 0, 0, 0);
	if (err != CL_SUCCESS)
		fatalError("Can't set OpenCL kernel argument");
}

template<class T>
void CLVertexBuffer<T>::release(cl_command_queue& queue) 
{
	cl_int err = clEnqueueReleaseGLObjects(queue, 1, &handle, 0, 0, 0);
	if (err != CL_SUCCESS)
		fatalError("Can't set OpenCL kernel argument");
}

#endif