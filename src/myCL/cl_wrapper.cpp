#pragma once

#include "cl_wrapper.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

CL_wrapper::CL_wrapper(cl_context_properties* context_properties, cl_command_queue_properties queue_properties)
{
	cl_int err;

	err = cl::Platform::get(&platforms);
	CHECK_ERR_CL(err);

	err = platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
	CHECK_ERR_CL(err);

	_context = cl::Context(devices, context_properties, NULL, NULL, &err);
	CHECK_ERR_CL(err);

#ifdef ENABLE_CL_PROFILING
	queue_properties |= CL_QUEUE_PROFILING_ENABLE;
#endif

	cl_command_queue_properties properties{ queue_properties };
	_commandQueue = cl::CommandQueue(_context, devices[0], &properties, &err);
	CHECK_ERR_CL(err);
}

CL_wrapper& CL_wrapper::build(std::string& filePath, std::string options)
{
	int err = CL_SUCCESS;

	//if (!pathAndName.empty() &&	!checkString.empty()) {
	//	tryReadProgramFromPath(pathAndName, checkString);
	//}

	std::fstream file(filePath, std::ios::binary | std::ios::in | std::ios::ate);
	if (!file.is_open()) {
		printf("[OpenCL Error]: Fail to open %s [%s:%s:%d]\n", filePath.c_str(), __FILE__, __FUNCTION__, __LINE__);
		return *this;
	}

	size_t fileSize = file.tellg();
	file.seekg(0);

	std::string kernelStr;
	kernelStr.resize(fileSize);

	file.read(&kernelStr[0], fileSize);
	file.close();

	program = cl::Program(_context, kernelStr, &err);
	if (err != CL_SUCCESS)
	{
		printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__);
		return *this;
	}


	err = program.build(options.c_str());
	if (err != CL_SUCCESS)
	{
		printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__);
		std::string log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0], &err);
		if (err != CL_SUCCESS) {
			printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__);
		} else {
			printf("CL Program Build Log: %s\n", log.c_str());
		}
	}

	return *this;
}

CL_wrapper& CL_wrapper::makeKernel() 
{
	std::vector<cl::Kernel> kernelVec;
	int32_t err = program.createKernels(&kernelVec);
	if (err != CL_SUCCESS)
	{
		printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__);
		return *this;
	}

	for (auto k = kernelVec.begin(); k != kernelVec.end(); ++k) {
		std::string name = k->getInfo<CL_KERNEL_FUNCTION_NAME>();
		kernels[name] = *k;
	}

	return *this;
}

cl::Kernel& CL_wrapper::kernel(std::string kernelName)
{
	cl::Kernel tmp;
	if (kernels.find(kernelName) == kernels.end()) {
		printf("[OpenCL Error]: Fail to find kernel[%s] [%s:%s:%d]\n", kernelName.c_str(), __FILE__, __FUNCTION__, __LINE__);
		return tmp;
	} else {
		return kernels[kernelName];
	}
}

void CL_wrapper::timer(std::string taskName)
{
	startTime = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	endTime = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();

	printf("Time of %s is %.2f ms\n", taskName.c_str(), (endTime - startTime) / 1000000.0f);
}

//int32_t queryDeviceInfos()
//{
//	int32_t err = CL_SUCCESS, error;
//	deviceInfos.platformVendor = platforms[0].getInfo<CL_PLATFORM_VENDOR>(&error);
//	err |= error;
//	deviceInfos.platformName = platforms[0].getInfo<CL_PLATFORM_NAME>(&error);
//	err |= error;
//	deviceInfos.platformVersion = platforms[0].getInfo<CL_PLATFORM_VERSION>(&error);
//	err |= error;
//
//	deviceInfos.deviceVendor = devices[0].getInfo<CL_DEVICE_VENDOR>(&error);
//	err |= error;
//	deviceInfos.deviceName = devices[0].getInfo<CL_DEVICE_NAME>(&error);
//	err |= error;
//	deviceInfos.deviceVersion = devices[0].getInfo<CL_DEVICE_VERSION>(&error);
//	err |= error;
//
//	deviceInfos.computeUnitsNum = devices[0].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(&error);
//	err |= error;
//	std::vector<size_t> maxWorkItemSizes = devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(&error);
//	err |= error;
//
//	deviceInfos.maxImage2DWidth = devices[0].getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>(&error);
//	err |= error;
//	deviceInfos.maxImage2DHeight = devices[0].getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>(&error);
//	err |= error;
//
//	if (err != CL_SUCCESS)
//	{
//		std::cerr << "Error " << err << " with queryDeviceInfos." << "\n";
//	}
//
//	return err;
//}

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

