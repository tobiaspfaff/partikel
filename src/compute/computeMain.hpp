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

class CLProgram
{
public:
	static CLProgram& get(CLQueue& queue, const std::string& filename);

	cl_program handle;

protected:
	static std::map<std::string, CLProgram> instances;
};
 
class CLKernel
{
public:
	CLKernel(CLQueue& queue, const std::string& filename, const std::string& kernel);
	template<class T> void setArg(int idx, const T& value);
	void enqueue(size_t problem_size);

	CLQueue& queue;
	CLProgram& program;
	cl_kernel handle;	
};

enum class BufferType { None = 0, Host = 1, Gpu = 2, Both = 3 };

template<class T>
class CLBuffer
{
public:
	CLBuffer(CLQueue& queue, int size, BufferType type);
	CLBuffer(CLQueue& queue) : queue(queue) {}
	void upload();
	void download();

	std::vector<T> buffer;
	BufferType type = BufferType::None;
	CLQueue& queue;
	cl_mem handle = 0;
	int size = 0;
};

template<class T>
class CLVertexBuffer : public CLBuffer<T>
{
public:
	CLVertexBuffer(CLQueue& queue, SingleVertexArray<T>& va);

	void acquire();
	void release();
};

// ------------------------------------
// IMPLEMENTATION
// ------------------------------------

template<class T>
CLBuffer<T>::CLBuffer(CLQueue& queue, int size, BufferType type) : 
	queue(queue), size(size), type(type)
{
	cl_int err;
	if (type == BufferType::Gpu || type == BufferType::Both)
		this->handle = clCreateBuffer(queue.context, CL_MEM_READ_WRITE, sizeof(T)*size, nullptr, &err);
	if (type == BufferType::Host || type == BufferType::Both)
		buffer.resize(size);
	clTest(err, "create cl buffer");
}

template<class T>
void CLBuffer<T>::download()
{
	if (type != BufferType::Both)
		fatalError("Try to read from a Host/GPU only buffer");
	clTest(clEnqueueReadBuffer(queue.handle, handle, CL_TRUE, 0, sizeof(T)*size, 
							   &buffer[0], 0, nullptr, nullptr), "read buffer");
}

template<class T>
void CLBuffer<T>::upload()
{
	if (type != BufferType::Both)
		fatalError("Try to write to a Host/GPU only buffer");
	clTest(clEnqueueWriteBuffer(queue.handle, handle, CL_TRUE, 0, sizeof(T)*size,
						 	    &buffer[0], 0, nullptr, nullptr), "write buffer");
}

template<class T>
CLVertexBuffer<T>::CLVertexBuffer(CLQueue& queue, SingleVertexArray<T>& va) :
	CLBuffer(queue)
{
	cl_int err;
	this->type = BufferType::Gpu;
	this->size = va.buffer.size;
	this->handle = clCreateFromGLBuffer(queue.context, CL_MEM_WRITE_ONLY, va.buffer.handle, &err);
	clTest(err, "create cl/gl buffer");
}

template<class T>
void CLKernel::setArg(int idx, const T& value) 
{
	clTest(clSetKernelArg(handle, idx, sizeof(T), &value), "set arg");
}

template<class T>
void CLVertexBuffer<T>::acquire() 
{
	clTest(clEnqueueAcquireGLObjects(queue.handle, 1, &this->handle, 0, 0, 0), "aquire gl");
}

template<class T>
void CLVertexBuffer<T>::release() 
{
	clTest(clEnqueueReleaseGLObjects(queue.handle, 1, &this->handle, 0, 0, 0), "release gl");
}

#endif