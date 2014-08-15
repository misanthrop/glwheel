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
			constexpr size_t bufsz = 1024*(sizeof(inotify_event) + 16);
			char buf[bufsz];
			int len = read(fd, buf, bufsz);
			for(char *p = buf; p < buf + len;)
			{
				inotify_event *event = (inotify_event *)p;
				p += sizeof(inotify_event);
				char empty = 0, *path = p;
				if(event->len) p += event->len; else path = &empty;
				fns[event->wd](event->mask, path);
				if(event->mask&IN_IGNORED) fns.erase(event->wd);
			}
		}
	};
}
