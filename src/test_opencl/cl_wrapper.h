#pragma once

#define CL_HPP_TARGET_OPENCL_VERSION 200
#define ENABLE_CL_PROFILING

#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

enum DeviceType {
	ADRENO,
	MALI,
	UNKNOW
};

struct DeviceInfo {
	//bool supportFp16;
	uint32_t maxWorkItemSizes;
	size_t maxImage2DWidth;
	size_t maxImage2DHeight;

	std::string platformVendor;
	std::string platformName;
	std::string platformVersion;

	std::string deviceVendor;
	std::string deviceName;
	std::string deviceVersion;

	DeviceType deviceType;
	int32_t computeUnitsNum;
	std::vector<int32_t> maxWorkItemSize;

	void show() {
		std::cout << "============ DeviceInfo ============" << std::endl;
		std::cout << "Platform:" << std::endl;
		std::cout << "  name   : " << platformName << std::endl;
		std::cout << "  vendor : " << platformVendor << std::endl;
		std::cout << "  version: " << platformVersion << std::endl;

		std::cout << "Device:" << std::endl;
		std::cout << "  name   : " << deviceName << std::endl;
		std::cout << "  vendor : " << deviceVendor << std::endl;
		std::cout << "  version: " << deviceVersion << std::endl;
		std::cout << "  CU num : " << computeUnitsNum << std::endl;
		std::cout << "  maxSize: " << "[ ";
		for (auto i = maxWorkItemSize.cbegin(); i != maxWorkItemSize.cend(); ++i) {
			std::cout << *i << " ";
		}
		std::cout << "]" << std::endl;
		std::cout << "====================================" << std::endl;
	}
};

class CL_Wrapper {
public:
	CL_Wrapper(cl_context_properties *context_properties = nullptr, cl_command_queue_properties queue_properties = 0)
	{
		cl_int err;

		err = cl::Platform::get(&platforms);
		if (err != CL_SUCCESS)
		{
			std::cerr << "Error " << err << " with clGetPlatformIDs." << "\n";
			return;
		}

		err = platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
		if (err != CL_SUCCESS)
		{
			std::cerr << "Error " << err << " with clGetDeviceIDs." << "\n";
			return;
		}

		context = cl::Context(devices, context_properties, NULL, NULL, &err);
		if (err != CL_SUCCESS)
		{
			std::cerr << "Error " << err << " with clCreateContext." << "\n";
			return;
		}

#ifdef ENABLE_CL_PROFILING
		queue_properties |= CL_QUEUE_PROFILING_ENABLE;
#endif

		cl_command_queue_properties properties{ queue_properties };
		commandQueue = cl::CommandQueue(context, devices[0], &properties, &err);
		if (err != CL_SUCCESS)
		{
			std::cerr << "Error " << err << " with clCreateCommandQueue." << "\n";
		}
	}

	DeviceInfo getDeviceInfos()
	{
		queryDeviceInfos();
		return deviceInfos;
	}

    cl::Device getDeviceId() const
	{
		return devices[0];
	}

    cl::Context getContext() const
	{
		return context;
	}

    cl::CommandQueue getCommandQueue() const
	{
		return commandQueue;
	}

	cl::Program makeProgram(cl::Context& context, std::string& filePath, cl_int* err, const char* options = NULL/*, std::string pathAndName = "", std::string checkString = ""*/)
	{
		cl_int error = 0;

		/*
		if (!pathAndName.empty() &&
			!checkString.empty())
		{
			tryReadProgramFromPath(pathAndName, checkString);
		}
		*/

		std::fstream file(filePath, std::ios::binary | std::ios::in | std::ios::ate);
		if (!file.is_open()) {
			std::cerr << "Error " << error << " Failed to open " << filePath << "\n";
		}

		size_t fileSize = file.tellg();
		file.seekg(0);

		std::vector<char> buffer(fileSize);
		char* data = buffer.data();

		file.read(data, fileSize);
		file.close();

		std::string kernelCode(data);
		cl::Program program(context, kernelCode, &error);
		if (error != CL_SUCCESS)
		{
			std::cerr << "Error " << error << " with make_program." << "\n";
			if (err != NULL) {
				*err = error;
			}
			return program;
		}
		error = program.build(options);
		if (error != CL_SUCCESS)
		{
			std::string log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
			std::cerr << "Error " << error << " with clBuildProgram:\n" << log << std::endl;
		}
		*err = error;

		return program;
	}

