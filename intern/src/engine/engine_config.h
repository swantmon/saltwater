
#pragma once

// -----------------------------------------------------------------------------
// Debugging Settings
// -----------------------------------------------------------------------------

// Debugging mode:
// 0: Off
// 1: On
#ifdef NDEBUG
#define APP_DEBUG_MODE 0
#else
#define APP_DEBUG_MODE 1
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
