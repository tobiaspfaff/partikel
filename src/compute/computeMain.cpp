// Init OpenCL

#include <iostream>
#include <fstream>
#include <iterator>
#include "compute/computeMain.hpp"
#include "tools/log.hpp"
#include "OpenGL/OpenGL.h"

using namespace std;

void clTest(cl_int err, const string& msg) 
{
	if (err != CL_SUCCESS)
		fatalError("OpenCL error: " + msg);
}

CLMaster::CLMaster() 
{
	initialize();
}

void CLMaster::initialize() 
{
	// Get platform	
	cl_int err;
	cl_uint num;
	cl_platform_id platform;
	if (clGetPlatformIDs(1, &platform, &num) != CL_SUCCESS || num !=1)
		fatalError("Can't obtain platform");

	size_t size;
	char platformName[256], platformVersion[256];
	clGetPlatformInfo(platform, CL_PLATFORM_NAME, 256, platformName, &size);
	clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 256, platformVersion, &size);
	cout << platformName << " " << platformVersion << endl;

	// Figure out which GPU is used for rendering
#if defined (__APPLE__)
	CGLContextObj glContext = CGLGetCurrentContext();
	CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);

	cl_context_properties props[] = { 
		CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE, (intptr_t)shareGroup, 
		CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
        0 
	};
	context = clCreateContext(props, 0, NULL, 0, 0, &err);
	clTest(err, "create gl context");

	cl_device_id renderer;
	clTest(clGetGLContextInfoAPPLE(context, glContext, CL_CGL_DEVICE_FOR_CURRENT_VIRTUAL_SCREEN_APPLE, 
								   sizeof(renderer), &renderer, NULL), "get gl context");
	clReleaseContext(context);
#elif defined (WIN32)
    cl_context_properties props[] =
    {
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
        0
    };
#else
    cl_context_properties props[] =
    {
        CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(),
        CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
        0
    };
#endif
 
	// Enumerate devices
	cl_device_id allDevices[16];
	cl_device_id defaultCPU;
	clTest(clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 16, allDevices, &num), "list devices");
	for (int i=0; i<num; i++)
	{
		cl_device_id& cur = allDevices[i];
		cl_device_type type;
		char deviceName[256], deviceVendor[256];		
		clGetDeviceInfo(cur, CL_DEVICE_TYPE, sizeof(type), &type, &size);
		if (type == CL_DEVICE_TYPE_CPU)
			defaultCPU = cur;
		else if (type == CL_DEVICE_TYPE_GPU && cur != renderer) 
			continue;
		
		clGetDeviceInfo(cur, CL_DEVICE_VENDOR, 256, deviceVendor, &size);
		clGetDeviceInfo(cur, CL_DEVICE_NAME, 256, deviceName, &size);
		cout << "Using " << (type == CL_DEVICE_TYPE_GPU ? "GPU" : "CPU");
		cout << " device : " << deviceVendor << " " << deviceName << endl;
		devices.push_back(cur);		
	}

	context = clCreateContext(props, devices.size(), &devices[0], nullptr, nullptr, &err);
    clTest(err, "create context");

    cpuQueue = clCreateCommandQueue(context, defaultCPU, 0, &err);
    clTest(err, "create cpu queue");
    gpuQueue = clCreateCommandQueue(context, renderer, 0, &err);
    clTest(err, "create gpu queue");
}

map<string, CLProgram> CLProgram::instances;

CLProgram& CLProgram::get(CLMaster& master, const std::string& filename) 
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
	prog.handle = clCreateProgramWithSource(master.context, 1, &buf, &size, &err);
	clTest(err, "Failed to create source of " + filename);
	if(clBuildProgram(prog.handle, 0, nullptr, nullptr, nullptr, nullptr) != CL_SUCCESS)
	{
		// output build error
		for(auto const& dev: master.devices) 
		{
			size_t len;
 			char buffer[2048];
 			clGetProgramBuildInfo(prog.handle, dev, CL_PROGRAM_BUILD_LOG, 
 								  sizeof(buffer), buffer, &len);
 			cerr << "Build log: " << buffer << endl;
 		}
		fatalError("Build failed");
	}
	return prog;
}

CLKernel::CLKernel(CLMaster& master, const string& filename, const string& kernel) :
	program(CLProgram::get(master, filename))
{
	cl_int err;
	handle = clCreateKernel(program.handle, kernel.c_str(), &err);
	clTest(err, "Can't create kernel " + kernel);
}

void CLKernel::enqueue(cl_command_queue& queue, size_t problem_size) 
{
	size_t local = 1;
	cl_int err = clEnqueueNDRangeKernel(queue, handle, 1, nullptr, &problem_size, 
										&local, 0, nullptr, nullptr);
	clTest(err, "Can't enqueue kernel");
}

