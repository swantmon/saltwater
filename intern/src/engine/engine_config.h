
#pragma once

// -----------------------------------------------------------------------------
// Version
// -----------------------------------------------------------------------------
#define ENGINE_MAJOR_VERSION 0
#define ENGINE_MINOR_VERSION 5

// -----------------------------------------------------------------------------
// Debugging Settings
// -----------------------------------------------------------------------------

// Debugging mode:
#ifndef NDEBUG
#define ENGINE_DEBUG_MODE
#endif // NDEBUG


// -----------------------------------------------------------------------------
// Export
// -----------------------------------------------------------------------------
#ifdef _WINDLL
#ifdef ENGINE_EXPORT
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
#else
#define ENGINE_API
#endif
