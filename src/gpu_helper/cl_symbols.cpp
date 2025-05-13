#include "cl_symbols.h"

// platform
// ==============================
cl_int CL_API_CALL clGetPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms) {
    return gpu::CLSymbols::get().proxy_clGetPlatformIDs(num_entries, platforms, num_platforms);
}

cl_int CL_API_CALL clGetPlatformInfo(cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
    return gpu::CLSymbols::get().proxy_clGetPlatformInfo(platform, param_name, param_value_size, param_value, param_value_size_ret);
}

// device
// ==============================
cl_int CL_API_CALL clRetainDevice(cl_device_id device) {
    return gpu::CLSymbols::get().proxy_clRetainDevice(device);
}

cl_int CL_API_CALL clReleaseDevice(cl_device_id device) {
    return gpu::CLSymbols::get().proxy_clReleaseDevice(device);
}

cl_int CL_API_CALL clGetDeviceIDs(cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices) {
    return gpu::CLSymbols::get().proxy_clGetDeviceIDs(platform, device_type, num_entries, devices, num_devices);
}

cl_int CL_API_CALL clGetDeviceInfo(cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
    return gpu::CLSymbols::get().proxy_clGetDeviceInfo(device, param_name, param_value_size, param_value, param_value_size_ret);
}

// context
// ==============================
cl_int CL_API_CALL clRetainContext(cl_context context) {
    return gpu::CLSymbols::get().proxy_clRetainContext(context);
}

cl_int CL_API_CALL clReleaseContext(cl_context context) {
    return gpu::CLSymbols::get().proxy_clReleaseContext(context);
}

cl_context CL_API_CALL clCreateContextFromType(const cl_context_properties *properties, cl_device_type device_type, void (CL_CALLBACK *pfn_notify)(const char *errinfo, const void *private_info, size_t cb, void *user_data), void *user_data, cl_int *errcode_ret) {
    return gpu::CLSymbols::get().proxy_clCreateContextFromType(properties, device_type, pfn_notify, user_data, errcode_ret);
}

cl_int CL_API_CALL clGetContextInfo(cl_context context, cl_context_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
    return gpu::CLSymbols::get().proxy_clGetContextInfo(context, param_name, param_value_size, param_value, param_value_size_ret);
}

// program
// ==============================
cl_int CL_API_CALL clReleaseProgram(cl_program program) {
    return gpu::CLSymbols::get().proxy_clReleaseProgram(program);
}

cl_program CL_API_CALL clCreateProgramWithSource(cl_context context, cl_uint count, const char **strings, const size_t *lengths, cl_int *errcode_ret) {
    return gpu::CLSymbols::get().proxy_clCreateProgramWithSource(context, count, strings, lengths, errcode_ret);
}

cl_int CL_API_CALL clBuildProgram(cl_program program, cl_uint num_devices, const cl_device_id *device_list, const char *options, void (CL_CALLBACK *pfn_notify)( cl_program program, void *user_data), void *user_data) {
    return gpu::CLSymbols::get().proxy_clBuildProgram(program, num_devices, device_list, options, pfn_notify, user_data);
}

cl_int CL_API_CALL clGetProgramInfo(cl_program program, cl_program_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
    return gpu::CLSymbols::get().proxy_clGetProgramInfo(program, param_name, param_value_size, param_value, param_value_size_ret);
}

cl_int CL_API_CALL clGetProgramBuildInfo(cl_program program, cl_device_id device, cl_program_build_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) {
    return gpu::CLSymbols::get().proxy_clGetProgramBuildInfo(program, device, param_name, param_value_size, param_value, param_value_size_ret);
}

// command queue
// ==============================
cl_int CL_API_CALL clRetainCommandQueue(cl_command_queue command_queue) {
    return gpu::CLSymbols::get().proxy_clRetainCommandQueue(command_queue);
}

cl_int CL_API_CALL clReleaseCommandQueue(cl_command_queue command_queue) {
    return gpu::CLSymbols::get().proxy_clReleaseCommandQueue(command_queue);
}

cl_command_queue CL_API_CALL clCreateCommandQueueWithProperties(cl_context context, cl_device_id device, const cl_queue_properties *properties, cl_int *errcode_ret) {
    return gpu::CLSymbols::get().proxy_clCreateCommandQueueWithProperties(context, device, properties, errcode_ret);
}

