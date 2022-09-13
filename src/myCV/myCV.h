#ifndef __MYCV__
#define __MYCV__

#include "opencv2/opencv.hpp"

//! @brief box filter
//! @param dstImg: dst image
//! @param srcImg: src image
//! @param radius: radius
//! @return -1 if failed
int boxFilter(cv::Mat& dstImg, cv::Mat& srcImg, int& radius);

#endif