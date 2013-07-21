#pragma once
#include <vector>
#include <functional>
#include <iostream>
#ifdef _WIN32
	#include <windows.h>
	typedef HANDLE pollfd; // :(
#else
	#include <sys/poll.h>
	inline bool operator==(const pollfd& a, const pollfd& b) { return a.fd == b.fd; }
#endif

namespace wheel
{
	using namespace std;

	struct eventloop
	{
		int n = 0;
		pollfd fds[64]; // in honor of Windows
		function<void(pollfd)> fns[64];

		void add(pollfd fd, function<void(pollfd)> fn) { fds[n] = fd; fns[n] = fn; ++n; }
		void remove(pollfd fd) { for(int i = 0; i < n; ++i) if(fds[i] == fd) { --n; swap(fds[i],fds[n]); swap(fns[i],fns[n]); return; } }

		void process(int timeout)
		{
		#ifdef _WIN32
			DWORD r = 0;
			while((r = MsgWaitForMultipleObjects(n, fds, false, timeout, QS_ALLEVENTS)) != WAIT_TIMEOUT)
			{
				if(r == WAIT_FAILED) { cerr << "MsgWaitForMultipleObjects() failed" << endl; return; }
				if(r == n)
				{
					MSG msg;
					if(GetMessage(&msg, 0, 0, 0))
					{
						if(msg.message == WM_QUIT) break;
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				else if(0 <= r && r < n) fns[r](fds[r]);
				timeout = 0;
			}
		#else
			while(int r = poll(fds, n, timeout))
			{
				if(r < 0) { cerr << "poll() failed" << endl; return; }
				for(int i = 0; i < n; ++i) if(fds[i].revents) fns[i](fds[i]);
				timeout = 0;
			}
		#endif
		}
	};
}
