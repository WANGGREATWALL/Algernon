#ifndef ALGERNON_LOG_XERROR_H_
#define ALGERNON_LOG_XERROR_H_

/**
 * @file xerror.h
 * @brief Unified error code definitions for the Algernon library.
 *
 * All modules should use these error codes as return values.
 * Success is always 0; all errors are positive integers.
 */

namespace algernon {

enum ErrorCode : int {
    kSuccess            = 0,
    kErrorUnknown       = 1,
    kErrorFileNotFound  = 2,
    kErrorOpenFailed    = 3,
    kErrorInvalidParam  = 4,
    kErrorInvalidHandle = 5,
    kErrorNotSupported  = 6,
    kErrorBadFormat     = 7,
    kErrorNoMemory      = 8,
    kErrorOutOfRange    = 9,
    kErrorTimeout       = 10,
    kErrorAlreadyExists = 11,
    kErrorNotReady      = 12,
};

} // namespace algernon

#endif // ALGERNON_LOG_XERROR_H_