	int32_t makeKernel(cl::Program program, std::vector<cl::Kernel>& kernels)
	{
		int32_t err = program.createKernels(&kernels);
		if (err != CL_SUCCESS)
		{
			std::cerr << "Error " << err << " with make_kernel." << "\n";
		}
		return err;
	}

private:
	int32_t queryDeviceInfos()
	{
		int32_t err = CL_SUCCESS, error;
		deviceInfos.platformVendor = platforms[0].getInfo<CL_PLATFORM_VENDOR>(&error);
		err |= error;
		deviceInfos.platformName = platforms[0].getInfo<CL_PLATFORM_NAME>(&error);
		err |= error;
		deviceInfos.platformVersion = platforms[0].getInfo<CL_PLATFORM_VERSION>(&error);
		err |= error;

		deviceInfos.deviceVendor = devices[0].getInfo<CL_DEVICE_VENDOR>(&error);
		err |= error;
		deviceInfos.deviceName = devices[0].getInfo<CL_DEVICE_NAME>(&error);
		err |= error;
		deviceInfos.deviceVersion = devices[0].getInfo<CL_DEVICE_VERSION>(&error);
		err |= error;

		deviceInfos.computeUnitsNum = devices[0].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(&error);
		err |= error;
		std::vector<size_t> maxWorkItemSizes = devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(&error);
		err |= error;

		for (auto i = maxWorkItemSizes.cbegin(); i != maxWorkItemSizes.cend(); ++i)
		{
			deviceInfos.maxWorkItemSize.push_back(static_cast<int32_t>(*i));
		}

		if (deviceInfos.deviceName.find("Adreno") != std::string::npos) 
		{
			deviceInfos.deviceType = DeviceType::ADRENO;
		}
		else if (deviceInfos.deviceName.find("Mali") != std::string::npos) 
		{
			deviceInfos.deviceType = DeviceType::MALI;
		}
		else 
		{
			deviceInfos.deviceType = DeviceType::UNKNOW;
		}

		deviceInfos.maxImage2DWidth = devices[0].getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>(&error);
		err |= error;
		deviceInfos.maxImage2DHeight = devices[0].getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>(&error);
		err |= error;

		if (err != CL_SUCCESS)
		{
			std::cerr << "Error " << err << " with queryDeviceInfos." << "\n";
		}

		return err;
	}

	/*
	bool tryReadProgramFromPath(std::string pathAndName, std::string checkString)
	{
		std::string checkInfo = deviceInfos.platformName +
			deviceInfos.platformVersion +
			deviceInfos.deviceName +
			deviceInfos.deviceVersion;

		if (checkInfo != checkString) {
			std::cerr << "Error" << " Binary program does not match, Need to recompile." << "\n";
			return false;
		}

		std::fstream file(pathAndName, std::ios::binary | std::ios::in | std::ios::ate);
		if (!file.is_open()) {
			std::cerr << "Failed to open " << " Trying to compile from sources." << "\n";
			return false;
		}

		size_t fileSize = file.tellg();
		file.seekg(0);

		char* buffer = (char*)malloc(fileSize);
		file.read(buffer, fileSize);
		file.close();

		//TO DO
	}
	*/

	std::vector<cl::Platform> platforms;
	std::vector<cl::Device> devices;
	cl::Context context;
	cl::CommandQueue commandQueue;

	DeviceInfo deviceInfos;
};

