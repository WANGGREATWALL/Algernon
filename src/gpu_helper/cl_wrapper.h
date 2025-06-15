/**
 * The OpenCL kernel code is written in a .cl file. To convert the kernel.cl file into a kernel.h file 
 * and obtain the kernel string for OpenCL compilation, the following command can be used in the shell:
 * 
 * pathCur=$(pwd)
 * cd ${pathCur}/../../src/common_utils/
 * xxd -i portrait_style.cl portrait_style_kernel.h
 * cd ${pathCur}/
 * 
 */

#ifndef __CL_WRAPPER_H__
#define __CL_WRAPPER_H__

#include <string>
#include <vector>
#include <map>

#include "cl_symbols.h"
#include "cv/ximage.h"


namespace gpu {

std::string clErrorInfo(int err);

/**
 * @return width if success, else 0
 */
size_t getImageWidth(const cl::Image& image);
size_t getImageHeight(const cl::Image& image);
size_t getImagePitch(const cl::Image& image);
size_t getImageDepth(const cl::Image& image);

bool isValid(const cl::Image& image);
bool isFormat(const cl::Image& image, const cl_image_format& format={CL_R, CL_UNORM_INT8});
bool isSize(const cl::Image& image, size_t width, size_t height, size_t depth=0);
bool isSameSize(const cl::Image& image0, const cl::Image& image1);
bool isSameFormat(const cl::Image& image0, const cl::Image& image1);
bool isSameSizeAndFormat(const cl::Image& image0, const cl::Image& image1);

std::string info(const cl::Image& image);


class CLWrapper {
public:
    CLWrapper(std::string folderBinary, std::string nameBinary);
    ~CLWrapper();

    CLWrapper(const CLWrapper&) = delete;
    CLWrapper& operator=(const CLWrapper&) = delete;

    int init(bool enableProfiling=false);
    int build(const std::string& kernelString, const std::string optionsCompile="-cl-std=CL2.0 -cl-fast-relaxed-math -cl-mad-enable -cl-no-signed-zeros -cl-unsafe-math-optimizations");
    int createKernels();

