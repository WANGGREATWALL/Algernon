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
#include "log/logger.h"


namespace gpu {

    class CLWrapper {
    public:
        CLWrapper(std::string folderBinary, std::string nameBinary);
        ~CLWrapper() = default;

        CLWrapper(const CLWrapper&) = delete;
        CLWrapper& operator=(const CLWrapper&) = delete;

        int init(bool enableProfiling=false);
        int build(const std::string& kernelString, const std::string optionCompile="-cl-std=CL2.0");
        int createKernels();

        int createImage2D(cl::Image2D& dst, const cv::XImage& image, cl::ImageFormat format={CL_R, CL_UNORM_INT8}, cl_mem_flags flags=CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
        int createImage2D(cl::Image2D& dst, void* data, int width, int height, cl::ImageFormat format={CL_RG, CL_UNORM_INT8}, cl_mem_flags=CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR);
        int createImage2D(cl::Image2D& dst, int width, int height, cl::ImageFormat format={CL_R, CL_UNORM_INT8}, cl_mem_flags flags=CL_MEM_READ_WRITE);

        int readImage2D(cv::XImage& dst, const cl::Image2D& image, bool block);
        int readImage2D(cv::XImage& dst, const cl::Image2D& plane0, const cl::Image2D& plane1);
        int readImage2D(void* dst, const cl::Image2D& image, int width, int height, int pitch, bool block);

        int mapImage2D(cv::XImage& dst, const cl::Image2D& image, bool block);
        int mapImage2D(cv::XImage& dst, const cl::Image2D& plane0, const cl::Image2D& plane1, bool block);

        CLWrapper& setNDRange(cl::NDRange global, cl::NDRange local={1,1}, cl::NDRange offset={0,0});

        template <typename.. Args>
        int enqueue(std::string nameKernel, Args.. args) {
            cl::Kernel kernel;
            int retGetKernel = getKernel(kernel, nameKernel);
            ASSERTER_WITH_RET(retGetKernel == VDKResultSuccess, retGetKernel);

            int idx = 0;
            std::vector<int> setargs{ [&] { return kernel.setArg(idx++, args); }()... };

            for (int i = 0; i < setargs.size(); ++i) {
                ASSERTER_WITH_INFO(setargs[i] == CL_SUCCESS, VDKResultEInvalidParam, "failed to setargs[%d]: %s!", i, clErrorInfo(setargs[i]).c_str());
            }

            cl::Event event;
            int retEnqueueNDRangeKernel = mCommandQueue.enqueueNDRangeKernel(kernel, mOffset, mGlobal, mLocal, nullptr, &event);
            ASSERTER_WITH_INFO(retEnqueueNDRangeKernel == CL_SUCCESS, retEnqueueNDRangeKernel, "clError: %s", clErrorInfo(retEnqueueNDRangeKernel).c_str());
            mEvents.emplace_back(nameKernel, event);

            return VDKResultSuccess;
        }

        int finish();
    
    private:
        struct CLEvent {
            CLEvent(const std::string& _name, const cl::Event& _event) : name(_name), event(_event) {}

            std::string name;
            cl::Event   event;
        };

        std::string clErrorInfo(int err);

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

        std::vector<cl::Plarform> mPlatforms;
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