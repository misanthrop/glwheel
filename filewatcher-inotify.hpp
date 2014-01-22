#pragma once
#include "application.hpp"
#include <map>
#include <string>
#include <functional>
#include <cstdint>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <fcntl.h>

namespace wheel
{
	using namespace std;

	struct filewatcher
	{
		enum { ignored = IN_IGNORED };

		application& app;
		int fd = inotify_init();
		map<int,function<void(uint32_t,const char*)>> fns;

		filewatcher(application& app) : app(app)
		{
			app.events.add(pollfd{fd,POLLIN}, [&](pollfd) { nextevent(); });
		}

		~filewatcher()
		{
			app.events.remove(pollfd{fd});
			for(auto& wd : fns) inotify_rm_watch(fd, wd.first);
			close(fd);
		}

		int add(const char *path, function<void(uint32_t,const char*)> fn)
		{
			int wd = inotify_add_watch(fd, path, IN_MODIFY);
			fns[wd] = fn;
			return wd;
		}

		void nextevent()
		{
			inotify_event event;
			int len = read(fd, (char*)&event, sizeof(inotify_event));
			if(len < 0) return;
			char path[event.len+1];
			if(event.len)
			{
				len = read(fd, path, event.len);
				if(len < 0) return;
			}
			else path[0] = 0;
			fns[event.wd](event.mask,path);
			if(event.mask&IN_IGNORED) fns.erase(event.wd);
		}
	};
}
