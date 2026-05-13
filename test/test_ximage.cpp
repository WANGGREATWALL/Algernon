#if ENABLE_TEST_XIMAGE

#include "gtest/gtest.h"
#include "cv/ximage.h"

using au::cv::XImage;
using au::cv::Image;
using au::cv::ImageRaw;
using au::cv::XImageFormat;
using au::cv::XImagePlane;

// ============================================================================
// Image struct
// ============================================================================

TEST(XImage, Image_default_values)
{
    Image img;
    EXPECT_EQ(img.width, 0);
    EXPECT_EQ(img.height, 0);
    EXPECT_EQ(img.format, 0);
    EXPECT_EQ(img.data[0], nullptr);
    EXPECT_EQ(img.stride[0], 0);
}

// ============================================================================
// Free functions: isValid
// ============================================================================

TEST(XImage, isValid_valid_image)
{
    Image img;
    img.width  = 64;
    img.height = 64;
    img.format = au::cv::kXFormatGrayU8;
    img.stride[0] = 64;
    uint8_t buf[64 * 64] = {};
    img.data[0] = buf;
    EXPECT_TRUE(au::cv::isValid(img));
}

TEST(XImage, isValid_zero_width)
{
    Image img;
    img.width = 0;
    img.height = 64;
    img.format = au::cv::kXFormatGrayU8;
    EXPECT_FALSE(au::cv::isValid(img));
}

TEST(XImage, isValid_zero_height)
{
    Image img;
    img.width = 64;
    img.height = 0;
    img.format = au::cv::kXFormatGrayU8;
    EXPECT_FALSE(au::cv::isValid(img));
}

TEST(XImage, isValid_invalid_format)
{
    Image img;
    img.width  = 64;
    img.height = 64;
    img.format = au::cv::kXFormatInvalid;
    uint8_t buf[64 * 64] = {};
    img.data[0] = buf;
    img.stride[0] = 64;
    EXPECT_FALSE(au::cv::isValid(img));
}

TEST(XImage, isValid_null_data)
{
    Image img;
    img.width  = 64;
    img.height = 64;
    img.format = au::cv::kXFormatGrayU8;
    img.stride[0] = 64;
    EXPECT_FALSE(au::cv::isValid(img));
}

TEST(XImage, isValid_stride_less_than_width)
{
    Image img;
    img.width  = 64;
    img.height = 64;
    img.format = au::cv::kXFormatGrayU8;
    img.stride[0] = 32;
    uint8_t buf[64 * 64] = {};
    img.data[0] = buf;
    EXPECT_FALSE(au::cv::isValid(img));
}

// ============================================================================
// Free functions: isFormat / isFormatIn
// ============================================================================

TEST(XImage, isFormat_exact_match)
{
    Image img;
    img.format = au::cv::kXFormatRGBU8;
    EXPECT_TRUE(au::cv::isFormat(img, au::cv::kXFormatRGBU8));
    EXPECT_FALSE(au::cv::isFormat(img, au::cv::kXFormatGrayU8));
}

TEST(XImage, isFormatIn_match_found)
{
    Image img;
    img.format = au::cv::kXFormatRGBAU8;
    EXPECT_TRUE(au::cv::isFormatIn(img, {au::cv::kXFormatRGBU8, au::cv::kXFormatRGBAU8}));
    EXPECT_FALSE(au::cv::isFormatIn(img, {au::cv::kXFormatRGBU8, au::cv::kXFormatGrayU8}));
}

TEST(XImage, isFormatIn_empty_list)
{
    Image img;
    img.format = au::cv::kXFormatGrayU8;
    EXPECT_FALSE(au::cv::isFormatIn(img, {}));
}

// ============================================================================
// Free functions: isSame*
// ============================================================================

TEST(XImage, isSameSizeWith_same_size_same_format)
{
    Image a, b;
    a.width = b.width = 64;
    a.height = b.height = 64;
    a.format = b.format = au::cv::kXFormatGrayU8;
    a.stride[0] = b.stride[0] = 64;
    EXPECT_TRUE(au::cv::isSameSizeWith(a, b));
}

