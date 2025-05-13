#ifndef __CL_SYMBOLS_H__
#define __CL_SYMBOLS_H__

#include <vector>
#include <string>
#include <memory>

#include "log/logger.h"
#include "ext/xdll_parser.h"

#ifndef CL_TARGET_OPENCL_VERSION
#define CL_TARGET_OPENCL_VERSION 200
#endif
#ifndef CL_HPP_TARGET_OPENCL_VERSION
#define CL_HPP_TARGET_OPENCL_VERSION 200
#endif
#ifndef CL_HPP_MINIMUM_OPENCL_VERSION
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#endif
#include "CL/opencl.hpp"


namespace gpu {

class CLSymbols {
public:
    static CLSymbols& get() {
        static CLSymbols instance;
        return instance;
    }
    ~CLSymbols() = default;

    // platform
    // ==============================
    cl_int proxy_clGetPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms) {
        return mParser->call<decltype(clGetPlatformIDs)>("clGetPlatformIDs", num_entries, platforms, num_platforms);
    }

    cl_int proxy_clGetPlatformInfo(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
        return mParser->call<decltype(clGetPlatformInfo)>("clGetPlatformInfo", platform, param_name, param_value_size, param_value, param_value_size_ret);
    }

    // device
    // ==============================
    cl_int proxy_clRetainDevice(cl_device_id device) {
        return mParser->call<decltype(clRetainDevice)>("clRetainDevice", device);
    }

    cl_int proxy_clReleaseDevice(cl_device_id device) {
        return mParser->call<decltype(clReleaseDevice)>("clReleaseDevice", device);
    }

    cl_int proxy_clGetDeviceIDs(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices) {
        return mParser->call<decltype(clGetDeviceIDs)>("clGetDeviceIDs", platform, device_type, num_entries, devices, num_devices);
    }

    cl_int proxy_clGetDeviceInfo(cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
        return mParser->call<decltype(clGetDeviceInfo)>("clGetDeviceInfo", device, param_name, param_value_size, param_value, param_value_size_ret);
    }

    // context
    // ==============================
    cl_int proxy_clRetainContext(cl_context context) {
        return mParser->call<decltype(clRetainContext)>("clRetainContext", context);
    }

    cl_int proxy_clReleaseContext(cl_context context) {
        return mParser->call<decltype(clReleaseContext)>("clReleaseContext", context);
    }

    cl_context proxy_clCreateContextFromType(const cl_context_properties *properties, cl_device_type device_type, void (CL_CALLBACK *pfn_notify)(const char *errinfo, const void *private_info, size_t cb, void *user_data), void *user_data, cl_int *errcode_ret) {
        return mParser->call<decltype(clCreateContextFromType)>("clCreateContextFromType", properties, device_type, pfn_notify, user_data, errcode_ret);
    }

    cl_int proxy_clGetContextInfo(cl_context context, cl_context_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
        return mParser->call<decltype(clGetContextInfo)>("clGetContextInfo", context, param_name, param_value_size, param_value, param_value_size_ret);
    }

    // program
    // ==============================
    cl_int proxy_clReleaseProgram(cl_program program) {
        return mParser->call<decltype(clReleaseProgram)>("clReleaseProgram", program);
    }

    cl_program proxy_clCreateProgramWithSource(cl_context context, cl_uint count, const char **strings, const size_t *lengths, cl_int *errcode_ret) {
        return mParser->call<decltype(clCreateProgramWithSource)>("clCreateProgramWithSource", context, count, strings, lengths, errcode_ret);
    }

    cl_int proxy_clBuildProgram(cl_program program, cl_uint num_devices, const cl_device_id *device_list, const char *options, void (CL_CALLBACK *pfn_notify)(cl_program program, void *user_data), void *user_data) {
        return mParser->call<decltype(clBuildProgram)>("clBuildProgram", program, num_devices, device_list, options, pfn_notify, user_data);
    }

    cl_int proxy_clGetProgramInfo(cl_program program, cl_program_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
        return mParser->call<decltype(clGetProgramInfo)>("clGetProgramInfo", program, param_name, param_value_size, param_value, param_value_size_ret);
    }

    cl_int proxy_clGetProgramBuildInfo(cl_program program, cl_device_id device, cl_program_build_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
        return mParser->call<decltype(clGetProgramBuildInfo)>("clGetProgramBuildInfo", program, device, param_name, param_value_size, param_value, param_value_size_ret);
    }

    // command queue
    // ==============================
    cl_int proxy_clRetainCommandQueue(cl_command_queue command_queue) {
        return mParser->call<decltype(clRetainCommandQueue)>("clRetainCommandQueue", command_queue);
    }

    cl_int proxy_clReleaseCommandQueue(cl_command_queue command_queue) {
        return mParser->call<decltype(clReleaseCommandQueue)>("clReleaseCommandQueue", command_queue);
    }

    cl_command_queue proxy_clCreateCommandQueueWithProperties(cl_context context, cl_device_id device, const cl_queue_properties *properties, cl_int *errcode_ret) {
        return mParser->call<decltype(clCreateCommandQueueWithProperties)>("clCreateCommandQueueWithProperties", context, device, properties, errcode_ret);
    }

