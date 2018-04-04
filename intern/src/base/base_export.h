
#pragma once

#ifdef _WINDLL
#ifdef BASE_EXPORT
#define BASE_API __declspec(dllexport)
#else
#define BASE_API __declspec(dllimport)
#endif
#else
#define BASE_API
#endif