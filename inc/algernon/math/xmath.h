#ifndef ALGERNON_MATH_XMATH_H_
#define ALGERNON_MATH_XMATH_H_

/**
 * @file xmath.h
 * @brief Unified math utilities: constants, min/max, clamp, alignment, trig, power.
 *
 * Include this single header for all common math operations.
 *
 * @example
 *   using namespace algernon::math;
 *   auto v = clamp(val, 0.0f, 1.0f);
 *   auto a = ceilTo8(width);
 *   bool ok = isPowerOf2(256);
 */

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>

namespace algernon { namespace math {

// ============================================================================
// Constants
// ============================================================================

inline constexpr double kPi       = 3.14159265358979323846;
inline constexpr double kTwoPi    = 6.28318530717958647692;
inline constexpr double kHalfPi   = 1.57079632679489661923;
inline constexpr double kE        = 2.71828182845904523536;
inline constexpr float  kEpsilonF = 1e-6f;
inline constexpr double kEpsilonD = 1e-12;

// ============================================================================
// Min / Max (2 and 3 arguments)
// ============================================================================

template <typename T> constexpr T minOf(T a, T b) { return std::min(a, b); }
template <typename T> constexpr T maxOf(T a, T b) { return std::max(a, b); }
template <typename T> constexpr T minOf(T a, T b, T c) { return std::min(a, std::min(b, c)); }
template <typename T> constexpr T maxOf(T a, T b, T c) { return std::max(a, std::max(b, c)); }

// ============================================================================
// Clamp
// ============================================================================

template <typename T>
constexpr T clamp(T val, T lo, T hi) { return std::max(lo, std::min(val, hi)); }

template <typename T>
constexpr uint8_t clampToU8(T val) {
    return static_cast<uint8_t>(std::max(static_cast<T>(0), std::min(val, static_cast<T>(255))));
}

template <typename T>
constexpr uint16_t clampToU16(T val) {
    return static_cast<uint16_t>(std::max(static_cast<T>(0), std::min(val, static_cast<T>(65535))));
}

template <typename T>
constexpr T clampToRange(T val, T lo, T hi) { return clamp(val, lo, hi); }

// ============================================================================
// Alignment
// ============================================================================

constexpr bool isAlignedTo2(int v)  { return (v & 1) == 0; }
constexpr bool isAlignedTo4(int v)  { return (v & 3) == 0; }
constexpr bool isAlignedTo8(int v)  { return (v & 7) == 0; }
constexpr bool isAlignedTo16(int v) { return (v & 15) == 0; }

/**
 * @brief Check if value is aligned to N (N must be power of 2).
 * @example isAlignedToN(64, 16) => true
 */
constexpr bool isAlignedToN(int v, int n) { return (v & (n - 1)) == 0; }

constexpr int ceilTo2(int v)  { return (v + 1) & ~1; }
constexpr int ceilTo4(int v)  { return (v + 3) & ~3; }
constexpr int ceilTo8(int v)  { return (v + 7) & ~7; }
constexpr int ceilTo16(int v) { return (v + 15) & ~15; }

/**
 * @brief Ceil value to the nearest multiple of N (N must be power of 2).
 * @example ceilToN(13, 8) => 16
 */
constexpr int ceilToN(int v, int n) { return (v + n - 1) & ~(n - 1); }

constexpr int floorTo2(int v)  { return v & ~1; }
constexpr int floorTo4(int v)  { return v & ~3; }
constexpr int floorTo8(int v)  { return v & ~7; }
constexpr int floorTo16(int v) { return v & ~15; }

constexpr int floorToN(int v, int n) { return v & ~(n - 1); }

constexpr bool isPowerOf2(int v) { return v > 0 && (v & (v - 1)) == 0; }

// ============================================================================
// Range checks
// ============================================================================

/** @brief Check if val is in [lo, hi]. */
template <typename T> constexpr bool inRangeCC(T val, T lo, T hi) { return val >= lo && val <= hi; }
/** @brief Check if val is in (lo, hi). */
template <typename T> constexpr bool inRangeOO(T val, T lo, T hi) { return val > lo && val < hi; }
/** @brief Check if val is in (lo, hi]. */
template <typename T> constexpr bool inRangeOC(T val, T lo, T hi) { return val > lo && val <= hi; }
/** @brief Check if val is in [lo, hi). */
template <typename T> constexpr bool inRangeCO(T val, T lo, T hi) { return val >= lo && val < hi; }

// ============================================================================
// Absolute value
// ============================================================================

template <typename T>
constexpr T abs(T val) { return val < 0 ? -val : val; }

// ============================================================================
// Power / Sqrt (thin wrappers to avoid cmath namespace issues)
// ============================================================================

inline float  powF(float base, float exp)   { return std::pow(base, exp); }
inline double powD(double base, double exp)  { return std::pow(base, exp); }
inline float  sqrtF(float x)                 { return std::sqrt(x); }
inline double sqrtD(double x)                { return std::sqrt(x); }

// ============================================================================
// Trigonometry
// ============================================================================

inline float  sinF(float x)  { return std::sin(x); }
inline double sinD(double x) { return std::sin(x); }
inline float  cosF(float x)  { return std::cos(x); }
inline double cosD(double x) { return std::cos(x); }
inline float  tanF(float x)  { return std::tan(x); }
inline double tanD(double x) { return std::tan(x); }
inline float  asinF(float x) { return std::asin(x); }
inline double asinD(double x){ return std::asin(x); }
inline float  acosF(float x) { return std::acos(x); }
inline double acosD(double x){ return std::acos(x); }
inline float  atanF(float x) { return std::atan(x); }
inline double atanD(double x){ return std::atan(x); }
inline float  atan2F(float y, float x)  { return std::atan2(y, x); }
inline double atan2D(double y, double x){ return std::atan2(y, x); }

// ============================================================================
// Angle conversion
// ============================================================================

template <typename T> constexpr T deg2rad(T deg) { return deg * static_cast<T>(kPi / 180.0); }
template <typename T> constexpr T rad2deg(T rad) { return rad * static_cast<T>(180.0 / kPi); }

}} // namespace algernon::math

#endif // ALGERNON_MATH_XMATH_H_
