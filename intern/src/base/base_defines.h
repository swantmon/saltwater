
#pragma once

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