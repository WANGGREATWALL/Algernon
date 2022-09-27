#include "myUtils/myUtils.h"
#include <fstream>

#if defined(_WIN64) || defined(WIN32)
#include <windows.h>
#include <time.h>
double timer()
{
	LARGE_INTEGER nFreq = { 0 };
	LARGE_INTEGER nCount = { 0 };
	QueryPerformanceFrequency(&nFreq);
	QueryPerformanceCounter(&nCount);
	return (nCount.QuadPart * 1000.0 / nFreq.QuadPart); //ms
}
#else
#include <sys/time.h>
double timer()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0); //ms
}
#endif

int write_data_to_file(std::string filePath, const char* pDstData, size_t fileSize)
{
	std::fstream file(filePath, std::ios::binary | std::ios::out);
	if (!file.is_open()) {
		printf("[OpenCL Error]: Fail to open %s [%s:%s:%d]\n", filePath.c_str(), __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	file.seekg(0);

	file.write(pDstData, fileSize);
	file.close();
	return 0;
}