std::string clErrorCheck(cl_int err)
{
	std::string errorInfo;
	if (err != CL_SUCCESS) {
		errorInfo = "[OpenCL Error] " + std::to_string(err) + " ";
		switch (err) {
		case -1: errorInfo += "CL_DEVICE_NOT_FOUND"; break;
		case -2: errorInfo += "CL_DEVICE_NOT_AVAILABLE"; break;
		case -3: errorInfo += "CL_COMPILER_NOT_AVAILABLE"; break;
		case -4: errorInfo += "CL_MEM_OBJECT_ALLOCATION_FAILURE"; break;
		case -5: errorInfo += "CL_OUT_OF_RESOURCES"; break;
		case -6: errorInfo += "CL_OUT_OF_HOST_MEMORY"; break;
		case -7: errorInfo += "CL_PROFILING_INFO_NOT_AVAILABLE"; break;
		case -8: errorInfo += "CL_MEM_COPY_OVERLAP"; break;
		case -9: errorInfo += "CL_IMAGE_FORMAT_MISMATCH"; break;
		case -10: errorInfo += "CL_IMAGE_FORMAT_NOT_SUPPORTED"; break;
		case -11: errorInfo += "CL_BUILD_PROGRAM_FAILURE"; break;
		case -12: errorInfo += "CL_MAP_FAILURE"; break;
		case -13: errorInfo += "CL_MISALIGNED_SUB_BUFFER_OFFSET"; break;
		case -14: errorInfo += "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST"; break;
		case -30: errorInfo += "CL_INVALID_VALUE"; break;
		case -31: errorInfo += "CL_INVALID_DEVICE_TYPE"; break;
		case -32: errorInfo += "CL_INVALID_PLATFORM"; break;
		case -33: errorInfo += "CL_INVALID_DEVICE"; break;
		case -34: errorInfo += "CL_INVALID_CONTEXT"; break;
		case -35: errorInfo += "CL_INVALID_QUEUE_PROPERTIES"; break;
		case -36: errorInfo += "CL_INVALID_COMMAND_QUEUE"; break;
		case -37: errorInfo += "CL_INVALID_HOST_PTR"; break;
		case -38: errorInfo += "CL_INVALID_MEM_OBJECT"; break;
		case -39: errorInfo += "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"; break;
		case -40: errorInfo += "CL_INVALID_IMAGE_SIZE"; break;
		case -41: errorInfo += "CL_INVALID_SAMPLER"; break;
		case -42: errorInfo += "CL_INVALID_BINARY"; break;
		case -43: errorInfo += "CL_INVALID_BUILD_OPTIONS"; break;
		case -44: errorInfo += "CL_INVALID_PROGRAM"; break;
		case -45: errorInfo += "CL_INVALID_PROGRAM_EXECUTABLE"; break;
		case -46: errorInfo += "CL_INVALID_KERNEL_NAME"; break;
		case -47: errorInfo += "CL_INVALID_KERNEL_DEFINITION"; break;
		case -48: errorInfo += "CL_INVALID_KERNEL"; break;
		case -49: errorInfo += "CL_INVALID_ARG_INDEX"; break;
		case -50: errorInfo += "CL_INVALID_ARG_VALUE"; break;
		case -51: errorInfo += "CL_INVALID_ARG_SIZE"; break;
		case -52: errorInfo += "CL_INVALID_KERNEL_ARGS"; break;
		case -53: errorInfo += "CL_INVALID_WORK_DIMENSION"; break;
		case -54: errorInfo += "CL_INVALID_WORK_GROUP_SIZE"; break;
		case -55: errorInfo += "CL_INVALID_WORK_ITEM_SIZE"; break;
		case -56: errorInfo += "CL_INVALID_GLOBAL_OFFSET"; break;
		case -57: errorInfo += "CL_INVALID_EVENT_WAIT_LIST"; break;
		case -58: errorInfo += "CL_INVALID_EVENT"; break;
		case -59: errorInfo += "CL_INVALID_OPERATION"; break;
		case -60: errorInfo += "CL_INVALID_GL_OBJECT"; break;
		case -61: errorInfo += "CL_INVALID_BUFFER_SIZE"; break;
		case -62: errorInfo += "CL_INVALID_MIP_LEVEL"; break;
		case -63: errorInfo += "CL_INVALID_GLOBAL_WORK_SIZE"; break;
		case -64: errorInfo += "CL_INVALID_PROPERTY"; break;
		default: errorInfo += "unknown error."; break;
		}
	}
	return errorInfo;
}