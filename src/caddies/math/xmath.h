#ifndef __XMATH_H__
#define __XMATH_H__

#include <algorithm>
#include <cmath>

namespace math {

#define PI      3.14159265358979323846
#define EPSILON 0.000001f

template <typename T>
T minOf(T val0, T val1) { return std::min(val0, val1); }

template <typename T>
T maxOf(T val0, T val1) { return std::max(val0, val1); }

template <typename T>
T minOf(T val0, T val1, T val2) { std::min(val0, std::min(val1, val2)); }

template <typename T>
T maxOf(T val0, T val1, T val2) { std::max(val0, std::max(val1, val2)); }

double powd(const double& base, const double& exponent) { return std::pow(base, exponent); }
float powf(const float& base, const float& exponent) { return std::powf(base, exponent); }

template <typename T> // [min, max]
bool isInRangeCC(T val, T min, T max) { return val >= min && val <= max; }

template <typename T> // (min, max)
bool isInRangeOO(T val, T min, T max) { return val > min && val < max; }

template <typename T> // (min, max]
bool isInRangeOC(T val, T min, T max) { return val > min && val <= max; }

template <typename T> // [min, max)
bool isInRangeCO(T val, T min, T max) { return val >= min && val < max; }

template <typename T>
inline T clamp2uint8(T val) { return std::max((T)0, std::min(val, (T)255)); }

template <typename T>
inline T clamp2range(T val, T min, T max) { return std::max(min, std::min(val, max)); }

inline bool isAlignedTo2(int num) { return (num & 1) == 0; }
inline bool isAlignedTo4(int num) { return (num & 3) == 0; }
inline bool isAlignedTo8(int num) { return (num & 7) == 0; }

inline int ceilTo2(int num) { return (num + 1) & ~1; }
inline int ceilTo4(int num) { return (num + 3) & ~3; }
inline int ceilTo8(int num) { return (num + 7) & ~7; }

inline int floorTo2(int num) { return num & ~1; }
inline int floorTo4(int num) { return num & ~3; }
inline int floorTo8(int num) { return num & ~7; }

} // namespace math

#endif // __XMATH_H__