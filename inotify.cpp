#pragma once
#include "app.hpp"
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

	namespace events
	{
		void add(int fd, function<void()>);
		void remove(int fd);
	}

	struct inotify
	{
		int fd = -1;
		map<int,function<void(uint32_t,const char*)>> fns;

		~inotify() { clear(); }

		void create()
		{
			if(fd != -1) return;
			fd = inotify_init();
			events::add(fd, nextevent);
		}

		void clear()
		{
			events::remove(fd);
			if(fd != -1)
			{
				for(auto& wd : fns) inotify_rm_watch(fd, wd.first);
				close(fd);
			}
		}

		static inotify& instance() { static inotify i; i.create(); return i; }

		static void nextevent()
		{
			inotify& intf = instance();
			constexpr size_t bufsz = 1024*(sizeof(inotify_event) + 16);
			char buf[bufsz];
			int len = read(intf.fd, buf, bufsz);
			for(char *p = buf; p < buf + len;)
			{
				inotify_event *event = (inotify_event *)p;
				p += sizeof(inotify_event);
				char empty = 0, *path = p;
				if(event->len) p += event->len; else path = &empty;
				intf.fns[event->wd](event->mask, path);
				if(event->mask&IN_IGNORED) intf.fns.erase(event->wd);
			}
		}
	};

	int watch(const char *path, function<void(uint32_t,const char*)> fn)
	{
		inotify& intf = inotify::instance();
		int wd = inotify_add_watch(intf.fd, path, IN_MODIFY);
		intf.fns[wd] = fn;
		return wd;
	}
}
