// Init OpenCL

#ifndef COMPUTE_COMPUTEMAIN_HPP
#define COMPUTE_COMPUTEMAIN_HPP

#include <vector>
#include <map>
#include <cassert>
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

struct LocalBlock
{
	LocalBlock(int s) : size(s) {}
	int size;
};

class CLKernel
{
public:
	CLKernel(CLQueue& queue, const std::string& filename, const std::string& kernel);
	template<class T> void setArg(int idx, const T& value);
	template<typename T, typename... Args> void setArgs(const T& value, Args... args);
	void enqueue(size_t problem_size, size_t local = 1);

	CLQueue& queue;
	CLProgram& program;
	cl_kernel handle;	
private:
	template<typename T, typename... Args> void _setArgs(int idx, const T& value, Args... args);
	inline void _setArgs(int idx);
};

enum class BufferType { None = 0, Host = 1, Gpu = 2, Both = 3 };

template<class T>
class CLBuffer
{
public:
	CLBuffer(CLQueue& queue, int size, BufferType type);
	CLBuffer(CLQueue& queue) : queue(queue) {}
	virtual ~CLBuffer();
	void upload();
	void download();
	void swap(CLBuffer<T>& other);
	void setSize(int nsize);
	void fill(const T& value);

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
	CLVertexBuffer(CLQueue& queue, SingleVertexArray& va);

	void acquire();
	void release();
	void grow(int nsize);

	SingleVertexArray& vaLink;
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
	{
		this->handle = clCreateBuffer(queue.context, CL_MEM_READ_WRITE, sizeof(T)*size, nullptr, &err);
		clTest(err, "create cl buffer");
	}
	if (type == BufferType::Host || type == BufferType::Both)
	{
		buffer.resize(size);
	}
}

template<class T>
CLBuffer<T>::~CLBuffer()
{
	clReleaseMemObject(this->handle);
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
void CLBuffer<T>::swap(CLBuffer<T>& other)
{
	assert(&queue == &other.queue);
	assert(size == other.size);
	std::swap(handle, other.handle);
	buffer.swap(other.buffer);
}

template<class T>
void CLBuffer<T>::setSize(int nsize)
{
	cl_int err;
	this->size = nsize;
	if (type == BufferType::Gpu || type == BufferType::Both)
	{
		clReleaseMemObject(this->handle);
		this->handle = clCreateBuffer(queue.context, CL_MEM_READ_WRITE, sizeof(T)*size, nullptr, &err);
		clTest(err, "create cl");
	}
	if (type == BufferType::Host || type == BufferType::Both)
	{
		buffer.resize(size);
	}
}

template<class T>
void CLBuffer<T>::fill(const T& val)
{
	clTest(clEnqueueFillBuffer(queue.handle, this->handle, &val, sizeof(T), 0, size*sizeof(T), 0, 0, 0), "fill buffer");
}

template<class T>
CLVertexBuffer<T>::CLVertexBuffer(CLQueue& queue, SingleVertexArray& va) :
	CLBuffer(queue), vaLink(va)
{
	cl_int err;
	this->type = BufferType::Gpu;
	this->size = va.buffer.size / sizeof(T);
	this->handle = clCreateFromGLBuffer(queue.context, CL_MEM_WRITE_ONLY, va.buffer.handle, &err);
	clTest(err, "create cl/gl buffer");
}

template<class T> 
inline void* get_ptr(const T& el) { return (void*)&el; }

template<class T>
inline size_t get_size(const T& el) { return sizeof(T); }

template<>
inline void* get_ptr<LocalBlock>(const LocalBlock& el) { return nullptr; }

template<>
inline size_t get_size<LocalBlock>(const LocalBlock& el) { return el.size; }

template<class T>
void CLKernel::setArg(int idx, const T& value) 
{
	clTest(clSetKernelArg(handle, idx, get_size<T>(value), get_ptr<T>(value)), "set arg");
}

template<typename T, typename... Args> 
void CLKernel::setArgs(const T& value, Args... args)
{
	_setArgs(0, value, args...);
}

inline void CLKernel::_setArgs(int idx)
{
}

template<typename T, typename... Args>
void CLKernel::_setArgs(int idx, const T& value, Args... args)
{
	clTest(clSetKernelArg(handle, idx, get_size<T>(value), get_ptr<T>(value)), "set arg");
	_setArgs(idx+1, args...);
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

template<class T>
void CLVertexBuffer<T>::grow(int nsize)
{
	if (nsize <= size)
		return;

	vaLink.buffer.setSize(nsize * sizeof(T));
	this->size = nsize;
	clReleaseMemObject(this->handle);
	cl_int err;
	this->handle = clCreateFromGLBuffer(queue.context, CL_MEM_WRITE_ONLY, vaLink.buffer.handle, &err);
	clTest(err, "create from gl");
}

#endif