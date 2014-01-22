#pragma once
#if defined(_WIN32)

namespace wheel
{
	struct application;

	struct filewatcher
	{
		enum { ignored = 0 };

		filewatcher(application&) {}

		int add(const char *, function<void(uint32_t,const char*)>) { return 0; }
	};
}

#else
#include "filewatcher-inotify.hpp"
#endif
