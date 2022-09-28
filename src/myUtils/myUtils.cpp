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

std::string read_data_from_file(std::string filePath, size_t fileSize)
{
	std::string data;
	std::fstream file(filePath, std::ios::binary | std::ios::in | std::ios::ate);
	if (!file.is_open()) {
		printf("Fail to open %s [%s:%s:%d]\n", filePath.c_str(), __FILE__, __FUNCTION__, __LINE__);
		return data;
	}

	if (fileSize == 0)
	{
		fileSize = file.tellg();
	}
	file.seekg(0);
	data.resize(fileSize);

	file.read(&data[0], fileSize);
	file.close();

	return data;
}

int write_data_to_file(std::string filePath, const char* pData, size_t fileSize)
{
	std::fstream file(filePath, std::ios::binary | std::ios::out);
	if (!file.is_open()) {
		printf("[OpenCL Error]: Fail to open %s [%s:%s:%d]\n", filePath.c_str(), __FILE__, __FUNCTION__, __LINE__);
		return -1;
	}

	file.seekg(0);

	file.write(pData, fileSize);
	file.close();
	return 0;
}