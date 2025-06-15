#include <set>

#include "log/logger.h"
#include "cl_wrapper.h"
#include "perf/performance.h"
#include "file/xfile.h"

namespace gpu {

std::string clErrorInfo(int err)
{
    switch (err) {
        case CL_SUCCESS                                  : return "CL_SUCCESS";
        case CL_DEVICE_NOT_FOUND                         : return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE                     : return "CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE                   : return "CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE            : return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES                         : return "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY                       : return "CL_OUT_OF_HOST_MEMORY";
        case CL_PROFILING_INFO_NOT_AVAILABLE             : return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case CL_MEM_COPY_OVERLAP                         : return "CL_MEM_COPY_OVERLAP";
        case CL_IMAGE_FORMAT_MISMATCH                    : return "CL_IMAGE_FORMAT_MISMATCH";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED               : return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case CL_BUILD_PROGRAM_FAILURE                    : return "CL_BUILD_PROGRAM_FAILURE";
        case CL_MAP_FAILURE                              : return "CL_MAP_FAILURE";
        case CL_MISALIGNED_SUB_BUFFER_OFFSET             : return "CL_MISALIGNED_SUB_BUFFER_OFFSET ";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case CL_COMPILE_PROGRAM_FAILURE                  : return "CL_COMPILE_PROGRAM_FAILURE";
        case CL_LINKER_NOT_AVAILABLE                     : return "CL_LINKER_NOT_AVAILABLE";
        case CL_LINK_PROGRAM_FAILURE                     : return "CL_LINK_PROGRAM_FAILURE";
        case CL_DEVICE_PARTITION_FAILED                  : return "CL_DEVICE_PARTITION_FAILED";
        case CL_KERNEL_ARG_INFO_NOT_AVAILABLE            : return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
        case CL_INVALID_VALUE                            : return "CL_INVALID_VALUE";
        case CL_INVALID_DEVICE_TYPE                      : return "CL_INVALID_DEVICE_TYPE";
        case CL_INVALID_PLATFORM                         : return "CL_INVALID_PLATFORM";
        case CL_INVALID_DEVICE                           : return "CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT                          : return "CL_INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES                 : return "CL_INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE                    : return "CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR                         : return "CL_INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT                       : return "CL_INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR          : return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE                       : return "CL_INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER                          : return "CL_INVALID_SAMPLER";
        case CL_INVALID_BINARY                           : return "CL_INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS                    : return "CL_INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM                          : return "CL_INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE               : return "CL_INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME                      : return "CL_INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION                : return "CL_INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL                           : return "CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX                        : return "CL_INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE                        : return "CL_INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE                         : return "CL_INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS                      : return "CL_INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION                   : return "CL_INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE                  : return "CL_INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE                   : return "CL_INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET                    : return "CL_INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST                  : return "CL_INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT                            : return "CL_INVALID_EVENT";
        case CL_INVALID_OPERATION                        : return "CL_INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT                        : return "CL_INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE                      : return "CL_INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL                        : return "CL_INVALID_MIP_LEVEL";
        case CL_INVALID_GLOBAL_WORK_SIZE                 : return "CL_INVALID_GLOBAL_WORK_SIZE";
        case CL_INVALID_PROPERTY                         : return "CL_INVALID_PROPERTY";
        case CL_INVALID_IMAGE_DESCRIPTOR                 : return "CL_INVALID_IMAGE_DESCRIPTOR";
        case CL_INVALID_COMPILER_OPTIONS                 : return "CL_INVALID_COMPILER_OPTIONS";
        case CL_INVALID_LINKER_OPTIONS                   : return "CL_INVALID_LINKER_OPTIONS";
        case CL_INVALID_DEVICE_PARTITION_COUNT           : return "CL_INVALID_DEVICE_PARTITION_COUNT";
        case CL_INVALID_PIPE_SIZE                        : return "CL_INVALID_PIPE_SIZE";
        case CL_INVALID_DEVICE_QUEUE                     : return "CL_INVALID_DEVICE_QUEUE";
    }
    return "CL_UNKNOWN_ERROR";
}

std::map<cl_channel_order, std::string> mapChannelOrder = {
    {CL_R,          "CL_R"},
    {CL_A,          "CL_A"},
    {CL_RG,         "CL_RG"},
    {CL_RA,         "CL_RA"},
    {CL_RGB,        "CL_RGB"},
    {CL_RGBA,       "CL_RGBA"},
    {CL_BGRA,       "CL_BGRA"},
    {CL_ARGB,       "CL_ARGB"},
    {CL_INTENSITY,  "CL_INTENSITY"},
    {CL_LUMINANCE,  "CL_LUMINANCE"}
};

std::map<cl_channel_type, std::string> mapChannelDataType = {
    {CL_SNORM_INT8,         "CL_SNORM_INT8"},
    {CL_SNORM_INT16,        "CL_SNORM_INT16"},
    {CL_UNORM_INT8,         "CL_UNORM_INT8"},
    {CL_UNORM_INT16,        "CL_UNORM_INT16"},
    {CL_UNORM_SHORT_565,    "CL_UNORM_SHORT_565"},
    {CL_UNORM_SHORT_555,    "CL_UNORM_SHORT_555"},
    {CL_UNORM_INT_101010,   "CL_UNORM_INT_101010"},
    {CL_SIGNED_INT8,        "CL_SIGNED_INT8"},
    {CL_SIGNED_INT16,       "CL_SIGNED_INT16"},
    {CL_SIGNED_INT32,       "CL_SIGNED_INT32"},
    {CL_UNSIGNED_INT8,      "CL_UNSIGNED_INT8"},
    {CL_UNSIGNED_INT16,     "CL_UNSIGNED_INT16"},
    {CL_UNSIGNED_INT32,     "CL_UNSIGNED_INT32"},
    {CL_HALF_FLOAT,         "CL_HALF_FLOAT"},
    {CL_FLOAT,              "CL_FLOAT"}
};

int getChannelBitWidth(const cl_channel_type& typeChannel)
{
    switch (typeChannel) {
        case CL_SNORM_INT8:
        case CL_UNORM_INT8:
        case CL_SIGNED_INT8:
        case CL_UNSIGNED_INT8:
            return 8;
        case CL_SNORM_INT16:
        case CL_UNORM_INT16:
        case CL_SIGNED_INT16:
        case CL_UNSIGNED_INT16:
        case CL_HALF_FLOAT:
            return 16;
        case CL_SIGNED_INT32:
        case CL_UNSIGNED_INT32:
        case CL_FLOAT:
            return 32;
        default:
            return -1;
    }
}

int convertCLImageFormat2NTIFormat(const cl_image_format& fmtCL, int& fmtCV)
{
    int bit = getChannelBitWidth(fmtCL.image_channel_data_type);
    ASSERTER_WITH_RET(bit > 0, ERROR_INVALID_PARAMETER);

    if (fmtCL.image_channel_order == CL_R) {
        switch (bit) {
            case 8:  fmtCV = cv::kXFormatGrayU8; break;
            case 16: fmtCV = cv::kXFormatGrayU16; break;
            case 32: fmtCV = cv::kXFormatGrayU32; break;
            default: return ERROR_UNSUPPORTED_TYPE;
        }
    }
    else if (fmtCL.image_channel_order == CL_RG) {
        switch (bit) {
            case 8:  fmtCV = cv::kXFormatUV; break;
            default: return ERROR_UNSUPPORTED_TYPE;
        }
    }
    else if (fmtCL.image_channel_order == CL_RGB) {
        switch (bit) {
            case 8:  fmtCV = cv::kXFormatRGBU8; break;
            default: return ERROR_UNSUPPORTED_TYPE;
        }
    }
    else if (fmtCL.image_channel_order == CL_RGBA || fmtCL.image_channel_order == CL_BGRA) {
        switch (bit) {
            case 8:  fmtCV = cv::kXFormatRGBAU8; break;
            default: return ERROR_UNSUPPORTED_TYPE;
        }
    } else {
        fmtCV = 0;
        return ERROR_UNSUPPORTED_TYPE;
    }
    
    return NO_ERROR;
}

size_t getImageWidth(const cl::Image& image)
{
    int ret = CL_SUCCESS;
    cl::size_type width = image.getImageInfo<CL_IMAGE_WIDTH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, 0, "clError: %s", clErrorInfo(ret).c_str());

