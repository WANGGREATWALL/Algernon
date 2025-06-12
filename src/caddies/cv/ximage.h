#ifndef __XIMAGE_H__
#define __XIMAGE_H__

#include <string>
#include <map>
#include <initializer_list>
#include <type_traits>

#include "stdint.h"

namespace cv {

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
    kXFormatRawPackedU10    = 31
};

enum XImagePlane : int {
    Plane0                  = 0,
    Plane1                  = 1,
    Plane2                  = 2,
    Plane3                  = 3
};

struct Image {
    int                     width;
    int                     height;
    int                     format;
    uint8_t*                data[4];
    int                     stride[4];
};


struct ImageRaw : public Image {
    int                     fd[4];
    int                     fdOffset[4];
    int                     scanline[4]; /* height of each plane */
    int                     dataSize[4]; /* mem size of each plane */
};


// for Image
bool isValid(const Image& image);
bool isFormat(const Image& image, const int& fmt);
bool isFormatIn(const Image& image, const std::initializer_list<int>& fmts);
bool isSameWith(const Image& image, const Image& imageWith);
bool isSameSizeWith(const Image& image, const Image& imageWith);
bool isSameFormatWith(const Image& image, const Image& imageWith);
bool isSameSizeAndFormatWith(const Image& image, const Image& imageWith);

std::string info(const Image& image);


class XImage : public ImageRaw {
public:
    XImage();
    ~XImage();

    XImage(void* mempool, uint32_t width, uint32_t height, int format);
    XImage(void* mempool, uint32_t width, uint32_t height, XImageFormat format);

    /**
     * @brief support for single channel format
     */
    XImage(uint32_t width, uint32_t height, uint32_t stride, int format, uint8_t* data0, uint8_t* data1=nullptr);
    XImage(uint32_t width, uint32_t height, uint32_t stride, XImageFormat format, uint8_t* data0, uint8_t* data1=nullptr);

    XImage(const Image& image);                    /* keep image memory */
    XImage& operator=(const Image& image);         /* keep image memory */

    XImage(const XImage& image);                    /* keep image memory */
    XImage& operator=(const XImage& image);         /* keep image memory */

    XImage(XImage&& image) noexcept;                /* move image memory */
    XImage& operator=(XImage&& image) noexcept;     /* move image memory */

    std::string info() const;

    template <class T=uint8_t, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    T* dataptr(const XImagePlane& channel=cv::Plane0, const uint32_t& row=0, const uint32_t& col=0)
    {
        return (T*)(data[channel] + row * stride[channel]) + col;
    }

    template <class T=uint8_t, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
    const T* dataptr(const XImagePlane& channel=cv::Plane0, const uint32_t& row=0, const uint32_t& col=0) const
    {
        return (T*)(data[channel] + row * stride[channel]) + col;
    }

    bool isValid() const;
    bool isFormat(const int& fmt) const;
    bool isFormat(const XImageFormat& fmt) const;
    bool isFormatIn(const std::initializer_list<int>& fmts) const;
    bool isFormatIn(const std::initializer_list<XImageFormat>& fmts) const;
    bool isSameWith(const Image& image) const;
    bool isSameSizeWith(const Image& image) const;
    bool isSameFormatWith(const Image& image) const;
    bool isSameSizeAndFormatWith(const Image& image) const;

private:
    void createImage(void* mempool, uint32_t width, uint32_t height, int format);
    void createImage(uint32_t width, uint32_t height, uint32_t stride, int format, uint8_t* data0, uint8_t* data1=nullptr);

    void deleteImage();

    void copyImage(const Image& image);
    void copyImage(const XImage& image);

    void resetImage();

private:
    void*   mMempool        = nullptr;
    bool    mNeedDestroy    = false;
    bool    mIsRaw          = false;
};


} // namespace cv

#endif // __XIMAGE_H__