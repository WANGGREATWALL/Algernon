#ifndef __XIMAGE_IO_H__
#define __XIMAGE_IO_H__

#include "cv/ximage.h"

namespace cv {

class XImageIO
{
public:
    static void setDumpEnable(bool enable);
    static void setDumpFolder(std::string folder);
    static void updateDumpTimestamp();

    /**
     * @brief format supported:
     *  - jpeg;
     *  - png;
     *  - nv21 / nv12;
     *  - gray;
     *  - raw;
     * @param pathFull for example:
     *  - "/data/hello.jpg";
     *  - "/data/hello_512x512.nv12";
     */
    static XImage load(std::string pathFull);

    /**
     * @brief format supported:
     *  - cv::XFormat::kVIFormatGray;
     *  - cv::XFormat::kVIFormatGrayU16;
     *  - cv::XFormat::kVIFormatNV12;
     *  - cv::XFormat::kVIFormatNV21;
     *  - cv::XFormat::kVIFormatR8G8B8;
     *  - cv::XFormat::kVIFormatRaw16;
     *  - cv::XFormat::kVIFormatRawPacked10;
     */
    static int dump(const XImage& image, std::string nameWithoutFormat);

};

} // namespace cv

#endif // __XIMAGE_IO_H__