    return width;
}

size_t getImageHeight(const cl::Image& image)
{
    int ret = CL_SUCCESS;
    cl::size_type height = image.getImageInfo<CL_IMAGE_HEIGHT>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, 0, "clError: %s", clErrorInfo(ret).c_str());

    return height;
}

size_t getImagePitch(const cl::Image& image)
{
    cl_int ret = CL_SUCCESS;
    cl::size_type pitch = image.getImageInfo<CL_IMAGE_ROW_PITCH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, 0, "clError: %s", clErrorInfo(ret).c_str());

    return pitch;
}

size_t getImageDepth(const cl::Image& image)
{
    cl_int ret = CL_SUCCESS;
    cl::size_type depth = image.getImageInfo<CL_IMAGE_DEPTH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, 0, "clError: %s", clErrorInfo(ret).c_str());

    return depth;
}

bool isValid(const cl::Image& image)
{
    return getImageWidth(image) > 0 && getImageHeight(image) > 0;
}

bool isFormat(const cl::Image& image, const cl_image_format& format)
{
    int ret = CL_SUCCESS;
    cl_image_format fmt = image.getImageInfo<CL_IMAGE_FORMAT>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, false, "clError: %s", clErrorInfo(ret).c_str());

    return fmt.image_channel_order == format.image_channel_order && fmt.image_channel_data_type == format.image_channel_data_type;
}

bool isSize(const cl::Image& image, size_t width, size_t height, size_t depth)
{
    return getImageWidth(image) == width && getImageHeight(image) == height && getImageDepth(image) == depth;
}

bool isSameSize(const cl::Image& image0, const cl::Image& image1)
{
    return getImageWidth(image0) == getImageWidth(image1) 
        && getImageHeight(image0) == getImageHeight(image1) 
        && getImageDepth(image0) == getImageDepth(image1);
}

bool isSameFormat(const cl::Image& image0, const cl::Image& image1)
{
    int ret = CL_SUCCESS;
    cl_image_format fmt0 = image0.getImageInfo<CL_IMAGE_FORMAT>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, false, "clError: %s", clErrorInfo(ret).c_str());

    cl_image_format fmt1 = image1.getImageInfo<CL_IMAGE_FORMAT>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, false, "clError: %s", clErrorInfo(ret).c_str());

    return fmt0.image_channel_order == fmt1.image_channel_order && fmt0.image_channel_data_type == fmt1.image_channel_data_type;
}

bool isSameSizeAndFormat(const cl::Image& image0, const cl::Image& image1)
{
    return isSameSize(image0, image1) && isSameFormat(image0, image1);
}