TEST(XImage, isSameSizeWith_different_size)
{
    Image a, b;
    a.width = 64; b.width = 32;
    a.height = b.height = 64;
    a.format = b.format = au::cv::kXFormatGrayU8;
    EXPECT_FALSE(au::cv::isSameSizeWith(a, b));
}

TEST(XImage, isSameSizeWith_different_format)
{
    Image a, b;
    a.width = b.width = 64;
    a.height = b.height = 64;
    a.format = au::cv::kXFormatGrayU8;
    b.format = au::cv::kXFormatRGBU8;
    EXPECT_FALSE(au::cv::isSameSizeWith(a, b));
}

TEST(XImage, isSameFormatWith_match)
{
    Image a, b;
    a.format = b.format = au::cv::kXFormatRGBAU8;
    EXPECT_TRUE(au::cv::isSameFormatWith(a, b));
}

TEST(XImage, isSameFormatWith_mismatch)
{
    Image a, b;
    a.format = au::cv::kXFormatGrayU8;
    b.format = au::cv::kXFormatRGBU8;
    EXPECT_FALSE(au::cv::isSameFormatWith(a, b));
}

TEST(XImage, isSameSizeAndFormatWith)
{
    Image a, b;
    a.width = b.width = 32;
    a.height = b.height = 32;
    a.format = b.format = au::cv::kXFormatGrayU8;
    a.stride[0] = b.stride[0] = 32;
    EXPECT_TRUE(au::cv::isSameSizeAndFormatWith(a, b));
    b.height = 16;
    EXPECT_FALSE(au::cv::isSameSizeAndFormatWith(a, b));
}

TEST(XImage, isSameWith_same_data_pointer)
{
    Image a;
    a.width = a.height = 64;
    a.format = au::cv::kXFormatGrayU8;
    a.stride[0] = 64;
    uint8_t buf[64 * 64] = {};
    a.data[0] = buf;
    Image b = a;
    EXPECT_TRUE(au::cv::isSameWith(a, b));
}

TEST(XImage, isSameWith_different_data_pointer)
{
    Image a;
    a.width = a.height = 64;
    a.format = au::cv::kXFormatGrayU8;
    a.stride[0] = 64;
    uint8_t buf1[64 * 64] = {};
    uint8_t buf2[64 * 64] = {};
    a.data[0] = buf1;
    Image b = a;
    b.data[0] = buf2;
    EXPECT_FALSE(au::cv::isSameWith(a, b));
}

// ============================================================================
// Free function: info
// ============================================================================

TEST(XImage, info_returns_non_empty)
{
    Image img;
    img.width = 10;
    img.height = 20;
    img.format = au::cv::kXFormatGrayU8;
    img.stride[0] = 16;
    uint8_t buf[320] = {};
    img.data[0] = buf;
    std::string s = au::cv::info(img);
    EXPECT_FALSE(s.empty());
    EXPECT_NE(s.find("10x20"), std::string::npos);
}

// ============================================================================
// XImage: default construction
// ============================================================================

TEST(XImage, default_ctor_invalid)
{
    XImage img;
    EXPECT_FALSE(img.isValid());
    EXPECT_EQ(img.data[0], nullptr);
    EXPECT_EQ(img.width, 0);
}

// ============================================================================
// XImage: allocation construction
// ============================================================================

TEST(XImage, alloc_ctor_gray_u8)
{
    XImage img(nullptr, 64, 64, au::cv::kXFormatGrayU8);
    EXPECT_TRUE(img.isValid());
    EXPECT_EQ(img.width, 64);
    EXPECT_EQ(img.height, 64);
    EXPECT_EQ(img.format, au::cv::kXFormatGrayU8);
    EXPECT_NE(img.data[0], nullptr);
    // stride should be ceilTo8(64) = 64
}