    int createBuffer(cl::Buffer& dst, const cv::Image& image, cl_mem_flags flags=CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
    int createBuffer(cl::Buffer& dst, void* data, size_t sizeInByte, cl_mem_flags flags=CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);

    int createImage2D(cl::Image2D& dst, const cv::Image& image, cl::ImageFormat format={CL_R, CL_UNORM_INT8}, cl_mem_flags flags=CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
    int createImage2D(cl::Image2D& dst, void* data, int width, int height, cl::ImageFormat format={CL_RG, CL_UNORM_INT8}, cl_mem_flags flags=CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
    int createImage2D(cl::Image2D& dst, int width, int height, cl::ImageFormat format={CL_R, CL_UNORM_INT8}, cl_mem_flags flags=CL_MEM_READ_WRITE);

    int createImage3D(cl::Image3D& dst, const cv::Image& image, cl::ImageFormat format={CL_R, CL_UNORM_INT8}, cl_mem_flags flags=CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
    int createImage3D(cl::Image3D& dst, void* data, int width, int height, int depth, cl::ImageFormat format={CL_RG, CL_UNORM_INT8}, cl_mem_flags flags=CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
    int createImage3D(cl::Image3D& dst, int width, int height, int depth, cl::ImageFormat format={CL_R, CL_UNORM_INT8}, cl_mem_flags flags=CL_MEM_READ_WRITE);
    
    int copyImage2D(cl::Image2D& dst, const cl::Image2D& src);

    int readImage2D(cv::Image& dst, const cl::Image2D& image, bool block);
    int readImage2D(cv::Image& dst, const cl::Image2D& plane0, const cl::Image2D& plane1);
    int readImage2D(void* dst, const cl::Image2D& image, int width, int height, int pitch, bool block);

    /**
     * @param dst the mapping output host image, could be 1/2 plane
     * @param image the cl mem object to be mapped
     */
    int mapImage2D(cv::Image& dst, const cl::Image2D& image);
    int mapImage2D(cv::Image& dst, const cl::Image2D& plane0, const cl::Image2D& plane1);

    void* mallocSVM(size_t sizeInByte, size_t align=64, cl_mem_flags flags=CL_MEM_READ_WRITE | CL_MEM_SVM_FINE_GRAIN_BUFFER);
    int freeSVM(void* data);

    CLWrapper& setNDRange(cl::NDRange global, cl::NDRange local={1,1}, cl::NDRange offset={0,0});

    template <typename... Args>
    int enqueue(std::string nameKernel, cl::Event* event=nullptr, Args... args) {
        cl::Kernel kernel;
        int retGetKernel = getKernel(kernel, nameKernel);
        ASSERTER_WITH_RET(retGetKernel == NO_ERROR, retGetKernel);

        int idx = 0;
        std::vector<int> setargs{ [&] { return kernel.setArg(idx++, args); }()... };

        for (int i = 0; i < setargs.size(); ++i) {
            ASSERTER_WITH_INFO(setargs[i] == CL_SUCCESS, ERROR_INVALID_PARAMETER, "failed to setargs[%d]: %s!", i, clErrorInfo(setargs[i]).c_str());
        }

        if (event) {
            int retEnqueueNDRangeKernel = mCommandQueue.enqueueNDRangeKernel(kernel, mOffset, mGlobal, mLocal, nullptr, event);
            ASSERTER_WITH_INFO(retEnqueueNDRangeKernel == CL_SUCCESS, retEnqueueNDRangeKernel, "clError: %s", clErrorInfo(retEnqueueNDRangeKernel).c_str());
            mEvents.emplace_back(nameKernel, *event);
        } else {
            cl::Event eventInner;
            int retEnqueueNDRangeKernel = mCommandQueue.enqueueNDRangeKernel(kernel, mOffset, mGlobal, mLocal, nullptr, &eventInner);
            ASSERTER_WITH_INFO(retEnqueueNDRangeKernel == CL_SUCCESS, retEnqueueNDRangeKernel, "clError: %s", clErrorInfo(retEnqueueNDRangeKernel).c_str());
            mEvents.emplace_back(nameKernel, eventInner);
        }
        
        return NO_ERROR;
    }

    /**
     * @brief
     * - flush(): batch submission of the above commands
     * - finish(): program end full sync
     * - wait(): wait for a specific event to complete
     * - barrier(): order barrier between commands
     * @note
     *           | BLOCK HOST | WAIT COMMAND TO COMPLETE
     * ----------|------------|-------------------------
     * flush()   | NO         | NO
     * finish()  | YES        | YES
     * wait()    | YES        | YES
     * barrier() | NO         | NO
     */
    int flush();
    int finish();
    int wait(const cl::Event& event);
    int barrier();

    int clearAndSyncEvents();

    int querySupportedImageFormats(const cl_mem_object_type object=CL_MEM_OBJECT_IMAGE2D, const cl_mem_flags flags=CL_MEM_READ_ONLY) const;
    int querySVMCapabilities() const;

private:
    struct CLEvent {
        CLEvent(const std::string& _name, const cl::Event& _event) : name(_name), event(_event) {}

        std::string name;
        cl::Event   event;
    };

    int getPlatform();
    int getDevice();
    int createContext(cl_context_properties* properties=nullptr);
    int createCommandQueue(cl_command_queue_properties queue_properties=0);
    int createProgramWithKernelString(const std::string& kernel);
    int buildProgram(const std::string& option);
    int getKernel(cl::Kernel& kernel, const std::string& name);

    // for binary check
    void checkLoadKey(const std::string& key, bool& isBinAvailable);

    int loadCheckKey(std::string& key);
    int storeBinAndKey();

    int getDeviceKey(std::string& key);
    int getKernelKey(std::string& key);
    int getBinaryKey(std::string& key);

    int getDeviceName(std::string& name);
    int getDeviceVendor(std::string& vendor);
    int getDeviceVersion(std::string& version);

    int getKernelName(const cl::Kernel kernel, std::string& name);

private:
    std::string mFolderBinary;
    std::string mNameBinaryWithoutFormat;

    bool mEnableProfiling;

    std::vector<cl::Platform> mPlatforms;
    std::vector<cl::Device> mDevices;
    cl::Context mContext;
    cl::CommandQueue mCommandQueue;
    cl::Program mProgram;
    std::map<std::string, cl::Kernel> mKernels;
    std::vector<CLEvent> mEvents;

    std::string mStringKernel;
    std::string mStringBinary;

    std::string mKeyLoaded;
    std::string mKeyDevice;
    std::string mKeyKernel;
    std::string mKeyBinary;

    cl::NDRange mOffset;
    cl::NDRange mGlobal;
    cl::NDRange mLocal;
};

} // namespace gpu

#endif // __CL_WRAPPER_H__