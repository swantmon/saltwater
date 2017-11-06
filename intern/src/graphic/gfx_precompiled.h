
#pragma once

#ifdef __ANDROID__
#include "GLES3/gl32.h"
#include "GLES3/gl3ext.h"
#else
#include "GL/glew.h"
#endif // __ANDROID__