#pragma once

#include "cl_wrapper.h"
#include "myUtils/myUtils.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <iomanip>
#include <functional>

#define WIDTH_PD	20
#define WIDTH_IM	20
#define WIDTH_PF	20
#define WIDTH_KN	20

std::string getAlgoVersion()
{
	//algoName_v1.0.3
	return "algoName_v1.0.3";
}

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
	bool isBinOK = false;

	//double sTime = timer();

	std::string binPath = filePath;
	std::string keyPath = filePath;

	binPath.replace(filePath.rfind(".cl"), 3, ".bin");
	keyPath.replace(filePath.rfind(".cl"), 3, ".key");

	std::string deviceName = devices[0].getInfo<CL_DEVICE_NAME>();
	std::string deviceVendor = devices[0].getInfo<CL_DEVICE_VENDOR>();
	std::string deviceVersion = devices[0].getInfo<CL_DEVICE_VERSION>();

	std::string deviceInfo = deviceName + " " + deviceVendor + " " + deviceVersion + " " + getAlgoVersion();
	std::string key_device = std::to_string(std::hash<std::string>{}(deviceInfo));

	// Try to read bin file:
	std::fstream file_bin(binPath, std::ios::binary | std::ios::in | std::ios::ate);
	if (file_bin.is_open())
	{
		size_t fileSize = file_bin.tellg();
		file_bin.seekg(0);

		std::vector<char> programBin(fileSize);
		cl::Program::Binaries kernelBin = { {programBin.data(), fileSize} };

		file_bin.read(programBin.data(), fileSize);
		file_bin.close();

		program = cl::Program(_context, devices, kernelBin, NULL, &err);
		if (err != CL_SUCCESS)
		{
			printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__);
			isBinOK = false;
			goto REBUILD;
		}

		std::string programStr(programBin.cbegin(), programBin.cend());
		std::string key_program = std::to_string(std::hash<std::string>{}(programStr));
		std::string key = key_device + key_program;

		// Check key file:
		std::fstream file_key(keyPath, std::ios::binary | std::ios::in | std::ios::ate);
		if (!file_key.is_open()) {
			printf("[OpenCL Error]: Fail to open %s [%s:%s:%d]\n", keyPath.c_str(), __FILE__, __FUNCTION__, __LINE__);
			isBinOK = false;
			goto REBUILD;
		}

		fileSize = file_key.tellg();
		file_key.seekg(0);

		if (fileSize > 40)
		{
			printf("[OpenCL Error]: Invalid key value from %s [%s:%s:%d]\n", keyPath.c_str(), __FILE__, __FUNCTION__, __LINE__);
			isBinOK = false;
			goto REBUILD;
		}

		std::string keyCheck;
		keyCheck.resize(fileSize);

		file_key.read(&keyCheck[0], fileSize);
		file_key.close();

		if (key != keyCheck)
		{
			isBinOK = false;
			goto REBUILD;
		}
		isBinOK = true;
	}

	REBUILD:
	if (!isBinOK)
	{
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

		// Write bin & key:
		auto programBin = program.getInfo<CL_PROGRAM_BINARIES>(&err);
		if (err != CL_SUCCESS)
		{
			printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__);
			return *this;
		}
		auto programSize = program.getInfo<CL_PROGRAM_BINARY_SIZES>(&err);
		if (err != CL_SUCCESS)
		{
			printf("%s [%s:%s:%d]\n", clErrorCheck(err).c_str(), __FILE__, __FUNCTION__, __LINE__);
			return *this;
		}

		std::string programStr(programBin[0], programSize[0]);
		std::string key_program = std::to_string(std::hash<std::string>{}(programStr));
		std::string key = key_device + key_program;

		err = write_data_to_file(binPath, programBin[0], programSize[0]);
		err |= write_data_to_file(keyPath, key.data(), key.size());
		if (err != CL_SUCCESS)
		{
			printf("Failed to write file [%s:%s:%d]\n", __FILE__, __FUNCTION__, __LINE__);
			return *this;
		}
	}

	//printf("	TIME of program creation is %.2f ms\n", timer() - sTime);

	//sTime = timer();

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

	//printf("	TIME of program build is %.2f ms\n", timer() - sTime);

	return *this;
}

