#pragma once

#define CL_HPP_TARGET_OPENCL_VERSION 200
#define ENABLE_CL_PROFILING

#include <CL/cl.hpp>
#include <string>
#include <vector>
#include <map>

using Map_kernel = std::map<std::string, cl::Kernel>;

std::string clErrorCheck(cl_int err);

#define CHECK_ERR_CL(err) if (err != CL_SUCCESS) {printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__); return;}
#define CHECK_RET_CL(err) if (err != CL_SUCCESS) {printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__); return err;}

struct PlatDevice_info {
	std::string platformName;
	std::string platformVendor;
	std::string platformVersion;

	std::string deviceName;
	std::string deviceVendor;
	std::string deviceVersion;
};

struct Image_capacity {
	size_t maxImage2DWidth;
	size_t maxImage2DHeight;
};

struct Performance_info {
	size_t maxWorkItemSize;
	size_t computeUnitsNum;
	std::vector<size_t> maxWorkItemSizes;
};

struct Kernel_Properties {

};

class CL_wrapper {
public:
	CL_wrapper(cl_context_properties* context_properties = nullptr, cl_command_queue_properties queue_properties = 0);

	CL_wrapper& build(std::string& filePath, std::string options = "");
	CL_wrapper& makeKernel();

	cl::Device& device() { return devices[0]; }
	cl::Context& context() { return _context; }
	cl::CommandQueue& commandQueue() { return _commandQueue; }
	cl::Kernel& kernel(std::string kernelName);

	void timer(std::string taskName);

	cl::Event					event;

private:
	int32_t queryDeviceInfos();

	//Todo:
	bool tryReadProgramFromPath(std::string pathAndName, std::string checkString);

	//Basic:
	std::vector<cl::Platform>	platforms;
	std::vector<cl::Device>		devices;
	cl::Context					_context;
	cl::CommandQueue			_commandQueue;
	cl::Program					program;
	Map_kernel					kernels;
	cl_long						startTime;
	cl_long						endTime;

	//Info:
	PlatDevice_info				PD_info;
	Image_capacity				IM_info;
	Performance_info			PF_info;
	Kernel_Properties			KN_info;
};

