// Init OpenCL

#include <iostream>
#include <fstream>
#include <iterator>
#include "render/opengl.hpp"
#include "compute/computeMain.hpp"
#include "tools/log.hpp"
#if defined (_WIN32)
	#define WINDOWS_LEAN_AND_MEAN
	#define NOMINMAX
	#include <windows.h>
#endif

using namespace std;

void clTest(cl_int err, const string& msg) 
{
	if (err != CL_SUCCESS)
		fatalError("OpenCL error: " + msg);
}

static cl_device_id findCPUDevice(cl_platform_id platform) 
{
	// Enumerate devices
	cl_device_id allDevices[16];
	cl_uint num;
	clTest(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 16, allDevices, &num), "list devices");
	for (int i = 0; i < num; i++)
	{
		size_t size;
		cl_device_type type;
		clGetDeviceInfo(allDevices[i], CL_DEVICE_TYPE, sizeof(type), &type, &size);
		if (type == CL_DEVICE_TYPE_CPU)
			return allDevices[i];
	}
	return nullptr;
}

static vector<cl_context_properties> renderContextProps(cl_platform_id platform) 
{
	vector<cl_context_properties> props;	
#if defined (__APPLE__)
	props.push_back(CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE);
	props.push_back((intptr_t)CGLGetShareGroup(CGLGetCurrentContext()));	
#elif defined (WIN32)
	props.push_back(CL_GL_CONTEXT_KHR);
	props.push_back((cl_context_properties)wglGetCurrentContext());
	props.push_back(CL_WGL_HDC_KHR);
	props.push_back((cl_context_properties)wglGetCurrentDC());
#else
	props.push_back(CL_GL_CONTEXT_KHR);
	props.push_back((cl_context_properties)glXGetCurrentContext());
	props.push_back(CL_GLX_DISPLAY_KHR);
	props.push_back((cl_context_properties)glXGetCurrentDisplay());	
#endif
	props.push_back(CL_CONTEXT_PLATFORM);
	props.push_back((cl_context_properties)platform);
	props.push_back(0);
	return props;
}

static cl_device_id findRenderDevice(cl_platform_id platform) {
	cl_device_id renderer;
	cl_context_properties props[16];
#if defined (__APPLE__)
	CGLContextObj glContext = CGLGetCurrentContext();
	CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);

	context = clCreateContext(&renderContextProps(platform)[0], 0, NULL, 0, 0, &err);
	clTest(err, "create gl context");

	clTest(clGetGLContextInfoAPPLE(context, glContext, CL_CGL_DEVICE_FOR_CURRENT_VIRTUAL_SCREEN_APPLE,
		sizeof(renderer), &renderer, NULL), "get gl context");
	clReleaseContext(context);
#elif defined (WIN32)
	auto fn = (clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddressForPlatform(platform, "clGetGLContextInfoKHR");
	clTest(fn(&renderContextProps(platform)[0], CL_DEVICES_FOR_GL_CONTEXT_KHR, sizeof(renderer), &renderer, NULL), "get gl context");
#else
	
#endif
	cl_device_type type;
	size_t size;
	clGetDeviceInfo(renderer, CL_DEVICE_TYPE, sizeof(type), &type, &size);
	if (type != CL_DEVICE_TYPE_GPU)
		return nullptr;
	
	return renderer;
}

void CLQueue::printInfo() {
	cl_device_type type;
	size_t size;
	char platformName[256], platformVersion[256];
	char deviceName[256], deviceVendor[256];
	clGetPlatformInfo(platform, CL_PLATFORM_NAME, 256, platformName, &size);
	clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 256, platformVersion, &size);
	clGetDeviceInfo(device, CL_DEVICE_VENDOR, 256, deviceVendor, &size);
	clGetDeviceInfo(device, CL_DEVICE_NAME, 256, deviceName, &size);
	clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, &size);

	cout << ((type == CL_DEVICE_TYPE_CPU) ? "CPU" : "GPU") << " queue:" << endl;
	cout << "  Platform " << platformName << ", " << platformVersion << endl;
	cout << "  Device " << deviceName << endl;
}

