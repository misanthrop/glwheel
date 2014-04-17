#pragma once
#if defined(_WIN32)
	#include "winapi.hpp"
#elif defined(ANDROID)
	#include "android.hpp"
#else
	#include "xlib.hpp"
#endif
