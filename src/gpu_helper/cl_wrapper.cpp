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

CLWrapper::CLWrapper(std::string folderBinary, std::string nameBinary)
{
    mFolderBinary = file::XFilenameMaker::getFolder(folderBinary + "/");
    mNameBinaryWithoutFormat = file::XFilenameMaker::eliminatePathAndFormat(nameBinary);
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

int CLWrapper::readImage2D(cv::Image& dst, const cl::Image2D& image, bool block)
{
    perf::TracerScoped trace("CLWrapper::readImage2D");

    ASSERTER_WITH_RET(cv::isValid(dst), ERROR_INVALID_PARAMETER);

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
    cl::size_type pitch0 = plane0.getImageInfo<CL_IMAGE_ROW_PITCH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type pitch1 = plane1.getImageInfo<CL_IMAGE_ROW_PITCH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type width0 = plane0.getImageInfo<CL_IMAGE_WIDTH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type width1 = plane1.getImageInfo<CL_IMAGE_WIDTH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type height0 = plane0.getImageInfo<CL_IMAGE_HEIGHT>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type height1 = plane1.getImageInfo<CL_IMAGE_HEIGHT>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    ASSERTER_WITH_RET(pitch0 == pitch1, ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(width0 == width1 * 2, ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(height0 == height1 * 2, ERROR_INVALID_PARAMETER);

    trace.sub("read");
    cl::Event event;
    int retReadImage0 = mCommandQueue.enqueueReadImage(plane0, false, {0, 0, 0}, {(cl::size_type)width0, (cl::size_type)height0, 1}, pitch0, 0, dst.data[0], 0, &event);
    ASSERTER_WITH_INFO(retReadImage0 == CL_SUCCESS, retReadImage0, "clError: %s", clErrorInfo(retReadImage0).c_str());
    mEvents.emplace_back("readImage2DP0", event);

    int retReadImage1 = mCommandQueue.enqueueReadImage(plane1, false, {0, 0, 0}, {(cl::size_type)width1, (cl::size_type)height1, 1}, pitch1, 0, dst.data[1], 0, &event);
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

int CLWrapper::mapImage2D(cv::Image& dst, const cl::Image2D& image, bool block)
{
    perf::TracerScoped trace("CLWrapper::mapImage2D");

    ASSERTER_WITH_RET(cv::isValid(dst), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(cv::isFormatIn(dst, {
        cv::kXFormatGrayU8,
        cv::kXFormatGrayU16,
        cv::kXFormatRGBU8,
        cv::kXFormatBGRU8,
        cv::kXFormatRGBAU8,
        cv::kXFormatBGRAU8}), ERROR_INVALID_PARAMETER);
    
    cl_int ret = CL_SUCCESS;

    trace.sub("query");
    cl::size_type pitch = image.getImageInfo<CL_IMAGE_ROW_PITCH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type width = image.getImageInfo<CL_IMAGE_WIDTH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type height = image.getImageInfo<CL_IMAGE_HEIGHT>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    if (cv::isFormat(dst, cv::kXFormatGrayU8)) {
        ASSERTER_WITH_RET(width == pitch, ERROR_INVALID_PARAMETER);
    }
    else if (cv::isFormat(dst, cv::kXFormatGrayU16)) {
        ASSERTER_WITH_RET(width * 2 == pitch, ERROR_INVALID_PARAMETER);
    }
    else if (cv::isFormatIn(dst, {cv::kXFormatRGBU8, cv::kXFormatBGRU8})) {
        ASSERTER_WITH_RET(width * 3 == pitch, ERROR_INVALID_PARAMETER);
    }
    else if (cv::isFormatIn(dst, {cv::kXFormatRGBAU8, cv::kXFormatBGRAU8})) {
        ASSERTER_WITH_RET(width * 4 == pitch, ERROR_INVALID_PARAMETER);
    }
    else {
        ASSERTER_WITH_RET(false, ERROR_INVALID_PARAMETER);
    }

    trace.sub("map");
    cl::Event event;
    void* data = mCommandQueue.enqueueMapImage(image, block, CL_MAP_READ | CL_MAP_WRITE, {0, 0, 0}, {(cl::size_type)width, (cl::size_type)height, 1}, &pitch, 0, 0, &event, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());
    mEvents.emplace_back("mapImage", event);

    trace.sub("copy");
    memcpy(dst.data[0], data, pitch * height);

    trace.sub("unmap");
    int retUnmapImage = mCommandQueue.enqueueUnmapMemObject(image, data, 0, &event);
    ASSERTER_WITH_INFO(retUnmapImage == CL_SUCCESS, retUnmapImage, "clError: %s", clErrorInfo(retUnmapImage).c_str());
    mEvents.emplace_back("unmapImage", event);

    return NO_ERROR;
}

int CLWrapper::mapImage2D(cv::Image& dst, const cl::Image2D& plane0, const cl::Image2D& plane1, bool block)
{
    perf::TracerScoped trace("CLWrapper::mapImage2D");

    ASSERTER_WITH_RET(cv::isValid(dst), ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(cv::isFormatIn(dst, {cv::kXFormatNV12, cv::kXFormatNV21}), ERROR_INVALID_PARAMETER);
    
    int ret = CL_SUCCESS;

    trace.sub("query");
    cl::size_type pitch0 = plane0.getImageInfo<CL_IMAGE_ROW_PITCH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type pitch1 = plane1.getImageInfo<CL_IMAGE_ROW_PITCH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type width0 = plane0.getImageInfo<CL_IMAGE_WIDTH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type width1 = plane1.getImageInfo<CL_IMAGE_WIDTH>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type height0 = plane0.getImageInfo<CL_IMAGE_HEIGHT>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    cl::size_type height1 = plane1.getImageInfo<CL_IMAGE_HEIGHT>(&ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());

    ASSERTER_WITH_RET(pitch0 == pitch1, ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(width0 == width1 * 2, ERROR_INVALID_PARAMETER);
    ASSERTER_WITH_RET(height0 == height1 * 2, ERROR_INVALID_PARAMETER);

    trace.sub("map");
    cl::Event event;
    void* data0 = mCommandQueue.enqueueMapImage(plane0, block, CL_MAP_READ | CL_MAP_WRITE, {0, 0, 0}, {(cl::size_type)width0, (cl::size_type)height0, 1}, &pitch0, 0, 0, &event, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());
    mEvents.emplace_back("mapImageP0", event);

    void* data1 = mCommandQueue.enqueueMapImage(plane1, block, CL_MAP_READ | CL_MAP_WRITE, {0, 0, 0}, {(cl::size_type)width1, (cl::size_type)height1, 1}, &pitch1, 0, 0, &event, &ret);
    ASSERTER_WITH_INFO(ret == CL_SUCCESS, ret, "clError: %s", clErrorInfo(ret).c_str());
    mEvents.emplace_back("mapImageP1", event);

    trace.sub("copy");
    memcpy(dst.data[0], data0, pitch0 * height0);
    memcpy(dst.data[1], data1, pitch1 * height1);

    trace.sub("unmap");
    int retUnmapImage0 = mCommandQueue.enqueueUnmapMemObject(plane0, data0, 0, &event);
    ASSERTER_WITH_INFO(retUnmapImage0 == CL_SUCCESS, retUnmapImage0, "clError: %s", clErrorInfo(retUnmapImage0).c_str());
    mEvents.emplace_back("unmapImageP0", event);

    int retUnmapImage1 = mCommandQueue.enqueueUnmapMemObject(plane1, data1, 0, &event);
    ASSERTER_WITH_INFO(retUnmapImage1 == CL_SUCCESS, retUnmapImage1, "clError: %s", clErrorInfo(retUnmapImage1).c_str());
    mEvents.emplace_back("unmapImageP1", event);

    return NO_ERROR;
}

CLWrapper& CLWrapper::setNDRange(cl::NDRange global, cl::NDRange local, cl::NDRange offset)
{
    mGlobal = global;
    mLocal = local;
    mOffset = offset;
    return *this;
}

int CLWrapper::finish()
{
    int retFinish = mCommandQueue.finish();
    ASSERTER_WITH_INFO(retFinish == CL_SUCCESS, retFinish, "clError: %s", clErrorInfo(retFinish).c_str());

    // profiling event
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

        mEvents.clear();
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