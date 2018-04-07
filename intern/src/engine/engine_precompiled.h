
#pragma once

// -----------------------------------------------------------------------------
// STL includes
// -----------------------------------------------------------------------------
#include <sstream> 
#include <fstream> 
#include <iostream> 
#include <stdarg.h> 
#include <assert.h> 
#include <exception> 
#include <memory.h> 
#include <new> 
#include <stdlib.h> 
#include <chrono> 
#include <algorithm> 
#include <random> 

#define _USE_MATH_DEFINES 
#define NOMINMAX
#include <cmath> 

// -----------------------------------------------------------------------------
// Base includes
// -----------------------------------------------------------------------------
#include "base/base_include_glm.h"
#include "base/base_defines.h"

// -----------------------------------------------------------------------------
// Core includes
// -----------------------------------------------------------------------------
#include "core/core_console.h"

// -----------------------------------------------------------------------------
// Platform specific includes
// -----------------------------------------------------------------------------
#ifdef PLATFORM_ANDROID

#include <jni.h> 
#include <android/native_window.h>

#else  // PLATFORM_ANDROID

#include <windows.h>

#endif

// -----------------------------------------------------------------------------
// Graphic
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Extension: OES_EGL_image_external
// https://www.khronos.org/registry/OpenGL/extensions/OES/OES_EGL_image_external.txt
// -----------------------------------------------------------------------------
#ifndef GL_OES_EGL_image_external

#define GL_OES_EGL_image_external 1
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#define GL_TEXTURE_BINDING_EXTERNAL_OES   0x8D67
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES 0x8D68

#endif /* GL_OES_EGL_image_external */

#ifdef PLATFORM_ANDROID

#define GL_GLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>

// -----------------------------------------------------------------------------
// Global defines
// -----------------------------------------------------------------------------
#define GFX_APIENTRY GL_APIENTRY

// -----------------------------------------------------------------------------
// Extension: EXT_buffer_storage
// -----------------------------------------------------------------------------
#define GL_MAP_PERSISTENT_BIT                      0x0040
#define GL_MAP_COHERENT_BIT                        0x0080
#define GL_DYNAMIC_STORAGE_BIT                     0x0100
#define GL_CLIENT_STORAGE_BIT                      0x0200

#define GL_BUFFER_IMMUTABLE_STORAGE_EXT            0x821F
#define GL_BUFFER_STORAGE_FLAGS_EXT                0x8220

#define GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT_EXT    0x00004000

typedef void (GFX_APIENTRY *GfxBufferStorageEXT)(GLenum target, GLsizeiptr size, const void * data, GLbitfield flags);

// -----------------------------------------------------------------------------
// Extension: EXT_disjoint_timer_query
// -----------------------------------------------------------------------------
#define GL_QUERY_COUNTER_BITS_EXT                       0x8864
#define GL_CURRENT_QUERY_EXT                            0x8865
#define GL_QUERY_RESULT_EXT                             0x8866
#define GL_QUERY_RESULT_AVAILABLE_EXT                   0x8867
#define GL_TIME_ELAPSED_EXT                             0x88BF
#define GL_TIMESTAMP_EXT                                0x8E28
#define GL_GPU_DISJOINT_EXT                             0x8FBB

typedef void (GFX_APIENTRY *GfxGenQueriesEXT)(GLsizei n, GLuint *ids);
typedef void (GFX_APIENTRY *GfxDeleteQueriesEXT)(GLsizei n, const GLuint *ids);
typedef GLboolean (GFX_APIENTRY *GfxIsQueryEXT)(GLuint id);
typedef void (GFX_APIENTRY *GfxBeginQueryEXT)(GLenum target, GLuint id);
typedef void (GFX_APIENTRY *GfxEndQueryEXT)(GLenum target);
typedef void (GFX_APIENTRY *GfxQueryCounterEXT)(uint id, GLenum target);
typedef void (GFX_APIENTRY *GfxGetQueryivEXT)(GLenum target, GLenum pname, GLint *params);
typedef void (GFX_APIENTRY *GfxGetQueryObjectivEXT)(GLuint id, GLenum pname, GLint *params);
typedef void (GFX_APIENTRY *GfxGetQueryObjectuivEXT)(GLuint id, GLenum pname, GLuint *params);
typedef void (GFX_APIENTRY *GfxGetQueryObjecti64vEXT)(GLuint id, GLenum pname, GLint64 *params);
typedef void (GFX_APIENTRY *GfxGetQueryObjectui64vEXT)(GLuint id, GLenum pname, GLuint64 *params);

#else // PLATFORM_ANDROID

#include "GL/glew.h"
#include "GL/wglew.h"

// -----------------------------------------------------------------------------
// Global defines
// -----------------------------------------------------------------------------
#define GFX_APIENTRY GLAPIENTRY

#endif