TEST(XImage, alloc_ctor_rgba_u8)
{
    XImage img(nullptr, 32, 32, au::cv::kXFormatRGBAU8);
    EXPECT_TRUE(img.isValid());
    EXPECT_NE(img.data[0], nullptr);
}

TEST(XImage, alloc_ctor_nv12)
{
    XImage img(nullptr, 64, 64, au::cv::kXFormatNV12);
    EXPECT_TRUE(img.isValid());
    EXPECT_NE(img.data[0], nullptr);
    EXPECT_NE(img.data[1], nullptr);  // NV12 has 2 planes
}

TEST(XImage, alloc_ctor_nv21)
{
    XImage img(nullptr, 64, 64, au::cv::kXFormatNV21);
    EXPECT_TRUE(img.isValid());
    EXPECT_NE(img.data[0], nullptr);
    EXPECT_NE(img.data[1], nullptr);
}

TEST(XImage, alloc_ctor_enum_overload)
{
    XImage img(nullptr, 64, 64, XImageFormat::kXFormatRGBU8);
    EXPECT_TRUE(img.isValid());
    EXPECT_EQ(img.format, au::cv::kXFormatRGBU8);
}

TEST(XImage, alloc_ctor_zero_width_returns_invalid)
{
    XImage img(nullptr, 0, 64, au::cv::kXFormatGrayU8);
    EXPECT_FALSE(img.isValid());
}

TEST(XImage, alloc_ctor_zero_height_returns_invalid)
{
    XImage img(nullptr, 64, 0, au::cv::kXFormatGrayU8);
    EXPECT_FALSE(img.isValid());
}

// ============================================================================
// XImage: wrapper construction
// ============================================================================

TEST(XImage, wrapper_ctor_wraps_external_buffer)
{
    uint8_t buf[64 * 64 * 4] = {};
    XImage img(64, 64, 64 * 4, au::cv::kXFormatRGBAU8, buf);
    EXPECT_TRUE(img.isValid());
    EXPECT_EQ(img.data[0], buf);
    // Wrapper images should not own the memory
}

TEST(XImage, wrapper_ctor_enum_overload)
{
    uint8_t buf[64 * 64] = {};
    XImage img(64, 64, 64, XImageFormat::kXFormatGrayU8, buf);
    EXPECT_TRUE(img.isValid());
    EXPECT_EQ(img.data[0], buf);
}

// ============================================================================
// XImage: dataptr
// ============================================================================

TEST(XImage, dataptr_default_plane0_row0_col0)
{
    XImage img(nullptr, 64, 64, au::cv::kXFormatGrayU8);
    auto* p = img.dataptr<uint8_t>();
    EXPECT_EQ(p, img.data[0]);
}

TEST(XImage, dataptr_with_row_and_col)
{
    XImage img(nullptr, 64, 64, au::cv::kXFormatGrayU8);
    // stride is ceilTo8(64) = 64, so row 1 starts at offset 64
    auto* p = img.dataptr<uint8_t>(XImagePlane::Plane0, 1, 0);
    EXPECT_EQ(p, img.data[0] + 64);
    auto* p2 = img.dataptr<uint8_t>(XImagePlane::Plane0, 1, 3);
    EXPECT_EQ(p2, img.data[0] + 67);
}

TEST(XImage, dataptr_const_version)
{
    XImage img(nullptr, 64, 64, au::cv::kXFormatGrayU8);
    const XImage& cimg = img;
    const auto* p = cimg.dataptr<uint8_t>();
    EXPECT_EQ(p, img.data[0]);
}

TEST(XImage, dataptr_different_types)
{
    XImage img(nullptr, 64, 64, au::cv::kXFormatGrayU16);
    auto* pu16 = img.dataptr<uint16_t>();
    EXPECT_NE(reinterpret_cast<void*>(pu16), nullptr);
    auto* pu32 = img.dataptr<uint32_t>();
    EXPECT_NE(reinterpret_cast<void*>(pu32), nullptr);
}

