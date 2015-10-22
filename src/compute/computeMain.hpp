// Init OpenCL

#ifndef COMPUTE_COMPUTEMAIN_HPP
#define COMPUTE_COMPUTEMAIN_HPP

#include <vector>
#include <map>
#include <memory>
#include "render/vertexArray.hpp"
#include "compute/opencl.hpp"
#include "tools/log.hpp"

void clTest(cl_int err, const std::string& msg);

class CLQueue
{
public:
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_command_queue handle;

	void printInfo();
};

void createQueues(CLQueue& cpuQueue, CLQueue& gpuQueue);

/*
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

template<class T>
class CLBuffer
{
public:
	void read(cl_command_queue& queue, std::vector<T>& buffer);
	void write(cl_command_queue& queue, const std::vector<T>& buffer);

	cl_mem handle;
	int size {0};
};

template<class T>
class CLVertexBuffer : public CLBuffer<T>
{
public:
	CLVertexBuffer(CLMaster& master, SingleVertexArray<T>& va);

	void acquire(cl_command_queue& queue);
	void release(cl_command_queue& queue);
};
*/

// ------------------------------------
// IMPLEMENTATION
// ------------------------------------
/*
template<class T>
void CLBuffer<T>::read(cl_command_queue& queue, std::vector<T>& buffer)
{
	buffer.resize(size);
	clTest(clEnqueueReadBuffer(queue, handle, CL_TRUE, 0, sizeof(T)*size, 
							   &buffer[0], 0, nullptr, nullptr), "read buffer");
}

template<class T>
void CLBuffer<T>::write(cl_command_queue& queue, const std::vector<T>& buffer)
{
	size = buffer.size();
	clTest(clEnqueueWriteBuffer(queue, handle, CL_TRUE, 0, sizeof(T)*size, 
						 	    &buffer[0], 0, nullptr, nullptr), "write buffer");
}

template<class T>
CLVertexBuffer<T>::CLVertexBuffer(CLMaster& master, SingleVertexArray<T>& va) 
{
	cl_int err;
	this->size = va.buffer.size;
	this->handle = clCreateFromGLBuffer(master.context, CL_MEM_WRITE_ONLY, va.buffer.handle, &err);
	clTest(err, "create cl/gl buffer");
}

template<class T>
void CLKernel::setArg(int idx, const T& value) 
{
	clTest(clSetKernelArg(handle, idx, sizeof(T), &value), "set arg");
}

template<class T>
void CLVertexBuffer<T>::acquire(cl_command_queue& queue) 
{
	clTest(clEnqueueAcquireGLObjects(queue, 1, &this->handle, 0, 0, 0), "aquire gl");
}

template<class T>
void CLVertexBuffer<T>::release(cl_command_queue& queue) 
{
	clTest(clEnqueueReleaseGLObjects(queue, 1, &this->handle, 0, 0, 0), "release gl");
}
*/
#endif