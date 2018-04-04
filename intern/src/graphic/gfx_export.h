
#pragma once

#ifdef _WINDLL
#ifdef GFX_EXPORT
#define GFX_API __declspec(dllexport)
#else
#define GFX_API __declspec(dllimport)
#endif
#else
#define GFX_API
#endif