std::string info(const cl::Image& image)
{
    char str[256];

    int ret = CL_SUCCESS;
    cl_image_format fmt = image.getImageInfo<CL_IMAGE_FORMAT>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, "unknown", "clError: %s", clErrorInfo(ret).c_str());

    snprintf(str, 256, "[%zux%zux%zu], fmt:{%s, %s}", getImageWidth(image), getImageHeight(image), getImageDepth(image),
        mapChannelOrder[fmt.image_channel_order].c_str(), mapChannelDataType[fmt.image_channel_data_type].c_str());
    
    return std::string(str);
}

CLWrapper::CLWrapper(std::string folderBinary, std::string nameBinary)
{
    mFolderBinary = file::XFilenameMaker::getFolder(folderBinary + "/");
    mNameBinaryWithoutFormat = file::XFilenameMaker::eliminatePathAndFormat(nameBinary);
}

CLWrapper::~CLWrapper()
{
    perf::TracerScoped trace("CLWrapper::~CLWrapper");
    clearAndSyncEvents();
}

int CLWrapper::init(bool enableProfiling)
{
    perf::TracerScoped trace("CLWrapper::init");

    ASSERTER_WITH_RET(!mFolderBinary.empty(), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(file::isDirectory(mFolderBinary), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(!mNameBinaryWithoutFormat.empty(), ERROR_INVALID_PARAMETER);
    
    int retGetPlatform = getPlatform();
    ASSERTER_WITH_INFO(retGetPlatform == CL_SUCCESS, retGetPlatform, "clError: %s", clErrorInfo(retGetPlatform).c_str());
    
    int retGetDevice = getDevice();
    ASSERTER_WITH_INFO(retGetDevice == CL_SUCCESS, retGetDevice, "clError: %s", clErrorInfo(retGetDevice).c_str());

    int retCreateContext = createContext();
    ASSERTER_WITH_INFO(retCreateContext == CL_SUCCESS, retCreateContext, "clError: %s", clErrorInfo(retCreateContext).c_str());

    int retCreateCommandQueue = createCommandQueue((mEnableProfiling = enableProfiling) ? CL_QUEUE_PROFILING_ENABLE : 0);
    ASSERTER_WITH_INFO(retCreateCommandQueue == CL_SUCCESS, retCreateCommandQueue, "clError: %s", clErrorInfo(retCreateCommandQueue).c_str());

    return NO_ERROR;
}

int CLWrapper::build(const std::string& kernelString, const std::string optionsCompile)
{
    perf::TracerScoped trace("CLWrapper::build");
    
    ASSERTER_WITH_RET(!kernelString.empty(), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(!optionsCompile.empty(), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(!mFolderBinary.empty(), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(file::isDirectory(mFolderBinary), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(!mNameBinaryWithoutFormat.empty(), ERROR_INVALID_PARAMETER);
    
    mStringKernel = kernelString;

    bool isBinAvailable = false;
    if (loadCheckKey(mKeyLoaded) == NO_ERROR) {
        checkLoadKey(mKeyLoaded, isBinAvailable);
    }

    if (isBinAvailable) {
        int retBuildProgram = buildProgram(optionsCompile);
        ASSERTER_WITH_RET(retBuildProgram == CL_SUCCESS, retBuildProgram);
    } else {
        int retCreateProgram = createProgramWithKernelString(mStringKernel);
        ASSERTER_WITH_RET(retCreateProgram == CL_SUCCESS, retCreateProgram);

        int retBuildProgram = buildProgram(optionsCompile);
        ASSERTER_WITH_RET(retBuildProgram == CL_SUCCESS, retBuildProgram);

        int retSaveBinAndKey = storeBinAndKey();
        ASSERTER_WITH_RET(retSaveBinAndKey == NO_ERROR, retSaveBinAndKey);
    }

    return NO_ERROR;
}

int CLWrapper::createKernels()
{
    perf::TracerScoped trace("CLWrapper::createKernels");

    std::vector<cl::Kernel> kernels;
    int retCreateKernels = mProgram.createKernels(&kernels);
    ASSERTER_WITH_INFO(retCreateKernels == CL_SUCCESS, retCreateKernels, "clError: %s", clErrorInfo(retCreateKernels).c_str());

    mKernels.clear();
    for (auto i = kernels.cbegin(); i != kernels.cend(); ++i) {
        int ret = CL_SUCCESS;
        std::string name = i->getInfo<CL_KERNEL_FUNCTION_NAME>(&ret);
        ASSERTER_WITH_RET(ret == CL_SUCCESS, ret);

        mKernels[name] = *i;
    }

    return NO_ERROR;
}

int CLWrapper::createBuffer(cl::Buffer& dst, const cv::Image& image, cl_mem_flags flags)
{
    ASSERTER_WITH_RET(cv::isValid(image), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(cv::isFormatIn(image, {
        cv::kXFormatGrayU8,
        cv::kXFormatGrayU16,
        cv::kXFormatGrayU32,
        cv::kXFormatRGBU8,
        cv::kXFormatBGRU8,
        cv::kXFormatRGBAU8,
        cv::kXFormatBGRAU8}), ERROR_UNSUPPORTED_TYPE);

    int ret = CL_SUCCESS;
    dst = cl::Buffer(mContext, flags, image.height * image.stride[0], image.data[0], &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::createBuffer(cl::Buffer& dst, void* data, size_t sizeInByte, cl_mem_flags flags)
{
    ASSERTER_WITH_RET(data != nullptr, ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    dst = cl::Buffer(mContext, flags, sizeInByte, data, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::createImage2D(cl::Image2D& dst, const cv::Image& image, cl::ImageFormat format, cl_mem_flags flags)
{
    ASSERTER_WITH_RET(cv::isValid(image), ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    dst = cl::Image2D(mContext, flags, format, image.width, image.height, image.stride[0], image.data[0], &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::createImage2D(cl::Image2D& dst, void* data, int width, int height, cl::ImageFormat format, cl_mem_flags flags)
{
    ASSERTER_WITH_RET(data != nullptr, ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(width > 0 && height > 0, ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    dst = cl::Image2D(mContext, flags, format, (cl::size_type)width, (cl::size_type)height, 0, data, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::createImage2D(cl::Image2D& dst, int width, int height, cl::ImageFormat format, cl_mem_flags flags)
{
    ASSERTER_WITH_RET(width > 0 && height > 0, ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    dst = cl::Image2D(mContext, flags, format, (cl::size_type)width, (cl::size_type)height, 0, nullptr, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::createImage3D(cl::Image3D& dst, const cv::Image& image, cl::ImageFormat format, cl_mem_flags flags)
{
    ASSERTER_WITH_RET(cv::isValid(image), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(cv::isFormatIn(image, {
        cv::kXFormatGrayU8,
        cv::kXFormatGrayU16,
        cv::kXFormatGrayU32,
        cv::kXFormatRGBU8,
        cv::kXFormatBGRU8,
        cv::kXFormatRGBAU8,
        cv::kXFormatBGRAU8}), ERROR_UNSUPPORTED_TYPE);
    ASSERTER_WITH_RET(image.width * image.width == image.height, ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    dst = cl::Image3D(mContext, flags, format, image.width, image.width, image.width, 0, 0, image.data[0], &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::createImage3D(cl::Image3D& dst, void* data, int width, int height, int depth, cl::ImageFormat format, cl_mem_flags flags)
{
    ASSERTER_WITH_RET(data != nullptr, ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    dst = cl::Image3D(mContext, flags, format, width, height, depth, 0, 0, data, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::createImage3D(cl::Image3D& dst, int width, int height, int depth, cl::ImageFormat format, cl_mem_flags flags)
{
    int ret = CL_SUCCESS;
    dst = cl::Image3D(mContext, flags, format, width, height, depth, 0, 0, nullptr, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::copyImage2D(cl::Image2D& dst, const cl::Image2D& src)
{
    ASSERTER_WITH_RET(isValid(src), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(isSameSizeAndFormat(src, dst), ERROR_INVALID_PARAMETER);

    auto width = getImageWidth(src);
    auto height = getImageHeight(src);

    cl::Event event;
    int retCopyImage = mCommandQueue.enqueueCopyImage(src, dst, {0, 0, 0}, {0, 0, 0}, {width, height, 1}, 0, &event);
    ASSERTER_WITH_INFO(retCopyImage == CL_SUCCESS, retCopyImage, "clError: %s", clErrorInfo(retCopyImage).c_str());

    mEvents.emplace_back("copyImage2D", event);

    return NO_ERROR;
}

int CLWrapper::readImage2D(cv::Image& dst, const cl::Image2D& image, bool block)
{
    perf::TracerScoped trace("CLWrapper::readImage2D");

    ASSERTER_WITH_RET(cv::isValid(dst), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(isSize(image, dst.width, dst.height), ERROR_INVALID_PARAMETER);

    cl::Event event;
    int retReadImage = mCommandQueue.enqueueReadImage(image, block, {0, 0, 0}, {(cl::size_type)dst.width, (cl::size_type)dst.height, 1}, dst.stride[0], 0, dst.data[0], 0, &event);
    ASSERTER_WITH_INFO(retReadImage == CL_SUCCESS, retReadImage, "clError: %s", clErrorInfo(retReadImage).c_str());

    mEvents.emplace_back("readImage2D", event);

    return NO_ERROR;
}

int CLWrapper::readImage2D(cv::Image& dst, const cl::Image2D& plane0, const cl::Image2D& plane1)
{
    perf::TracerScoped trace("CLWrapper::readImage2D");

    ASSERTER_WITH_RET(cv::isValid(dst), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(cv::isFormatIn(dst, {cv::kXFormatNV12, cv::kXFormatNV21}), ERROR_INVALID_PARAMETER);

    cl_int ret = CL_SUCCESS;

    trace.sub("query");
    ASSERTER_WITH_RET(isSize(plane0, dst.width, dst.height), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(isSize(plane1, dst.width / 2, dst.height / 2), ERROR_INVALID_PARAMETER);

    size_t pitch0 = getImagePitch(plane0);
    size_t pitch1 = getImagePitch(plane1);

    trace.sub("read");
    cl::Event event;
    int retReadImage0 = mCommandQueue.enqueueReadImage(plane0, false, {0, 0, 0}, {(cl::size_type)dst.width, (cl::size_type)dst.height, 1}, pitch0, 0, dst.data[0], 0, &event);
    ASSERTER_WITH_INFO(retReadImage0 == CL_SUCCESS, retReadImage0, "clError: %s", clErrorInfo(retReadImage0).c_str());
    mEvents.emplace_back("readImage2DP0", event);

    int retReadImage1 = mCommandQueue.enqueueReadImage(plane1, false, {0, 0, 0}, {(cl::size_type)dst.width >> 1, (cl::size_type)dst.height >> 1, 1}, pitch1, 0, dst.data[1], 0, &event);
    ASSERTER_WITH_INFO(retReadImage1 == CL_SUCCESS, retReadImage1, "clError: %s", clErrorInfo(retReadImage1).c_str());
    mEvents.emplace_back("readImage2DP1", event);

    return NO_ERROR;
}

int CLWrapper::readImage2D(void* dst, const cl::Image2D& image, int width, int height, int pitch, bool block)
{
    perf::TracerScoped trace("CLWrapper::readImage2D");

    ASSERTER_WITH_RET(dst != nullptr, ERROR_INVALID_PARAMETER);

    cl::Event event;
    int retReadImage = mCommandQueue.enqueueReadImage(image, block, {0, 0, 0}, {(cl::size_type)width, (cl::size_type)height, 1}, (cl::size_type)pitch, 0, dst, 0, &event);
    ASSERTER_WITH_INFO(retReadImage == CL_SUCCESS, retReadImage, "clError: %s", clErrorInfo(retReadImage).c_str());
    mEvents.emplace_back("readImage2D", event);

    return NO_ERROR;
}

int CLWrapper::mapImage2D(cv::Image& dst, const cl::Image2D& image)
{
    perf::TracerScoped trace("CLWrapper::mapImage2D");

    ASSERTER_WITH_RET(cv::isValid(dst), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(cv::isFormatIn(dst, {
        cv::kXFormatGrayU8,
        cv::kXFormatRGBU8,
        cv::kXFormatBGRU8,
        cv::kXFormatRGBAU8,
        cv::kXFormatBGRAU8}), ERROR_INVALID_PARAMETER);
    
    cl_int ret = CL_SUCCESS;

    trace.sub("query");
    cl::size_type pitch = getImagePitch(image);
    cl::size_type width = getImageWidth(image);
    cl::size_type height = getImageHeight(image);
    
    ASSERTER_WITH_RET(dst.height == height, ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(dst.width <= pitch, ERROR_INVALID_PARAMETER);

    trace.sub("map");
    cl::Event event;
    void* data = mCommandQueue.enqueueMapImage(image, CL_TRUE, CL_MAP_READ, {0, 0, 0}, {width, height, 1}, &pitch, 0, 0, &event, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());
    mEvents.emplace_back("mapImage", event);

    trace.sub("copy");
    for (int i = 0; i < dst.height; ++i) {
        auto dataSrc = (uint8_t*)data + pitch * i;
        auto dataDst = dst.data[0] + i * dst.stride[0];
        memcpy(dataDst, dataSrc, dst.stride[0]);
    }

    trace.sub("unmap");
    auto retUnmapImage = mCommandQueue.enqueueUnmapMemObject(image, data, 0, &event);
    ASSERTER_WITH_INFO(retUnmapImage == CL_SUCCESS, retUnmapImage, "clError: %s", clErrorInfo(retUnmapImage).c_str());
    mEvents.emplace_back("unmapImage", event);

    return NO_ERROR;
}

int CLWrapper::mapImage2D(cv::Image& dst, const cl::Image2D& plane0, const cl::Image2D& plane1)
{
    perf::TracerScoped trace("CLWrapper::mapImage2D");

    ASSERTER_WITH_RET(cv::isValid(dst), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(cv::isFormatIn(dst, {cv::kXFormatNV12, cv::kXFormatNV21}), ERROR_INVALID_PARAMETER);
    
    cl_int ret = CL_SUCCESS;

    trace.sub("query");
    cl::size_type pitch0 = getImagePitch(plane0);
    cl::size_type pitch1 = getImagePitch(plane1);
    cl::size_type width0 = getImageWidth(plane0);
    cl::size_type width1 = getImageWidth(plane1);
    cl::size_type height0 = getImageHeight(plane0);
    cl::size_type height1 = getImageHeight(plane1);

    ASSERTER_WITH_RET(pitch0 == pitch1, ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(width0 == width1 * 2, ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(height0 == height1 * 2, ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(dst.width == width0 && dst.height == height0 && dst.stride[0] == pitch0, ERROR_INVALID_PARAMETER);

    trace.sub("map");
    cl::Event event;
    void* data0 = mCommandQueue.enqueueMapImage(plane0, false, CL_MAP_READ | CL_MAP_WRITE, {0, 0, 0}, {width0, height0, 1}, &pitch0, 0, 0, &event, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());
    mEvents.emplace_back("mapImageP0", event);

    void* data1 = mCommandQueue.enqueueMapImage(plane1, true, CL_MAP_READ | CL_MAP_WRITE, {0, 0, 0}, {width1, height1, 1}, &pitch1, 0, 0, &event, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());
    mEvents.emplace_back("mapImageP1", event);

    trace.sub("copy");
    memcpy(dst.data[0], data0, dst.stride[0] * dst.height);
    memcpy(dst.data[1], data1, dst.stride[1] * dst.height >> 1);

    trace.sub("unmap");
    auto retUnmapImage0 = mCommandQueue.enqueueUnmapMemObject(plane0, data0, 0, &event);
    ASSERTER_WITH_INFO(retUnmapImage0 == CL_SUCCESS, retUnmapImage0, "clError: %s", clErrorInfo(retUnmapImage0).c_str());
    mEvents.emplace_back("unmapImageP0", event);

    auto retUnmapImage1 = mCommandQueue.enqueueUnmapMemObject(plane1, data1, 0, &event);
    ASSERTER_WITH_INFO(retUnmapImage1 == CL_SUCCESS, retUnmapImage1, "clError: %s", clErrorInfo(retUnmapImage1).c_str());
    mEvents.emplace_back("unmapImageP1", event);

    return NO_ERROR;
}

void* CLWrapper::mallocSVM(size_t sizeInByte, size_t align, cl_mem_flags flags)
{
    int ret = CL_SUCCESS;
    cl_device_svm_capabilities validSVM = mDevices.front().getInfo<CL_DEVICE_SVM_CAPABILITIES>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS && (validSVM & CL_DEVICE_SVM_FINE_GRAIN_BUFFER), nullptr, "clError: %s", clErrorInfo(ret).c_str());

    void* data = clSVMAlloc(mContext(), flags, sizeInByte, align);
    ASSERTER_WITH_RET(data != nullptr, nullptr);

    return data;
}

int CLWrapper::freeSVM(void* data)
{
    ASSERTER_WITH_RET(data != nullptr, ERROR_INVALID_PARAMETER);
    clSVMFree(mContext(), data);
    data = nullptr;
    return NO_ERROR;
}

CLWrapper& CLWrapper::setNDRange(cl::NDRange global, cl::NDRange local, cl::NDRange offset)
{
    mGlobal = global;
    mLocal = local;
    mOffset = offset;
    return *this;
}

int CLWrapper::flush()
{
    int retFlush = mCommandQueue.flush();
    ASSERTER_WITH_INFO(retFlush == CL_SUCCESS, retFlush, "clError: %s", clErrorInfo(retFlush).c_str());
    return NO_ERROR;
}

int CLWrapper::finish()
{
    int retFinish = mCommandQueue.finish();
    ASSERTER_WITH_INFO(retFinish == CL_SUCCESS, retFinish, "clError: %s", clErrorInfo(retFinish).c_str());

    return clearAndSyncEvents();
}

int CLWrapper::wait(const cl::Event& event)
{
    int retEventWait = event.wait();
    ASSERTER_WITH_INFO(retEventWait == CL_SUCCESS, retEventWait, "clError: %s", clErrorInfo(retEventWait).c_str());
    return NO_ERROR;
}

int CLWrapper::barrier()
{
    int retBarrier = mCommandQueue.enqueueBarrierWithWaitList();
    ASSERTER_WITH_INFO(retBarrier == CL_SUCCESS, retBarrier, "clError: %s", clErrorInfo(retBarrier).c_str());
    return NO_ERROR;
}

int CLWrapper::clearAndSyncEvents()
{
    if (mEnableProfiling) {
        int ret = CL_SUCCESS;
        for (const auto& e : mEvents) {
            double enqueue = e.event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>(&ret) / 1000000.0; // in ms
            ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

            double submit = e.event.getProfilingInfo<CL_PROFILING_COMMAND_SUBMIT>(&ret) / 1000000.0;
            ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

            double start = e.event.getProfilingInfo<CL_PROFILING_COMMAND_START>(&ret) / 1000000.0;
            ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

            double end = e.event.getProfilingInfo<CL_PROFILING_COMMAND_END>(&ret) / 1000000.0;
            ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

            LOGGER_I("cl profiling => (enqueue)%.3f, (:submit)%.3f, (:start)%.3f, (:end)%.3f ms => [%s]\n", enqueue, (submit - enqueue), (start - submit), (end - start), e.name.c_str());
        }
    }
    mEvents.clear();

    return NO_ERROR;
}

int CLWrapper::querySupportedImageFormats(const cl_mem_object_type object, const cl_mem_flags flags) const
{
    std::vector<cl::ImageFormat> formats;
    int retGetSupportedFormats = mContext.getSupportedImageFormats(flags, CL_MEM_OBJECT_IMAGE2D, &formats);
    ASSERTER_WITH_INFO(retGetSupportedFormats == CL_SUCCESS, retGetSupportedFormats, "clError: %s", clErrorInfo(retGetSupportedFormats).c_str());

    std::set<std::string> listChannelOrder;
    std::set<std::string> listChannelDataType;

    for (const auto& format : formats) {
        auto orderIt = mapChannelOrder.find(format.image_channel_order);
        if (orderIt != mapChannelOrder.end()) {
            listChannelOrder.insert(orderIt->second);
        }

        auto dataTypeIt = mapChannelDataType.find(format.image_channel_data_type);
        if (dataTypeIt != mapChannelDataType.end()) {
            listChannelDataType.insert(dataTypeIt->second);
        }
    }

    LOGGER_I("OpenCL supported channel order:\n");
    for (const auto& order : listChannelOrder) {
        LOGGER_I("    %s\n", order.c_str());
    }

    LOGGER_I("OpenCL supported channel data type:\n");
    for (const auto& type : listChannelDataType) {
        LOGGER_I("    %s\n", type.c_str());
    }

    return NO_ERROR;
}

int CLWrapper::querySVMCapabilities() const
{
    int ret = CL_SUCCESS;
    cl_device_svm_capabilities capsSVM = mDevices.front().getInfo<CL_DEVICE_SVM_CAPABILITIES>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    if (capsSVM & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER) {
        LOGGER_I("OpenCL supported SVM type: %s\n", "CL_DEVICE_SVM_COARSE_GRAIN_BUFFER");
    }
    if (capsSVM & CL_DEVICE_SVM_FINE_GRAIN_BUFFER) {
        LOGGER_I("OpenCL supported SVM type: %s\n", "CL_DEVICE_SVM_FINE_GRAIN_BUFFER");
    }
    if (capsSVM & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM) {
        LOGGER_I("OpenCL supported SVM type: %s\n", "CL_DEVICE_SVM_FINE_GRAIN_SYSTEM");
    }
    if (capsSVM & CL_DEVICE_SVM_ATOMICS) {
        LOGGER_I("OpenCL supported SVM type: %s\n", "CL_DEVICE_SVM_ATOMICS");
    }

    return NO_ERROR;
}

int CLWrapper::getPlatform()
{
    perf::TracerScoped trace("CLWrapper::getPlatform");
    return cl::Platform::get(&mPlatforms);
}

int CLWrapper::getDevice()
{
    perf::TracerScoped trace("CLWrapper::getDevice");

    ASSERTER_WITH_RET(!mPlatforms.empty(), ERROR_INVALID_PARAMETER);
    return mPlatforms.front().getDevices(CL_DEVICE_TYPE_GPU, &mDevices);
}

int CLWrapper::createContext(cl_context_properties* properties)
{
    perf::TracerScoped trace("CLWrapper::createContext");

    ASSERTER_WITH_RET(!mDevices.empty(), ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    mContext = cl::Context(mDevices, properties, nullptr, nullptr, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::createCommandQueue(cl_command_queue_properties properties)
{
    perf::TracerScoped trace("CLWrapper::createCommandQueue");

    ASSERTER_WITH_RET(!mDevices.empty(), ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    mCommandQueue = cl::CommandQueue(mContext, mDevices.front(), properties, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::createProgramWithKernelString(const std::string& kernel)
{
    perf::TracerScoped trace("CLWrapper::createProgramWithKernelString");

    int ret = CL_SUCCESS;
    mProgram = cl::Program(mContext, kernel, true, &ret);

    if (ret != CL_SUCCESS) {
        LOGGER_E("clError: %s\n", clErrorInfo(ret).c_str());

        int retGetProgramBuildLog = CL_SUCCESS;
        std::string log = mProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(mDevices.front(), &retGetProgramBuildLog);
        ASSERTER_WITH_INFO(retGetProgramBuildLog == CL_SUCCESS, retGetProgramBuildLog, "clError: %s", clErrorInfo(retGetProgramBuildLog).c_str());

        LOGGER_E("cl build log: %s\n", log.c_str());
    }

    return ret;
}

int CLWrapper::buildProgram(const std::string& option)
{
    perf::TracerScoped trace("CLWrapper::buildProgram");

    int retBuildProgram = mProgram.build(option.c_str());
    if (retBuildProgram != CL_SUCCESS) {
        LOGGER_E("clError: %s\n", clErrorInfo(retBuildProgram).c_str());

        int retGetProgramBuildLog = CL_SUCCESS;
        std::string log = mProgram.getBuildInfo<CL_PROGRAM_BUILD_LOG>(mDevices.front(), &retGetProgramBuildLog);
        ASSERTER_WITH_INFO(retGetProgramBuildLog == CL_SUCCESS, retGetProgramBuildLog, "clError: %s", clErrorInfo(retGetProgramBuildLog).c_str());

        LOGGER_E("cl build log: %s\n", log.c_str());
    }

    return retBuildProgram;
}

int CLWrapper::getKernel(cl::Kernel& kernel, const std::string& name)
{
    ASSERTER_WITH_INFO(mKernels.find(name) != mKernels.end(), ERROR_NOT_FOUND, "failed to find kernel[%s]!", name.c_str());
    kernel = mKernels[name];
    return NO_ERROR;
}

void CLWrapper::checkLoadKey(const std::string& key, bool& isBinAvailable)
{
    perf::TracerScoped trace("CLWrapper::checkLoadKey");

    isBinAvailable = false;

    if (key.size() == 60) {
        std::string keyDeviceLoaded(key.cbegin(), key.cbegin() + 20);
        std::string keyKernelLoaded(key.cbegin() + 20, key.cbegin() + 40);
        std::string keyBinaryLoaded(key.cbegin() + 40, key.cbegin() + 60);
        LOGGER_D("CLWrapper: key verification(1/5)\n");

        if (getDeviceKey(mKeyDevice) == NO_ERROR && mKeyDevice == keyDeviceLoaded) {
            LOGGER_D("CLWrapper: key verification(2/5)\n");

            if (getKernelKey(mKeyKernel) == NO_ERROR && mKeyKernel == keyKernelLoaded) {
                LOGGER_D("CLWrapper: key verification(3/5)\n");

                if (getBinaryKey(mKeyBinary) == NO_ERROR && mKeyBinary == keyBinaryLoaded) {
                    LOGGER_D("CLWrapper: key verification(4/5)\n");

                    std::vector<unsigned char> bin(mStringBinary.cbegin(), mStringBinary.cend()); // mStringBinary loaded in getBinaryKey()
                    cl::Program::Binaries bins = { bin };

                    int ret = CL_SUCCESS;
                    mProgram = cl::Program(mContext, mDevices, bins, NULL, &ret);
                    if (ret == CL_SUCCESS) {
                        isBinAvailable = true;
                        LOGGER_D("CLWrapper: key verification(5/5)!\n");
                    } else {
                        LOGGER_W("clError: %s\n", clErrorInfo(ret).c_str());
                    }
                }
            }
        }
    }
}

int CLWrapper::loadCheckKey(std::string& key)
{
    perf::TracerScoped trace("CLWrapper::loadCheckKey");

    ASSERTER_WITH_RET(!mFolderBinary.empty(), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(!mNameBinaryWithoutFormat.empty(), ERROR_INVALID_PARAMETER);

    std::string pathKey = mFolderBinary + mNameBinaryWithoutFormat + ".key";
    if (!file::exists(pathKey)) {
        key.clear();
        return ERROR_NOT_FOUND;
    }

    int retLoadBuffer = file::XFile::loadFileToBuffer(pathKey, key);
    ASSERTER_WITH_RET(retLoadBuffer == NO_ERROR, retLoadBuffer);

    return NO_ERROR;
}

int CLWrapper::storeBinAndKey()
{
    perf::TracerScoped trace("CLWrapper::storeBinAndKey");

    ASSERTER_WITH_RET(!mFolderBinary.empty(), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(!mNameBinaryWithoutFormat.empty(), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(!mStringKernel.empty(), ERROR_INVALID_PARAMETER);

    std::string pathBin = mFolderBinary + mNameBinaryWithoutFormat + ".bin";
    std::string pathKey = mFolderBinary + mNameBinaryWithoutFormat + ".key";

    trace.sub("bin");
    int retGetBinary = CL_SUCCESS;
    cl::Program::Binaries bins = mProgram.getInfo<CL_PROGRAM_BINARIES>(&retGetBinary);
    ASSERTER_WITH_INFO(retGetBinary == CL_SUCCESS && !bins.empty(), retGetBinary, "clError: %s", clErrorInfo(retGetBinary).c_str());

    std::string bin(bins.front().cbegin(), bins.front().cend());

    int retSaveBin = file::XFile::saveBufferToFile(bin, pathBin);
    ASSERTER_WITH_RET(retSaveBin == NO_ERROR, retSaveBin);

    trace.sub("key");
    int retGetDeviceKey = getDeviceKey(mKeyDevice);
    ASSERTER_WITH_RET(retGetDeviceKey == NO_ERROR, retGetDeviceKey);

    mKeyKernel = std::to_string(std::hash<std::string>{}(mStringKernel));
    mKeyBinary = std::to_string(std::hash<std::string>{}(bin));

    mKeyKernel.resize(20, 'X'); // ensure 20 characters
    mKeyBinary.resize(20, 'X');
    std::string key = mKeyDevice + mKeyKernel + mKeyBinary;

    int retSaveKey = file::XFile::saveBufferToFile(key, pathKey);
    ASSERTER_WITH_RET(retSaveKey == NO_ERROR, retSaveKey);

    return NO_ERROR;
}

int CLWrapper::getDeviceKey(std::string& key)
{
    std::string nameDevice, vendorDevice, versionDevice;

    int retGetName = getDeviceName(nameDevice);
    ASSERTER_WITH_RET(retGetName == NO_ERROR, retGetName);

    int retGetVendor = getDeviceVendor(vendorDevice);
    ASSERTER_WITH_RET(retGetVendor == NO_ERROR, retGetVendor);

    int retGetVersion = getDeviceVersion(versionDevice);
    ASSERTER_WITH_RET(retGetVersion == NO_ERROR, retGetVersion);

    std::string strDevice = nameDevice + vendorDevice + versionDevice;
    key = std::to_string(std::hash<std::string>{}(strDevice));
    key.resize(20, 'X'); // ensure 20 characters

    return NO_ERROR;
}

int CLWrapper::getKernelKey(std::string& key)
{
    ASSERTER_WITH_RET(!mStringKernel.empty(), ERROR_INVALID_PARAMETER);

    key = std::to_string(std::hash<std::string>{}(mStringKernel));
    key.resize(20, 'X'); // ensure 20 characters

    return NO_ERROR;
}

int CLWrapper::getBinaryKey(std::string& key)
{
    ASSERTER_WITH_RET(!mFolderBinary.empty(), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(!mNameBinaryWithoutFormat.empty(), ERROR_INVALID_PARAMETER);

    std::string pathBinary = mFolderBinary + mNameBinaryWithoutFormat + ".bin";

    if (!file::exists(pathBinary)) {
        mStringBinary.clear();
        return ERROR_NOT_FOUND;
    }

    int retLoadBuffer = file::XFile::loadFileToBuffer(pathBinary, mStringBinary);
    ASSERTER_WITH_RET(retLoadBuffer == NO_ERROR, retLoadBuffer);
    ASSERTER_WITH_RET(!mStringBinary.empty(), ERROR_INVALID_DATA);

    key = std::to_string(std::hash<std::string>{}(mStringBinary));
    key.resize(20, 'X');

    return NO_ERROR;
}

int CLWrapper::getDeviceName(std::string& name)
{
    ASSERTER_WITH_RET(!mDevices.empty(), ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    name = mDevices.front().getInfo<CL_DEVICE_NAME>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::getDeviceVendor(std::string& vendor)
{
    ASSERTER_WITH_RET(!mDevices.empty(), ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    vendor = mDevices.front().getInfo<CL_DEVICE_VENDOR>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::getDeviceVersion(std::string& version)
{
    ASSERTER_WITH_RET(!mDevices.empty(), ERROR_INVALID_PARAMETER);

    int ret = CL_SUCCESS;
    version = mDevices.front().getInfo<CL_DEVICE_VERSION>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    return NO_ERROR;
}

int CLWrapper::getKernelName(const cl::Kernel kernel, std::string& name)
{
    name.clear();
    size_t length = 0;
    cl_int retGetKernelNameLength =clGetKernelInfo(kernel.get(), CL_KERNEL_FUNCTION_NAME, 0, nullptr, &length);
    ASSERTER_WITH_INFO(retGetKernelNameLength == CL_SUCCESS, retGetKernelNameLength, "clError: %s", clErrorInfo(retGetKernelNameLength).c_str());

    name = std::string(length, '0');
    cl_int retGetKernelName = clGetKernelInfo(kernel.get(), CL_KERNEL_FUNCTION_NAME, length, (char*)name.c_str(), nullptr);
    ASSERTER_WITH_INFO(retGetKernelName == CL_SUCCESS, retGetKernelName, "clError: %s", clErrorInfo(retGetKernelName).c_str());

    return NO_ERROR;
}

} // namespace gpu