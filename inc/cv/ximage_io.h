#ifndef ALGERNON_CV_XIMAGE_IO_H_
#define ALGERNON_CV_XIMAGE_IO_H_

/**
 * @file ximage_io.h
 * @brief Image load/save utilities.
 *
 * Supported formats: JPEG, PNG, NV21/NV12, gray, raw.
 *
 * @example
 *   auto img = algernon::cv::XImageIO::load("/data/photo.jpg");
 *   algernon::cv::XImageIO::dump(img, "/data/output");
 */

#include "cv/ximage.h"

namespace algernon { namespace cv {

class XImageIO {
public:
    static void setDumpEnable(bool enable);
    static void setDumpFolder(const std::string& folder);
    static void updateDumpTimestamp();

    /**
     * @brief Load an image from file.
     * @param pathFull Full path with extension, e.g. "/data/image.jpg", "/data/img_512x512.nv12".
     */
    static XImage load(const std::string& pathFull);

    /**
     * @brief Dump image to file.
     * @param image The image to save.
     * @param nameWithoutFormat Filename without extension (format auto-determined).
     */
    static int dump(const XImage& image, const std::string& nameWithoutFormat);
};

}} // namespace algernon::cv

#endif // ALGERNON_CV_XIMAGE_IO_H_
