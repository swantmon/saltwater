
#pragma once

#ifdef __ANDROID__

#define GL_GLEXT_PROTOTYPES

#include <EGL/egl.h>
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



#else // __ANDROID__

#include "GL/glew.h"
#include "GL/wglew.h"

#include <windows.h>

// -----------------------------------------------------------------------------
// Global defines
// -----------------------------------------------------------------------------
#define GFX_APIENTRY GLAPIENTRY

#endif