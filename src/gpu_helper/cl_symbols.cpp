#include "cl_symbols.h"

// platform
// ==============================
cl_int CL_API_CALL clGetPlatformIDs(cl_uint num_entries, cl_platform_id* platforms, cl_uint* num_platforms) {
    return gpu::CLSymbols::get().clGetPlatformIDs(num_entries, platforms, num_platforms);
}