void createQueues(CLQueue& cpu, CLQueue& gpu)
{
	// Get platforms
	cl_int err;
	cl_uint numPlatforms;
	cl_platform_id platform[16];
	if (clGetPlatformIDs(16, &platform[0], &numPlatforms) != CL_SUCCESS || numPlatforms == 0)
		fatalError("Can't obtain platforms");

	// find GPU device
	for (int i = 0; i < numPlatforms; i++)
	{
		gpu.device = findRenderDevice(platform[i]);
		if (gpu.device) {
			gpu.platform = platform[i];
			break;
		}
		
	}

	// find CPU device: try to share context first, then the rest
	cpu.device = findCPUDevice(gpu.platform);
	if (cpu.device) 
	{
		cpu.platform = gpu.platform;
		cl_device_id devices[2] = { gpu.device, cpu.device };
		gpu.context = clCreateContext(&renderContextProps(gpu.platform)[0], 2, devices, nullptr, nullptr, &err);
		cpu.context = gpu.context;
		clTest(err, "create context");
	}
	else
	{
		for (int i = 0; i < numPlatforms && !cpu.device; i++)
		{
			cpu.device = findCPUDevice(platform[i]);
			if (cpu.device)
				cpu.platform = platform[i];
		}
		gpu.context = clCreateContext(&renderContextProps(gpu.platform)[0], 1, &gpu.device, nullptr, nullptr, &err);
		clTest(err, "create context");
		cl_context_properties props[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)cpu.platform, 0 };
		cpu.context = clCreateContext(props, 1, &cpu.device, nullptr, nullptr, &err);
		clTest(err, "create context");
	}
	
	// create queues
    cpu.handle = clCreateCommandQueue(cpu.context, cpu.device, 0, &err);
    clTest(err, "create cpu queue");
	gpu.handle = clCreateCommandQueue(gpu.context, gpu.device, 0, &err);
    clTest(err, "create gpu queue");

	cpu.printInfo();
	gpu.printInfo();
}

map<string, CLProgram> CLProgram::instances;

CLProgram& CLProgram::get(CLQueue& queue, const std::string& filename) 
{
	auto it = instances.find(filename);
	if (it != instances.end()) 
		return it->second;

	instances[filename] = CLProgram();
	CLProgram& prog = instances[filename];

	string pathName = "src/opencl/" + filename;
	fstream file(pathName.c_str());
	if (!file.is_open())
		fatalError("Can't load OpenCL kernel " + filename);
	string buffer (istreambuf_iterator<char>(file), (istreambuf_iterator<char>()));

	// compile program
	cl_int err;
	size_t size = buffer.size();
	const char* buf = buffer.c_str();
	prog.handle = clCreateProgramWithSource(queue.context, 1, &buf, &size, &err);
	clTest(err, "Failed to create source of " + filename);
	if(clBuildProgram(prog.handle, 0, nullptr, nullptr, nullptr, nullptr) != CL_SUCCESS)
	{
		// output build error
		size_t len;
 		char buffer[2048];
 		clGetProgramBuildInfo(prog.handle, queue.device, CL_PROGRAM_BUILD_LOG, 
			sizeof(buffer), buffer, &len);
 		cerr << "Build log: " << buffer << endl;
 		fatalError("Build failed");
	}
	return prog;
}

CLKernel::CLKernel(CLQueue& queue, const string& filename, const string& kernel) :
	queue(queue), program(CLProgram::get(queue, filename))
{
	cl_int err;
	handle = clCreateKernel(program.handle, kernel.c_str(), &err);
	clTest(err, "Can't create kernel " + kernel);
}

void CLKernel::enqueue(size_t problem_size) 
{
	size_t local = 1;
	cl_int err = clEnqueueNDRangeKernel(queue.handle, handle, 1, nullptr, &problem_size, 
										&local, 0, nullptr, nullptr);
	clTest(err, "Can't enqueue kernel");
}