// ============================================================================
// XImage: copy semantics
// ============================================================================

TEST(XImage, copy_ctor_shares_pointer)
{
    XImage a(nullptr, 32, 32, au::cv::kXFormatGrayU8);
    XImage b(a);
    EXPECT_TRUE(b.isValid());
    // Copy is shallow — same data pointer
    EXPECT_EQ(a.data[0], b.data[0]);
}

TEST(XImage, copy_assign_from_image)
{
    XImage a(nullptr, 32, 32, au::cv::kXFormatGrayU8);
    Image raw = a;  // upcast
    XImage b;
    b = raw;
    EXPECT_TRUE(b.isValid());
}

TEST(XImage, copy_assign_from_ximage)
{
    XImage a(nullptr, 64, 64, au::cv::kXFormatRGBU8);
    XImage b;
    b = a;
    EXPECT_TRUE(b.isValid());
    EXPECT_EQ(b.width, 64);
    EXPECT_EQ(b.format, au::cv::kXFormatRGBU8);
}

// ============================================================================
// XImage: move semantics
// ============================================================================

TEST(XImage, move_ctor_transfers_ownership)
{
    XImage a(nullptr, 32, 32, au::cv::kXFormatGrayU8);
    void* origData = a.data[0];
    XImage b(std::move(a));
    EXPECT_TRUE(b.isValid());
    EXPECT_EQ(b.data[0], origData);
    // After move, source should not destroy data
}

TEST(XImage, move_assign_transfers_ownership)
{
    XImage a(nullptr, 64, 64, au::cv::kXFormatRGBAU8);
    void* origData = a.data[0];
    XImage b;
    b = std::move(a);
    EXPECT_TRUE(b.isValid());
    EXPECT_EQ(b.data[0], origData);
}

// ============================================================================
// XImage: isFormat / isFormatIn member functions
// ============================================================================

TEST(XImage, member_isFormat)
{
    XImage img(nullptr, 32, 32, au::cv::kXFormatRGBU8);
    EXPECT_TRUE(img.isFormat(au::cv::kXFormatRGBU8));
    EXPECT_FALSE(img.isFormat(au::cv::kXFormatGrayU8));
    EXPECT_TRUE(img.isFormat(XImageFormat::kXFormatRGBU8));
}

TEST(XImage, member_isFormatIn_int_list)
{
    XImage img(nullptr, 32, 32, au::cv::kXFormatRGBAU8);
    EXPECT_TRUE(img.isFormatIn({au::cv::kXFormatRGBU8, au::cv::kXFormatRGBAU8}));
    EXPECT_FALSE(img.isFormatIn({au::cv::kXFormatGrayU8, au::cv::kXFormatRGBU8}));
}

TEST(XImage, member_isFormatIn_enum_list)
{
    XImage img(nullptr, 32, 32, XImageFormat::kXFormatGrayU8);
    EXPECT_TRUE(img.isFormatIn({XImageFormat::kXFormatGrayU8, XImageFormat::kXFormatRGBU8}));
    EXPECT_FALSE(img.isFormatIn({XImageFormat::kXFormatRGBU8, XImageFormat::kXFormatRGBAU8}));
}

// ============================================================================
// XImage: member info
// ============================================================================

TEST(XImage, member_info_returns_non_empty)
{
    XImage img(nullptr, 16, 32, au::cv::kXFormatGrayU8);
    std::string s = img.info();
    EXPECT_FALSE(s.empty());
    EXPECT_NE(s.find("16x32"), std::string::npos);
}

// ============================================================================
// XImage: self-assignment safety
// ============================================================================

TEST(XImage, self_assign_is_safe)
{
    XImage img(nullptr, 32, 32, au::cv::kXFormatGrayU8);
    img = img;  // self-assignment
    EXPECT_TRUE(img.isValid());
}

#endif  // ENABLE_TEST_XIMAGE
