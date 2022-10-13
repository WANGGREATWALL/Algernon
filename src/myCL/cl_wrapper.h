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
std::string getAlgoVersion();

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

	void printTaskTime(std::string taskName);

	cl::Event					event;

private:
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

#define UINT8		0
#define INT8		1
#define UINT16		2
#define INT16		3
#define INT32		4
#define FLOAT32		5

class CLImgViewer {
public:
	CLImgViewer() = default;
	CLImgViewer(cl::CommandQueue &commandQueue, cl::Image2D& cl_image2d) : commandQ(commandQueue), image(cl_image2d)
	{
		int err = 0;
		cols = cl_image2d.getImageInfo<CL_IMAGE_WIDTH>(&err);
		rows = cl_image2d.getImageInfo<CL_IMAGE_HEIGHT>(&err);
		step = cl_image2d.getImageInfo<CL_IMAGE_ROW_PITCH>(&err);
		CHECK_ERR_CL(err);

		cl::size_t<3> origin;
		cl::size_t<3> region;
		region[0] = cols;
		region[1] = rows;
		region[2] = 1;

		size_t row_pitch, slice_pitch;
		data = (uint8_t*)commandQ.enqueueMapImage(cl_image2d, true,
			CL_MAP_READ, origin, region,
			&row_pitch, &slice_pitch, NULL, NULL, &err);
		CHECK_ERR_CL(err);
	}
	~CLImgViewer() 
	{ 
		commandQ.enqueueUnmapMemObject(image, data, NULL, NULL);
	}
	/** type:
	*	UINT8:		0;
	*	INT8:		1;
	*	UINT16:		2;
	*	INT16:		3;
	*	INT32:		4;
	*	FLOAT32:	5;
	*/
	int type		= UINT8;
	int rows		= 0;
	int cols		= 0;
	int step		= 0;
	int layer		= 1;
	int channel		= 1;
	uint8_t* data	= nullptr;

private:
	cl::CommandQueue	commandQ;
	cl::Image2D			image;
};