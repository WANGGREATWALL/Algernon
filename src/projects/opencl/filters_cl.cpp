#include "MYTEST.h"
#include "myUtils/myUtils.h"
#include "myCL/cl_wrapper.h"
#include "opencv2/opencv.hpp"

void cl_boxFilter()
{
	int err = CL_SUCCESS;
	std::string srcimgPath = "../image/NaNa.jpeg";
	std::string clfilePath = "../src/projects/opencl/filters.cl";

	cv::Mat srcImg = cv::imread(srcimgPath, cv::IMREAD_GRAYSCALE);
	cv::Mat dstImg(srcImg.rows, srcImg.cols, CV_8UC1);
	CHECK_ERR(srcImg.empty(), "Fail to open %s", srcimgPath.c_str());

	//! Create Platform, Device, Context, CommandQueue
	CL_wrapper wrapper;

	//! Build Program
	std::string options = "-cl-std=CL1.2";
	wrapper.build(clfilePath, options).makeKernel();

	//! Show DeviceInfo
	wrapper.checkPlatformDevice(true).checkImageCapacity(true).checkPerformanceInfo(true).checkKernelProperties(true);

	//! Set Args
	cl_channel_order channelOrder(CL_R);
	cl_channel_type channelType(CL_UNSIGNED_INT8);
	cl::ImageFormat imageFormat(channelOrder, channelType);
	cl::Image2D clImgSrc(wrapper.context(), CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, imageFormat, srcImg.cols, srcImg.rows, 0, srcImg.data, &err);
	CHECK_ERR_CL(err);

	cl::Image2D clImgDst(wrapper.context(), CL_MEM_WRITE_ONLY, imageFormat, srcImg.cols, srcImg.rows, 0, NULL, &err);
	CHECK_ERR_CL(err);

	cl::Kernel kernel = wrapper.kernel("boxFilter");
	CHECK_ERR_CL(kernel.setArg(0, clImgSrc));
	CHECK_ERR_CL(kernel.setArg(1, clImgDst));

	//! Enqueue Command
	cl::NDRange offset(0, 0);
	cl::NDRange global(2, 2);
	cl::NDRange local(1, 1);
	cl::size_t<3> origin;
	cl::size_t<3> region; 
	region[0] = srcImg.cols;
	region[1] = srcImg.rows;
	region[2] = 1;

	kernel = wrapper.kernel("hello");
	CHECK_ERR_CL(wrapper.commandQueue().enqueueNDRangeKernel(kernel, offset, global, local, NULL, &wrapper.event));

	global = cl::NDRange(region[0], region[1]);
	kernel = wrapper.kernel("boxFilter");
	CHECK_ERR_CL(wrapper.commandQueue().enqueueNDRangeKernel(kernel, offset, global, local, NULL, &wrapper.event));
	wrapper.timer("boxFilter");

	CHECK_ERR_CL(wrapper.commandQueue().enqueueReadImage(clImgDst, CL_TRUE, origin, region, 0, 0, dstImg.data, NULL, &wrapper.event));
	CHECK_ERR_CL(wrapper.commandQueue().finish());

	return;
}

void test_OpenCL() 
{
	cl_boxFilter();
}