CL_wrapper& CL_wrapper::makeKernel() 
{
	double sTime = timer();
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

CL_wrapper& CL_wrapper::checkPlatformDevice(bool show)
{
	int index = 0;
	for (auto i = platforms.cbegin(); i != platforms.cend(); ++i, ++index) {
		std::string name	= i->getInfo<CL_PLATFORM_NAME>();
		std::string vendor	= i->getInfo<CL_PLATFORM_VENDOR>();
		std::string version = i->getInfo<CL_PLATFORM_VERSION>();

		PD_info.platformName.push_back(name);
		PD_info.platformVendor.push_back(vendor);
		PD_info.platformVersion.push_back(version);

		if (show) {
			std::cout << std::setw(WIDTH_PD) << std::left << "Platform_Name_" + std::to_string(index) + ": " << name << std::endl;
			std::cout << std::setw(WIDTH_PD) << std::left << "Platform_Vendor_" + std::to_string(index) + ": " << vendor << std::endl;
			std::cout << std::setw(WIDTH_PD) << std::left << "Platform_Version_" + std::to_string(index) + ": " << version << std::endl;
		}
	}
	
	index = 0;
	for (auto i = devices.cbegin(); i != devices.cend(); ++i, ++index) {
		std::string name	= i->getInfo<CL_DEVICE_NAME>();
		std::string vendor	= i->getInfo<CL_DEVICE_VENDOR>();
		std::string version = i->getInfo<CL_DEVICE_VERSION>();

		PD_info.deviceName.push_back(name);
		PD_info.deviceVendor.push_back(vendor);
		PD_info.deviceVersion.push_back(version);

		if (show) {
			std::cout << std::setw(WIDTH_PD) << std::left << "Device_Name_" + std::to_string(index) + ": " << name << std::endl;
			std::cout << std::setw(WIDTH_PD) << std::left << "Device_Vendor_" + std::to_string(index) + ": " << vendor << std::endl;
			std::cout << std::setw(WIDTH_PD) << std::left << "Device_Version_" + std::to_string(index) + ": " << version << std::endl;
		}
	}

	return *this;
}

CL_wrapper& CL_wrapper::checkImageCapacity(bool show)
{
	IM_info.maxImage2DWidth = devices[0].getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>();
	IM_info.maxImage2DHeight = devices[0].getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>();
	IM_info.maxReadImageArgs = devices[0].getInfo<CL_DEVICE_MAX_READ_IMAGE_ARGS>();
	IM_info.maxWriteImageArgs = devices[0].getInfo<CL_DEVICE_MAX_WRITE_IMAGE_ARGS>();

	std::set<std::string> orders;
	std::vector<cl::ImageFormat> formats;
	_context.getSupportedImageFormats(CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D, &formats);
	for (auto i = formats.cbegin(); i != formats.cend(); ++i) {
		std::string order = IM_info.order2string(i->image_channel_order);
		orders.insert(order);
	}

	if (show) {
		std::cout << std::setw(22) << std::left << "Max_Image2D_Width: " << IM_info.maxImage2DWidth << std::endl;
		std::cout << std::setw(22) << std::left << "Max_Image2D_Height: " << IM_info.maxImage2DHeight << std::endl;
		std::cout << std::setw(22) << std::left << "Max_Read_Image_Args: " << IM_info.maxReadImageArgs << std::endl;
		std::cout << std::setw(22) << std::left << "Max_Write_Image_Args: " << IM_info.maxWriteImageArgs << std::endl;

		int index = 0;
		std::cout << std::setw(WIDTH_IM) << std::left << "Available_Channel_Orders: ";
		for (auto i = orders.cbegin(); i != orders.cend(); ++i, ++index) {
			if (index != 0) {
				std::cout << std::setw(26) << "";
			}
			std::cout << *i << std::endl;
		}
	}

	return *this;
}

std::string Image_capacity::order2string(int num)
{
	switch (num) {
	case CL_R:	return "CL_R";
	case CL_A:	return "CL_A";
	case CL_RG: return "CL_RG";
	case CL_RA:	return "CL_RA";
	case CL_RGB: return "CL_RGB";
	case CL_RGBA:	return "CL_RGBA";
	case CL_BGRA: return "CL_BGRA";
	case CL_ARGB:	return "CL_ARGB";
	case CL_INTENSITY: return "CL_INTENSITY";
	case CL_LUMINANCE:	return "CL_LUMINANCE";
	case CL_Rx: return "CL_Rx";
	case CL_RGx:	return "CL_RGx";
	case CL_RGBx: return "CL_RGBx";
	case CL_DEPTH:	return "CL_DEPTH";
	case CL_DEPTH_STENCIL: return "CL_DEPTH_STENCIL";
	case CL_sRGB: return "CL_sRGB";
	case CL_sRGBx:	return "CL_sRGBx";
	case CL_sRGBA: return "CL_sRGBA";
	case CL_sBGRA:	return "CL_sBGRA";
	case CL_ABGR: return "CL_ABGR";
	default: return "";
	}
}

CL_wrapper& CL_wrapper::checkPerformanceInfo(bool show)
{
	PF_info.maxWorkItemDim = devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>();
	PF_info.maxWorkgroupSize = devices[0].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
	PF_info.maxComputeUnit = devices[0].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
	PF_info.maxWorkitemSizes = devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
	PF_info.localMemSize = devices[0].getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();

	if (show) {
		std::cout << std::setw(WIDTH_PF) << std::left << "Max_Workitem_Dim: " << PF_info.maxWorkItemDim << std::endl;
		std::cout << std::setw(WIDTH_PF) << std::left << "Max_Workgroup_Size: " << PF_info.maxWorkgroupSize << std::endl;
		std::cout << std::setw(WIDTH_PF) << std::left << "Max_Compute_Unit: " << PF_info.maxComputeUnit << std::endl;
		std::cout << std::setw(WIDTH_PF) << std::left << "Max_Workitem_Sizes: "
			<< PF_info.maxWorkitemSizes[0] << " "
			<< PF_info.maxWorkitemSizes[1] << " "
			<< PF_info.maxWorkitemSizes[2] << std::endl;
		std::cout << std::setw(WIDTH_PF) << std::left << "Local_Memory_Size: " << PF_info.localMemSize << std::endl;
	}

	return *this;
}

CL_wrapper& CL_wrapper::checkKernelProperties(bool show)
{
	vec_string names;
	vec_size_t wg_sizes;
	vec_array3 cwg_sizes;
	vec_size_t pm_sizes;
	vec_size_t lm_sizes;
	vec_size_t pwg_sizes;

	int maxLength = 10;

	int index = 0;
	for (auto i = kernels.cbegin(); i != kernels.cend(); ++i, ++index) {
		std::string name = i->first;
		size_t wg_size = i->second.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(devices[0]);
		cl::size_t<3> cwg_size = i->second.getWorkGroupInfo<CL_KERNEL_COMPILE_WORK_GROUP_SIZE>(devices[0]);
		size_t pm_size = i->second.getWorkGroupInfo<CL_KERNEL_PRIVATE_MEM_SIZE>(devices[0]);
		size_t lm_size = i->second.getWorkGroupInfo<CL_KERNEL_LOCAL_MEM_SIZE>(devices[0]);
		size_t pwg_size = i->second.getWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(devices[0]);

		maxLength = max(maxLength, name.size());

		names.push_back(name);
		wg_sizes.push_back(wg_size);
		cwg_sizes.push_back(cwg_size);
		pm_sizes.push_back(pm_size);
		lm_sizes.push_back(lm_size);
		pwg_sizes.push_back(pwg_size);

		KN_info.workgroupSizes.push_back(wg_size);
		KN_info.compileWorkgroupSizes.push_back(cwg_size);
		KN_info.privateMemSizes.push_back(pm_size);
		KN_info.localMemSizes.push_back(lm_size);
		KN_info.preferredWorkgroupSizeMultiple.push_back(pwg_size);
	}

	maxLength += 1;

	if (show) {
		std::cout << std::setw(WIDTH_KN) << std::left << "Kernel_Properties: "
			<< std::setw(maxLength) << std::left << "<kernel>  "
			<< std::setw(7) << std::left << "wg_size" << " | "
			<< std::setw(8) << std::left << "cwg_sizes" << " | "
			<< std::setw(7) << std::left << "pm_size" << " | "
			<< std::setw(7) << std::left << "lm_size" << " | "
			<< std::setw(7) << std::left << "pwg_size" << std::endl;

		index = 0;
		for (auto i = kernels.cbegin(); i != kernels.cend(); ++i, ++index) {
			std::string cwgStr = "[" + std::to_string(cwg_sizes[index][0]) 
				+ "," + std::to_string(cwg_sizes[index][1]) 
				+ "," + std::to_string(cwg_sizes[index][2]) + "]";
			std::cout << std::setw(WIDTH_KN) << std::left << ""
				<< std::setw(maxLength) << std::left << names[index]
				<< std::setw(7) << std::left << wg_sizes[index] << " | "
				<< std::setw(9) << std::left << cwgStr << " | "
				<< std::setw(7) << std::left << pm_sizes[index] << " | "
				<< std::setw(7) << std::left << lm_sizes[index] << " | "
				<< std::setw(7) << std::left << pwg_sizes[index] << std::endl;
		}
	}

	return *this;
}

void CL_wrapper::printTaskTime(std::string taskName)
{
	startTime = event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	endTime	  = event.getProfilingInfo<CL_PROFILING_COMMAND_END>();

	printf("Time of %s is %.2f ms\n", taskName.c_str(), (endTime - startTime) / 1000000.0f);
}

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