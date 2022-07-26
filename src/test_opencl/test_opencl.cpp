#include <iostream>
#include "test.h"
#include "test_opencl/cl_wrapper.h"
#include "opencv2/opencv.hpp"

void cl_boxblur() 
{
	std::string inPath = "../image/NaNa.jpeg";
	cv::Mat srcImg = cv::imread(inPath);
	if (srcImg.empty())
		std::cerr << "Failed to open " << inPath << std::endl;

	CL_Wrapper clWrapper;
	DeviceInfo deviceInfo = clWrapper.getDeviceInfos();
	deviceInfo.show();

	return;
}

void test_OpenCL() {
	cl_boxblur();
}