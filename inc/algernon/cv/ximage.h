#ifndef ALGERNON_CV_XIMAGE_H_
#define ALGERNON_CV_XIMAGE_H_

/**
 * @file ximage.h
 * @brief Image container with multi-plane support and format utilities.
 *
 * @example
 *   algernon::cv::XImage img(mempool, 1920, 1080, algernon::cv::kXFormatNV21);
 *   auto* ptr = img.dataptr<uint8_t>(algernon::cv::Plane0, row, col);
 */

#include <string>
#include <initializer_list>
#include <type_traits>
#include <cstdint>

namespace algernon { namespace cv {

enum XImageFormat : int {
    kXFormatInvalid         = 0,
    kXFormatGrayU8          = 1,
    kXFormatGrayU16         = 2,
    kXFormatGrayU32         = 3,
    kXFormatNV12            = 10,
    kXFormatNV21            = 11,
    kXFormatUV              = 12,
    kXFormatRGBU8           = 20,
    kXFormatBGRU8           = 21,
    kXFormatRGBAU8          = 22,
    kXFormatBGRAU8          = 23,
    kXFormatRawU16          = 30,
    kXFormatRawPackedU10    = 31,
};

enum XImagePlane : int {
    Plane0 = 0, Plane1 = 1, Plane2 = 2, Plane3 = 3,
};

struct Image {
    int       width   = 0;
    int       height  = 0;
    int       format  = 0;
    uint8_t*  data[4]   = {};
    int       stride[4] = {};
};

struct ImageRaw : public Image {
    int fd[4]       = {};
    int fdOffset[4] = {};
    int scanline[4] = {};
    int dataSize[4] = {};
};

// -- Free functions for Image --
bool isValid(const Image& image);
bool isFormat(const Image& image, int fmt);
bool isFormatIn(const Image& image, const std::initializer_list<int>& fmts);
bool isSameWith(const Image& image, const Image& other);
bool isSameSizeWith(const Image& image, const Image& other);
bool isSameFormatWith(const Image& image, const Image& other);
bool isSameSizeAndFormatWith(const Image& image, const Image& other);
std::string info(const Image& image);

class XImage : public ImageRaw {
public:
    XImage();
    ~XImage();

    XImage(void* mempool, uint32_t width, uint32_t height, int format);
    XImage(void* mempool, uint32_t width, uint32_t height, XImageFormat format);
    XImage(uint32_t width, uint32_t height, uint32_t stride, int format,
           uint8_t* data0, uint8_t* data1 = nullptr);
    XImage(uint32_t width, uint32_t height, uint32_t stride, XImageFormat format,
           uint8_t* data0, uint8_t* data1 = nullptr);

    XImage(const Image& image);
    XImage& operator=(const Image& image);

    XImage(const XImage& image);
    XImage& operator=(const XImage& image);

    XImage(XImage&& image) noexcept;
    XImage& operator=(XImage&& image) noexcept;

    std::string info() const;

    template <class T = uint8_t, class = std::enable_if_t<std::is_arithmetic_v<T>>>
    T* dataptr(XImagePlane channel = Plane0, uint32_t row = 0, uint32_t col = 0) {
        return reinterpret_cast<T*>(data[channel] + row * stride[channel]) + col;
    }

    template <class T = uint8_t, class = std::enable_if_t<std::is_arithmetic_v<T>>>
    const T* dataptr(XImagePlane channel = Plane0, uint32_t row = 0, uint32_t col = 0) const {
        return reinterpret_cast<const T*>(data[channel] + row * stride[channel]) + col;
    }

    bool isValid() const;
    bool isFormat(int fmt) const;
    bool isFormat(XImageFormat fmt) const;
    bool isFormatIn(const std::initializer_list<int>& fmts) const;
    bool isFormatIn(const std::initializer_list<XImageFormat>& fmts) const;
    bool isSameWith(const Image& image) const;
    bool isSameSizeWith(const Image& image) const;
    bool isSameFormatWith(const Image& image) const;
    bool isSameSizeAndFormatWith(const Image& image) const;

private:
    void createImage(void* mempool, uint32_t width, uint32_t height, int format);
    void createImage(uint32_t width, uint32_t height, uint32_t stride, int format,
                     uint8_t* data0, uint8_t* data1 = nullptr);
    void deleteImage();
    void copyImage(const Image& image);
    void copyImage(const XImage& image);
    void resetImage();

    void*  mMempool     = nullptr;
    bool   mNeedDestroy = false;
    bool   mIsRaw       = false;
};

}} // namespace algernon::cv

#endif // ALGERNON_CV_XIMAGE_H_
