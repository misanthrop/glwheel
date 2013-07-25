#pragma once
#if defined(_WIN32)
	#include "winapi-gl.hpp"
#elif defined(ANDROID)
	#include "android-gl.hpp"
#else
	#include "xlib-gl.hpp"
#endif
