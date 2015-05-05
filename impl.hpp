#include "app.hpp"

#if defined(_WIN32)
	#include "winapi.cpp"
#elif defined(ANDROID)
	#include "android.cpp"
	#include "inotify.cpp"
#else
	#include "xlib.cpp"
	#include "inotify.cpp"
#endif
