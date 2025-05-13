#ifndef __XIMAGE_H__
#define __XIMAGE_H__

#include <string>
#include <map>
#include <initializer_list>
#include <type_traits>

#include "vivo_comdef.h"
#include "VivoType.h"
#include "stdint.h"

namespace cv {

    using XFormat = VImageFormat;
    using XPackedType = VRawPackedType;
    using XBayerType = VBayerFilter;

    struct Image {
        int             width;
        int             height;
        int             format;
        uint8_t*        data[4];
        int             stride[4];
    };

    struct VImageRawExt: public Image {
        int             fd[4];
        int             fdOffset[4];

        int             scanline[4];    /* height of each plane */
        int             dataSize[4];    /* mem size of each plane */

        int             bitPerPixel;    /* define the image bits per pixel */
        VBayerFilter    bayerFilter;    /* define sensor bayer color filter pattern */
        VRawPackedType  packedType;     /* define raw image packed type */
    };

    enum XImagePlane {
        Plane0 = 0,
        Plane1 = 1,
        Plane2 = 2,
        Plane3 = 3
    };

    class XImage: public VImageRawExt {
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
    
    public:
        XImage();
        ~XImage();

        XImage(void* mempool, uint32_t width, uint32_t height, XFormat format);
        XImage(void* mempool, uint32_t width, uint32_t height, XBayerType bayer, XPackedType packed, int bitPerPixel);

        /**
         * @brief support for single channel format
         */
        XImage(uint32_t width, uint32_t height, XFormat format, uint8_t* data);

        XImage(const VImage& image);                    /* keep image memory */
        XImage& operator=(const VImage& image);         /* keep image memory */

        XImage(const XImage& image);                    /* keep image memory */
        XImage& operator=(const XImage& image);         /* keep image memory */

        XImage(const VRawImage& image);                 /* keep image memory */
        XImage& operator=(const VRawImage& image);      /* keep image memory */

        XImage(const VRawImageV1& image);               /* keep image memory */
        XImage& operator=(const VRawImageV1& image);    /* keep image memory */

        XImage(const VImageExV1& image);                /* keep image memory */
        XImage& operator=(const VImageExV1& image);     /* keep image memory */

        XImage(XImage&& image) noexcept;                /* move image memory */
        XImage& operator=(XImage&& image) noexcept;     /* move image memory */

        operator VRawImage() const;
        operator VRawImageV1() const;

        operator VImageEx() const;
        operator VImageExV1() const;

        std::string info() const;

        template <class T=uint8_t, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
        T* dataptr(XImagePlane channel) {
            return (T*)(data[channel]);
        }

        template <class T=uint8_t, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
        const T* dataptr(XImagePlane channel) const {
            return (T*)(data[channel]);
        }

        template <class T=uint8_t, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
        T* dataptr(XImagePlane channel, uint32_t row, uint32_t col=0) {
            return (T*)(data[channel] + row * stride[channel]) + col;
        }

        template <class T=uint8_t, class = typename std::enable_if<std::is_arithmetic<T>::value>::type>
        const T* dataptr(XImagePlane channel, uint32_t row, uint32_t col=0) const {
            return (T*)(data[channel] + row * stride[channel]) + col;
        }

        bool isValid() const;
        bool isFormat(VImageFormat fmt) const;
        bool isFormat(XBayerType fmt) const;
        bool isFormat(XPackedType fmt) const;
        bool isFormatIn(const std::initializer_list<VImageFormat>& fmts) const;
        bool isSameWith(const VImage& image) const;
        bool isSameSizeWith(const VImage& image) const;
        bool isSameFormatWith(const VImage& image) const;
        bool isSameSizeAndFormatWith(const VImage& image) const;
    
    private:
        void createImage(void* mempool, uint32_t width, uint32_t height, int format);
        void createImage(void* mempool, uint32_t width, uint32_t height, XBayerType bayer, XPackedType packed, int bitPerPixel);
        void createImage(uint32_t width, uint32_t height, int format, uint8_t* data);

        void deleteImage();

        void copyImage(const VImage& image);
        void copyImage(const XImage& image);
        void copyImage(const VRawImage& image);
        void copyImage(const VRawImageV1& image);
        void copyImage(const VImageExV1& image);

        void resetImage();

        bool isValidFormat() const;
    
    private:
        void*   mMempool        = nullptr;
        bool    mNeedDestroy    = false;
        bool    mIsRaw          = false;
    };

} // namespace cv

#endif // __XIMAGE_H__