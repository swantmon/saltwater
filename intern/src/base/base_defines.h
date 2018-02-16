
#pragma once

// -----------------------------------------------------------------------------
// Platform
// -----------------------------------------------------------------------------
#if __ANDROID__
#define PLATFORM_ANDROID 1
#elif __APPLE__
#define PLATFORM_APPLE 1
#elif _WIN32
#define PLATFORM_WINDOWS 1
#else
#pragma message("ERROR: The platform/compiler is not supported.")
#endif

// -----------------------------------------------------------------------------
// Used variable macro
// -----------------------------------------------------------------------------
#define BASE_UNUSED(Argument)           ((void) & Argument)

// -----------------------------------------------------------------------------
// Macro concation
// -----------------------------------------------------------------------------
#define BASE_CONCAT(A, B)               BASE_CONCAT_IMPL(A, B)
#define BASE_CONCAT_IMPL(A, B)          BASE_CONCAT_IMPL2(A, B)
#define BASE_CONCAT_IMPL2(A, B)         A ## B

// -----------------------------------------------------------------------------
// NULL
// -----------------------------------------------------------------------------
#define BASE_NULL NULL

// -----------------------------------------------------------------------------
// Include configuration of user
// -----------------------------------------------------------------------------
#include "base/base_config.h"

// -----------------------------------------------------------------------------
// Configuration of namespace usage
// -----------------------------------------------------------------------------
#if (BASE_HAS_USER_NAMESPACE == BASE_TRUE)
#define CON                         BASE_USER_NAMESPACE
#define CORE                        BASE_USER_NAMESPACE
#define ENC                         BASE_USER_NAMESPACE
#define IMG                         BASE_USER_NAMESPACE
#define IO                          BASE_USER_NAMESPACE
#define MATH                        BASE_USER_NAMESPACE
#define MEM                         BASE_USER_NAMESPACE
#define PAT                         BASE_USER_NAMESPACE
#define SER                         BASE_USER_NAMESPACE
#define STR                         BASE_USER_NAMESPACE
#define UT                          BASE_USER_NAMESPACE
#else
#define CON                         Container
#define CORE                        Core
#define ENC                         Encryption
#define IMG                         Image
#define IO                          IO
#define MATH                        Math
#define MEM                         Memory
#define PAT                         Pattern
#define SER                         Serialization
#define STR                         String
#define UT                          Unittest
#endif // User defined or default namespace

// -----------------------------------------------------------------------------
// Deprecated
// -----------------------------------------------------------------------------
#ifdef PLATFORM_ANDROID
#define BASE_DEPRECATED(Function) Function __attribute__ ((deprecated))
#elif PLATFORM_WINDOWS
#define BASE_DEPRECATED(Function) __declspec(deprecated) Function
#else
#pragma message("INFO: Highlighting deprecated functions is not supported on this platform. Please implement function.")
#define BASE_DEPRECATED(Function) Function
#endif