    cl_int proxy_clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) {
        return mParser->call<decltype(clEnqueueNDRangeKernel)>("clEnqueueNDRangeKernel", command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
    }

    // mem object
    // ==============================
    cl_int proxy_clRetainMemObject(cl_mem memobj) {
        return mParser->call<decltype(clRetainMemObject)>("clRetainMemObject", memobj);
    }

    cl_int proxy_clReleaseMemObject(cl_mem memobj) {
        return mParser->call<decltype(clReleaseMemObject)>("clReleaseMemObject", memobj);
    }

    cl_mem proxy_clCreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret) {
        return mParser->call<decltype(clCreateBuffer)>("clCreateBuffer", context, flags, size, host_ptr, errcode_ret);
    }

    void* proxy_clSVMAlloc(cl_context context, cl_svm_mem_flags flags, size_t size, cl_uint alignment) {
        return mParser->call<decltype(clSVMAlloc)>("clSVMAlloc", context, flags, size, alignment);
    }

    void proxy_clSVMFree(cl_context context, void *svm_pointer) {
        return mParser->call<decltype(clSVMFree)>("clSVMFree", context, svm_pointer);
    }

    cl_int proxy_clEnqueueSVMMap(cl_command_queue command_queue, cl_bool blocking_map, cl_map_flags map_flags, void *svm_ptr, size_t size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) {
        return mParser->call<decltype(clEnqueueSVMMap)>("clEnqueueSVMMap", command_queue, blocking_map, map_flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, event);
    }

    cl_mem proxy_clCreatePipe(cl_context context, cl_mem_flags flags, cl_uint pipe_packet_size, cl_uint pipe_max_packets, const cl_pipe_properties * properties, cl_int *errcode_ret) {
        return mParser->call<decltype(clCreatePipe)>("clCreatePipe", context, flags, pipe_packet_size, pipe_max_packets, properties, errcode_ret);
    }

    void* proxy_clEnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags, size_t offset, size_t size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event, cl_int *errcode_ret) {
        return mParser->call<decltype(clEnqueueMapBuffer)>("clEnqueueMapBuffer", command_queue, buffer, blocking_map, map_flags, offset, size, num_events_in_wait_list, event_wait_list, event, errcode_ret);
    }

    cl_int proxy_clEnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj, void *mapped_ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) {
        return mParser->call<decltype(clEnqueueUnmapMemObject)>("clEnqueueUnmapMemObject", command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
    }

    // kernel
    // ==============================
    cl_int proxy_clSetKernelArgSVMPointer(cl_kernel kernel, cl_uint arg_index, const void *arg_value) {
        return mParser->call<decltype(clSetKernelArgSVMPointer)>("clSetKernelArgSVMPointer", kernel, arg_index, arg_value);
    }

    cl_int proxy_clReleaseKernel(cl_kernel kernel) {
        return mParser->call<decltype(clReleaseKernel)>("clReleaseKernel", kernel);
    }

    cl_int proxy_clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value) {
        return mParser->call<decltype(clSetKernelArg)>("clSetKernelArg", kernel, arg_index, arg_size, arg_value);
    }

    cl_kernel proxy_clCreateKernel(cl_program program, const char *kernel_name, cl_int *errcode_ret) {
        return mParser->call<decltype(clCreateKernel)>("clCreateKernel", program, kernel_name, errcode_ret);
    }

    cl_int proxy_clSetKernelExecInfo(cl_kernel kernel, cl_kernel_exec_info param_name, size_t param_value_size, const void *param_value) {
        return mParser->call<decltype(clSetKernelExecInfo)>("clSetKernelExecInfo", kernel, param_name, param_value_size, param_value);
    }

    // event
    // ==============================
    cl_int proxy_clReleaseEvent(cl_event event) {
        return mParser->call<decltype(clReleaseEvent)>("clReleaseEvent", event);
    }

    cl_int proxy_clWaitForEvents(cl_uint num_events, const cl_event *event_list) {
        return mParser->call<decltype(clWaitForEvents)>("clWaitForEvents", num_events, event_list);
    }

private:
    CLSymbols() {
        mParser.reset(new XDLLParser);
        int retLoadCLLib = mParser->load(mListLibPath);
        ASSERTER(retLoadCLLib == NO_ERROR);
    }

private:
    std::unique_ptr<XDLLParser> mParser;

    std::vector<std::string> mListLibPath = {
        "/vendor/lib64/libOpenCL.so",

        // __aarch64__
        "/system/vendor/lib64/libOpenCL.so",
        "/system/lib64/libOpenCL.so",

        // __aarch32__
        "/system/vendor/lib/libOpenCL.so",
        "/system/lib/libOpenCL.so",

        // windows default
        "C:/Windows/System32/opencl.dll",
        
        // NVIDIA CUDA
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.6/bin/OpenCL.dll",
        "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v11.4/bin/OpenCL.dll",
        
        // AMD APP SDK
        "C:/Program Files (x86)/AMD APP/bin/x86_64/OpenCL.dll",
        "C:/Program Files (x86)/AMD APP/bin/x86/OpenCL.dll",
        
        // Intel SDK
        "C:/Program Files (x86)/Intel/OpenCL SDK/6.3/bin/x64/OpenCL.dll",
        "C:/Program Files (x86)/Intel/OpenCL SDK/6.3/bin/x86/OpenCL.dll"
    };
};

} // namespace gpu

#endif // __CL_SYMBOLS_H__