cl_int CL_API_CALL clEnqueueNDRangeKernel(cl_command_queue command_queue, cl_kernel kernel, cl_uint work_dim, const size_t *global_work_offset, const size_t *global_work_size, const size_t *local_work_size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) {
    return gpu::CLSymbols::get().proxy_clEnqueueNDRangeKernel(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event);
}

// mem object
// ==============================
cl_int CL_API_CALL clRetainMemObject(cl_mem memobj) {
    return gpu::CLSymbols::get().proxy_clRetainMemObject(memobj);
}

cl_int CL_API_CALL clReleaseMemObject(cl_mem memobj) {
    return gpu::CLSymbols::get().proxy_clReleaseMemObject(memobj);
}

cl_mem CL_API_CALL clCreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void *host_ptr, cl_int *errcode_ret) {
    return gpu::CLSymbols::get().proxy_clCreateBuffer(context, flags, size, host_ptr, errcode_ret);
}

void* CL_API_CALL clSVMAlloc(cl_context context, cl_svm_mem_flags flags, size_t size, cl_uint alignment) {
    return gpu::CLSymbols::get().proxy_clSVMAlloc(context, flags, size, alignment);
}

void CL_API_CALL clSVMFree(cl_context context, void *svm_pointer) {
    return gpu::CLSymbols::get().proxy_clSVMFree(context, svm_pointer);
}

cl_int CL_API_CALL clEnqueueSVMMap(cl_command_queue command_queue, cl_bool blocking_map, cl_map_flags map_flags, void *svm_ptr, size_t size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) {
    return gpu::CLSymbols::get().proxy_clEnqueueSVMMap(command_queue, blocking_map, map_flags, svm_ptr, size, num_events_in_wait_list, event_wait_list, event);
}

cl_mem CL_API_CALL clCreatePipe(cl_context context, cl_mem_flags flags, cl_uint pipe_packet_size, cl_uint pipe_max_packets, const cl_pipe_properties * properties, cl_int *errcode_ret) {
    return gpu::CLSymbols::get().proxy_clCreatePipe(context, flags, pipe_packet_size, pipe_max_packets, properties, errcode_ret);
}

void* CL_API_CALL clEnqueueMapBuffer(cl_command_queue command_queue, cl_mem buffer, cl_bool blocking_map, cl_map_flags map_flags, size_t offset, size_t size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event, cl_int *errcode_ret) {
    return gpu::CLSymbols::get().proxy_clEnqueueMapBuffer(command_queue, buffer, blocking_map, map_flags, offset, size, num_events_in_wait_list, event_wait_list, event, errcode_ret);
}

cl_int CL_API_CALL clEnqueueUnmapMemObject(cl_command_queue command_queue, cl_mem memobj, void *mapped_ptr, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event) {
    return gpu::CLSymbols::get().proxy_clEnqueueUnmapMemObject(command_queue, memobj, mapped_ptr, num_events_in_wait_list, event_wait_list, event);
}

// kernel
// ==============================
cl_int CL_API_CALL clSetKernelArgSVMPointer(cl_kernel kernel, cl_uint arg_index, const void *arg_value) {
    return gpu::CLSymbols::get().proxy_clSetKernelArgSVMPointer(kernel, arg_index, arg_value);
}

cl_int CL_API_CALL clReleaseKernel(cl_kernel kernel) {
    return gpu::CLSymbols::get().proxy_clReleaseKernel(kernel);
}

cl_int CL_API_CALL clSetKernelArg(cl_kernel kernel, cl_uint arg_index, size_t arg_size, const void *arg_value) {
    return gpu::CLSymbols::get().proxy_clSetKernelArg(kernel, arg_index, arg_size, arg_value);
}

cl_kernel CL_API_CALL clCreateKernel(cl_program program, const char *kernel_name, cl_int *errcode_ret) {
    return gpu::CLSymbols::get().proxy_clCreateKernel(program, kernel_name, errcode_ret);
}

cl_int CL_API_CALL clSetKernelExecInfo(cl_kernel kernel, cl_kernel_exec_info param_name, size_t param_value_size, const void *param_value) {
    return gpu::CLSymbols::get().proxy_clSetKernelExecInfo(kernel, param_name, param_value_size, param_value);
}

// event
// ==============================
cl_int CL_API_CALL clReleaseEvent(cl_event event) {
    return gpu::CLSymbols::get().proxy_clReleaseEvent(event);
}

cl_int CL_API_CALL clWaitForEvents(cl_uint num_events, const cl_event *event_list) {
    return gpu::CLSymbols::get().proxy_clWaitForEvents(num_events, event_list);
}