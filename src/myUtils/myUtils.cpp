#include "myUtils/myUtils.h"

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