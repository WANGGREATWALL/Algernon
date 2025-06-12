#include <cstring>

#include "cv/ximage.h"
#include "math/xmath.h"
#include "log/logger.h"

namespace cv {

static std::map<int, std::string> MapImageFormat
{
    {kXFormatInvalid         , "kXFormatInvalid"        },
    {kXFormatGrayU8          , "kXFormatGrayU8"         },
    {kXFormatGrayU16         , "kXFormatGrayU16"        },
    {kXFormatGrayU32         , "kXFormatGrayU32"        },
    {kXFormatNV12            , "kXFormatNV12"           },
    {kXFormatNV21            , "kXFormatNV21"           },
    {kXFormatUV              , "kXFormatUV"           },
    {kXFormatRGBU8           , "kXFormatRGBU8"          },
    {kXFormatBGRU8           , "kXFormatBGRU8"          },
    {kXFormatRGBAU8          , "kXFormatRGBAU8"         },
    {kXFormatBGRAU8          , "kXFormatBGRAU8"         },
    {kXFormatRawU16          , "kXFormatRawU16"         },
    {kXFormatRawPackedU10    , "kXFormatRawPackedU10"   }
};

Image imageAlloc(void* mempool, uint32_t width, uint32_t height, int format)
{
    ASSERTER_WITH_RET(width > 0 && height > 0, Image{0});
    ASSERTER_WITH_RET(format > kXFormatInvalid, Image{0});

    Image image;
    image.width = width;
    image.height = height;
    image.format = format;

    memset(image.data, 0, sizeof(image.data));
    memset(image.stride, 0, sizeof(image.stride));

    if (format == kXFormatGrayU8) {
        image.stride[0] = math::ceilTo8(width);
        image.data[0] = (uint8_t*)malloc(image.height * image.stride[0]);
    } else if (format == kXFormatGrayU16 || format == kXFormatUV) {
        image.stride[0] = math::ceilTo8(width * 2);
        image.data[0] = (uint8_t*)malloc(image.height * image.stride[0]);
    } else if (format == kXFormatNV12 || format == kXFormatNV21) {
        image.stride[0] = math::ceilTo8(width);
        image.stride[1] = image.stride[0];
        image.data[0] = (uint8_t*)malloc(image.height * image.stride[0]);
        image.data[1] = (uint8_t*)malloc((image.height / 2) * image.stride[1]);
    } else if (format == kXFormatRGBU8 || format == kXFormatBGRU8) {
        image.stride[0] = math::ceilTo8(width * 3);
        image.data[0] = (uint8_t*)malloc(image.height * image.stride[0]);
    } else if (format == kXFormatGrayU32 || format == kXFormatRGBAU8 || format == kXFormatBGRAU8) {
        image.stride[0] = math::ceilTo8(width * 4);
        image.data[0] = (uint8_t*)malloc(image.height * image.stride[0]);
    }

    return image;
}

void imageFree(void* mempool, Image& image)
{
    for (int i = 0; i < 4; ++i) {
        if (image.data[i]) {
            free(image.data[i]);
            image.data[i] = nullptr;
        }
    }

    memset(&image, 0, sizeof(Image));
}

bool isValid(const Image& image)
{
    bool validFormat = image.format > 0;
    bool validSize = (image.width > 0) && (image.height > 0) && (image.stride[0] >= image.width);
    bool validData = (image.data[0] != nullptr);
    return validFormat && validSize && validData;
}

bool isFormat(const Image& image, const int& fmt)
{
    return image.format == fmt;
}

bool isFormatIn(const Image& image, const std::initializer_list<int>& fmts)
{
    for (const auto& fmt : fmts) {
        if (image.format == fmt) return true;
    }
    return false;
}

bool isSameWith(const Image& image, const Image& imageWith)
{
    return isSameSizeAndFormatWith(image, imageWith) && (image.data[0] == imageWith.data[0]);
}

bool isSameSizeWith(const Image& image, const Image& imageWith)
{
    if (image.format == imageWith.format) {
        return (image.width == imageWith.width) && (image.height == imageWith.height) && (image.stride[0] == imageWith.stride[0]);
    }

    // else:
    return (image.width == imageWith.width) && (image.height == imageWith.height);
}

bool isSameFormatWith(const Image& image, const Image& imageWith)
{
    return image.format == imageWith.format;
}

bool isSameSizeAndFormatWith(const Image& image, const Image& imageWith)
{
    return isSameSizeWith(image, imageWith) && isSameFormatWith(image, imageWith);
}

std::string info(const Image& image)
{
    char str[256];

    snprintf(str, 256, "[%dx%d|%d,%d,%d,%d], fmt:%s, data:[%p,%p,%p,%p]", 
        image.width, image.height, image.stride[0], image.stride[1], image.stride[2], image.stride[3],
        MapImageFormat[image.format].c_str(), image.data[0], image.data[1], image.data[2], image.data[3]);
    
    return std::string(str);
}


XImage::XImage()
    : mMempool(nullptr)
    , mNeedDestroy(false)
    , mIsRaw(false)
{
    resetImage();
}

XImage::~XImage()
{
    deleteImage();
}

XImage::XImage(void* mempool, uint32_t width, uint32_t height, int format)
    : mMempool(nullptr)
    , mIsRaw(false)
{
    createImage(mempool, width, height, format);
}

XImage::XImage(void* mempool, uint32_t width, uint32_t height, XImageFormat format)
    : mMempool(nullptr)
    , mIsRaw(false)
{
    createImage(mempool, width, height, format);
}

XImage::XImage(uint32_t width, uint32_t height, uint32_t stride, int format, uint8_t* data0, uint8_t* data1)
    : mMempool(nullptr)
    , mIsRaw(false)
{
    createImage(width, height, stride, format, data0, data1);
}

XImage::XImage(uint32_t width, uint32_t height, uint32_t stride, XImageFormat format, uint8_t* data0, uint8_t* data1)
    : mMempool(nullptr)
    , mIsRaw(false)
{
    createImage(width, height, stride, format, data0, data1);
}

XImage::XImage(const Image& image)
    : mMempool(nullptr)
    , mNeedDestroy(false)
{
    copyImage(image);
}

XImage& XImage::operator=(const Image& image)
{
    if (!isSameWith(image)) {
        deleteImage();
        copyImage(image);
    }
    return *this;
}

XImage::XImage(const XImage& image)
{
    copyImage(image);
}

XImage& XImage::operator=(const XImage& image)
{
    if (!isSameWith(image)) {
        deleteImage();
        copyImage(image);
    }
    return *this;
}

XImage::XImage(XImage&& image) noexcept
{
    copyImage(image);
    this->mMempool = image.mMempool;
    this->mNeedDestroy = image.mNeedDestroy;

    image.mNeedDestroy = false; // discard memory control
}

XImage& XImage::operator=(XImage&& image) noexcept
{
    if (!isSameWith(image)) {
        deleteImage();
        copyImage(image);
        
        this->mMempool = image.mMempool;
        this->mNeedDestroy = image.mNeedDestroy;

        image.mNeedDestroy = false; // discard memory control
    }
    return *this;
}

void XImage::createImage(void* mempool, uint32_t width, uint32_t height, int format)
{
    Image image = imageAlloc(mempool, width, height, format);
    
    if (image.data[0] != nullptr) {
        copyImage(image);

        mMempool = mempool;
        mNeedDestroy = true;
    }
}

void XImage::createImage(uint32_t width, uint32_t height, uint32_t stride, int format, uint8_t* data0, uint8_t* data1)
{
    Image image{
        format, 
        (int)width, 
        (int)height, 
        {data0, data1, nullptr, nullptr},
        {(int)stride, 0, 0, 0}
    };

    ASSERTER(stride >= width);
    ASSERTER(cv::isFormatIn(image, {
        kXFormatGrayU8,
        kXFormatGrayU16,
        kXFormatNV12,
        kXFormatNV21,
        kXFormatRGBU8,
        kXFormatBGRU8,
        kXFormatRGBAU8,
        kXFormatBGRAU8,
        kXFormatRawU16,
        kXFormatRawPackedU10}));
       
    if (image.data[0] != nullptr) {
        copyImage(image);

        mMempool = nullptr;
        mNeedDestroy = false;
    }
}

void XImage::deleteImage()
{
    if (mNeedDestroy && isValid()) {
        if (mIsRaw) {
            delete data[0];
        }
        else {
            imageFree(mMempool, *this);
        }

        resetImage();
        mMempool = nullptr;
        mNeedDestroy = false;
    }
}

void XImage::copyImage(const Image& image)
{
    format = image.format;
    width = image.width;
    height = image.height;
    memcpy(stride, image.stride, sizeof(stride));
    memcpy(data, image.data, sizeof(data));
}

void XImage::copyImage(const XImage& image)
{
    mIsRaw = image.mIsRaw;

    format = image.format;
    width = image.width;
    height = image.height;

    memcpy(stride, image.stride, sizeof(stride));
    memcpy(scanline, image.scanline, sizeof(scanline));
    memcpy(data, image.data, sizeof(data));
    memcpy(dataSize, image.dataSize, sizeof(dataSize));
    memcpy(fd, image.fd, sizeof(fd));
    memcpy(fdOffset, image.fdOffset, sizeof(fdOffset));
}

void XImage::resetImage()
{
    format = 0;
    width = 0;
    height = 0;

    memset(stride, 0, sizeof(stride));
    memset(scanline, 0, sizeof(scanline));
    memset(data, 0, sizeof(data));
    memset(dataSize, 0, sizeof(dataSize));
    memset(fd, 0, sizeof(fd));
    memset(fdOffset, 0, sizeof(fdOffset));
}

std::string XImage::info() const
{
    char str[256];

    snprintf(str, 256, "[%dx%d|%d,%d,%d,%d], fmt:%s, data:[%p,%p,%p,%p]", 
        width, height, stride[0], stride[1], stride[2], stride[3],
        MapImageFormat[format].c_str(), data[0], data[1], data[2], data[3]);
    
    return std::string(str);
}

bool XImage::isValid() const
{
    return cv::isValid(*this);
}

bool XImage::isFormat(const int& fmt) const
{
    return cv::isFormat(*this, fmt);
}

bool XImage::isFormat(const XImageFormat& fmt) const
{
    return format == fmt;
}

bool XImage::isFormatIn(const std::initializer_list<int>& fmts) const
{
    return cv::isFormatIn(*this, fmts);
}

bool XImage::isFormatIn(const std::initializer_list<XImageFormat>& fmts) const
{
    for (const auto& fmt : fmts) {
        if (this->format == fmt) return true;
    }
    return false;
}

bool XImage::isSameWith(const Image& image) const
{
    return cv::isSameWith(*this, image);
}

bool XImage::isSameSizeWith(const Image& image) const
{
    return cv::isSameSizeWith(*this, image);
}

bool XImage::isSameFormatWith(const Image& image) const
{
    return cv::isSameFormatWith(*this, image);
}

bool XImage::isSameSizeAndFormatWith(const Image& image) const
{
    return cv::isSameSizeAndFormatWith(*this, image);
}


} // namespace cv