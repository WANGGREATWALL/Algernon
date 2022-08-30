#pragma once

#define CL_HPP_TARGET_OPENCL_VERSION 200
#define ENABLE_CL_PROFILING

#include <CL/cl.hpp>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>

using map_kernel = std::map<std::string, cl::Kernel>;
using vec_string = std::vector<std::string>;
using set_string = std::set<std::string>;
using vec_size_t = std::vector<size_t>;
using vec_array3 = std::vector<cl::size_t<3> >;


std::string clErrorCheck(cl_int err);

#define CHECK_ERR_CL(err) if (err != CL_SUCCESS) {printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__); return;}
#define CHECK_RET_CL(err) if (err != CL_SUCCESS) {printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__); return err;}

struct PlatDevice_info {
	vec_string		platformName;
	vec_string		platformVendor;
	vec_string		platformVersion;

	vec_string		deviceName;
	vec_string		deviceVendor;
	vec_string		deviceVersion;
};

struct Image_capacity {
	size_t			maxImage2DWidth;
	size_t			maxImage2DHeight;
	size_t			maxReadImageArgs;
	size_t			maxWriteImageArgs;
	set_string		enableChannelOrders;

	std::string order2string(int num);
};

struct Performance_info {
	size_t			maxWorkItemDim;
	size_t			maxWorkgroupSize;
	size_t			maxComputeUnit;
	vec_size_t		maxWorkitemSizes;
	size_t			localMemSize;
};

struct Kernel_Properties {
	vec_size_t		workgroupSizes;
	vec_array3		compileWorkgroupSizes;
	vec_size_t		privateMemSizes;
	vec_size_t		localMemSizes;
	vec_size_t		preferredWorkgroupSizeMultiple;
};

class CL_wrapper {
public:
	CL_wrapper(cl_context_properties* context_properties = nullptr, cl_command_queue_properties queue_properties = 0);

	CL_wrapper& build(std::string& filePath, std::string options = "");
	CL_wrapper& makeKernel();

	CL_wrapper& checkPlatformDevice(bool show);
	CL_wrapper& checkImageCapacity(bool show);
	CL_wrapper& checkPerformanceInfo(bool show);
	CL_wrapper& checkKernelProperties(bool show);

	cl::Device& device()				{ return devices[0]; }
	cl::Context& context()				{ return _context; }
	cl::CommandQueue& commandQueue()	{ return _commandQueue; }
	cl::Kernel& kernel(std::string kernelName);

	void timer(std::string taskName);

	cl::Event					event;

private:
	//Todo:
	bool tryReadProgramFromPath(std::string pathAndName, std::string checkString);

	//Basic:
	std::vector<cl::Platform>	platforms;
	std::vector<cl::Device>		devices;
	cl::Context					_context;
	cl::CommandQueue			_commandQueue;
	cl::Program					program;
	map_kernel					kernels;
	cl_long						startTime;
	cl_long						endTime;

	//Info:
	PlatDevice_info				PD_info;
	Image_capacity				IM_info;
	Performance_info			PF_info;
	Kernel_Properties			KN_info;
};

