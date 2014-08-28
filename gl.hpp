#pragma once
#ifdef _WIN32
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES
#ifdef ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif
