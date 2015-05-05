#pragma once
#include <GL/glew.h>
#include <GL/gl.h>
#include <windows.h>
#include <windowsx.h>
#include <functional>
#include <fstream>
#include <iostream>
#include <cassert>
#include "utf.hpp"
#include "app.hpp"
#include "audio.hpp"

namespace wheel
{
	namespace winapi
	{
		constexpr const key vkkey[256] =
		{
			key::unknown,	key::lbutton,	key::rbutton,	key::brk,		//00
			key::mbutton,	key::xbutton1,	key::xbutton2,	key::unknown,
			key::backspace,	key::tab,		key::unknown,	key::unknown,
			key::clear,		key::enter,		key::unknown,	key::unknown,
			key::lshift,	key::lcontrol,	key::lalt,		key::pause,		//10
			key::capslock,	key::unknown,	key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::esc,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::space,		key::pageup,	key::pagedown,	key::end,		//20
			key::home,		key::left,		key::up,		key::right,
			key::down,		key::select,	key::print,		key::exec,
			key::print,		key::insert,	key::del,		key::help,
			key::n0,		key::n1,		key::n2,		key::n3,		//30
			key::n4,		key::n5,		key::n6,		key::n7,
			key::n8,		key::n9,		key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::unknown, 	key::a,			key::b,			key::c,			//40
			key::d,			key::e,			key::f,			key::g,
			key::h,			key::i,			key::j,			key::k,
			key::l,			key::m,			key::n,			key::o,
			key::p,			key::q,			key::r,			key::s,			//50
			key::t,			key::u,			key::v,			key::w,
			key::x,			key::y,			key::z,			key::lsuper,
			key::rsuper,	key::menu,		key::unknown,	key::sleep,
			key::kp0,		key::kp1,		key::kp2,		key::kp3,		//60
			key::kp4,		key::kp5,		key::kp6,		key::kp7,
			key::kp8,		key::kp9,		key::multiply,	key::plus,
			key::separator,	key::minus,		key::period,	key::divide,
			key::f1,		key::f2,		key::f3,		key::f4,		//70
			key::f5,		key::f6,		key::f7,		key::f8,
			key::f9,		key::f10,		key::f11,		key::f12,
			key::f13,		key::f14,		key::f15,		key::f16,
			key::f17,		key::f18,		key::f19,		key::f20,		//80
			key::f21,		key::f22,		key::f23,		key::f24,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::numlock,	key::scroll,	key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,	//90
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::lshift,	key::rshift,	key::lcontrol,	key::rcontrol,
			key::lalt,		key::ralt,		key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,	//a0
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,	//b0
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::equals,
			key::comma,		key::minus,		key::period,	key::slash,
			key::grave,		key::unknown,	key::unknown,	key::unknown,	//c0
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::unknown,	//d0
			key::unknown,	key::unknown,	key::unknown,	key::unknown,
			key::unknown,	key::unknown,	key::unknown,	key::lbracket,
			key::backslash,	key::rbracket,	key::apostrophe,key::backtick
		};

		inline key tokey(WPARAM wp, LPARAM lp)
		{
			switch(wp)
			{
				case VK_CONTROL:
					return (lp&(1<<24)) ? key::rcontrol : key::lcontrol;
				case VK_SHIFT:
					// Stupid WinAPI does not recognize left and right shift.
					// The code below is suitable for WM_KEYDOWN events. For WM_KEYUPs workaround is needed.
					//if(GetKeyState(VK_LSHIFT) & 0x8000) return key::lshift;
					//if(GetKeyState(VK_RSHIFT) & 0x8000) return key::rshift;
					return key::lshift;
				case VK_MENU:
					return (lp&(1<<24)) ? key::ralt : key::lalt;
				case VK_RETURN:
					if(lp & 0x1000000) return key::kpenter;
			}
			UINT mvk = MapVirtualKey(HIWORD(lp)&0xff, wp);
			return vkkey[mvk?mvk:wp];
		}

		inline void updatebuttons(WPARAM wp)
		{
			*key::lbutton = (bool)(wp&MK_LBUTTON);
			*key::mbutton = (bool)(wp&MK_MBUTTON);
			*key::rbutton = (bool)(wp&MK_RBUTTON);
			*key::xbutton1 = (bool)(wp&MK_XBUTTON1);
			*key::xbutton2 = (bool)(wp&MK_XBUTTON2);
		}

		HWND wnd = 0;
		HDC dc;
		HGLRC rc;
		bool visible = 0, fs = 0;

		void clear()
		{
			wglMakeCurrent(0, 0);
			if(wnd)
			{
				wglDeleteContext(rc);
				ReleaseDC(wnd, dc);
				DestroyWindow(wnd);
				wnd = 0;
			}
		}

		LRESULT __stdcall wndproc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			switch(msg)
			{
				case WM_CREATE: SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lp)->lpCreateParams); return 0;
				case WM_DESTROY: PostQuitMessage(0); return 0;
				case WM_CLOSE: clear(); break;
				case WM_SHOWWINDOW: visible = wp; break;

				case WM_ACTIVATE:
					//if(fs) { if(wp == WA_INACTIVE) app->hide(); else app->show(); }
					visible = wp != WA_INACTIVE;
					memset(app->keystate, 0, sizeof(app->keystate));
					break;

				//case WM_MOVE: w->move(point(LOWORD(lp), HIWORD(lp))); return 0;

				case WM_SIZE:
					app->width = LOWORD(lp);
					app->height = HIWORD(lp);
					//w->size(point(LOWORD(lp), HIWORD(lp)));
					switch(wp)
					{
						//case SIZE_MINIMIZED: w->cur = vg::window::minimized; break;
						//case SIZE_RESTORED: w->cur = vg::window::normal; break;
						case SIZE_MAXIMIZED:
							if(fs)
							{
								DEVMODE mode;
								mode.dmSize = sizeof(mode);
								mode.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
								EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &mode);
								app->width = mode.dmPelsWidth;
								app->height = mode.dmPelsHeight;
								SetWindowLong(wnd, GWL_STYLE, WS_POPUP);
								SetWindowPos(wnd, HWND_TOPMOST, 0, 0, app->width, app->height, SWP_SHOWWINDOW);
							}
							break;
					}
					if(app->resized) app->resized();
					return 0;

				case WM_KEYDOWN: case WM_SYSKEYDOWN:
				{
					//if(lp&(1<<30)) return 0;
					key k = tokey(wp,lp); *k = true; if(app->pressed) app->pressed(k);
					break;
				}

				case WM_CHAR:
				{
					uint16_t* i = (uint16_t*)&wp;
					uint32_t uc; utf16to32(i, i+1, &uc);
					if(app->keytyped) app->keytyped(uc);
					return 0;
				}

				case WM_KEYUP: case WM_SYSKEYUP:
				{
					key k = tokey(wp,lp); *k = false; if(app->released) app->released(k);
					break;
				}

				case WM_LBUTTONDOWN: winapi::updatebuttons(wp); if(app->pressed) app->pressed(key::lbutton); return 1;
				case WM_RBUTTONDOWN: winapi::updatebuttons(wp); if(app->pressed) app->pressed(key::rbutton); return 1;
				case WM_MBUTTONDOWN: winapi::updatebuttons(wp); if(app->pressed) app->pressed(key::mbutton); return 1;
				case WM_XBUTTONDOWN: winapi::updatebuttons(wp); if(app->pressed) app->pressed(HIWORD(wp) == 1 ? key::xbutton1:key::xbutton2); return 1;
				case WM_LBUTTONUP: winapi::updatebuttons(wp); if(app->released) app->released(key::lbutton); return 0;
				case WM_RBUTTONUP: winapi::updatebuttons(wp); if(app->released) app->released(key::rbutton); return 0;
				case WM_MBUTTONUP: winapi::updatebuttons(wp); if(app->released) app->released(key::mbutton); return 0;
				case WM_XBUTTONUP: winapi::updatebuttons(wp); if(app->released) app->released(HIWORD(wp) == 1 ? key::xbutton1:key::xbutton2); return 0;
				case WM_MOUSEMOVE: app->pointers[0] = { (float)GET_X_LPARAM(lp), (float)GET_Y_LPARAM(lp) }; if(app->pointermoved) app->pointermoved(); return 0;
				case WM_MOUSEWHEEL: if(app->scrolled) app->scrolled(-GET_WHEEL_DELTA_WPARAM(wp)/WHEEL_DELTA); return 0;
			}
			return DefWindowProc(wnd, msg, wp, lp);
		}

		void init(const string& title, int w, int h)
		{
			constexpr const wchar_t classname[] = L"glwheel";

			HINSTANCE inst = GetModuleHandle(0);

			WNDCLASSEXW wcex = WNDCLASSEXW{
				sizeof(WNDCLASSEX), CS_OWNDC, wndproc, 0,0, inst, LoadIcon(0, IDI_APPLICATION), LoadCursor(0, IDC_ARROW),
				(HBRUSH)GetStockObject(BLACK_BRUSH), 0, classname, 0};
			RegisterClassExW(&wcex);

			if(!w) w = GetSystemMetrics(SM_CXSCREEN);
			if(!h) h = GetSystemMetrics(SM_CYSCREEN);
			RECT wr = {0,0,w,h}; AdjustWindowRectEx(&wr, WS_TILEDWINDOW, 0, 0);
			w = wr.right - wr.left;
			h = wr.bottom - wr.top;
			wchar_t title16[title.size() + 1];
			*utf8to16(title.begin(), title.end(), title16) = 0;
			wnd = CreateWindowExW(0, classname, title16, WS_TILEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, 0, 0, inst, 0);
			dc = GetDC(wnd);

			uint8_t depth = 16, stencil = 0, r = 0, g = 0, b = 0, a = 0;
			PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR),1, PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
										  32, r,0,g,0,b,0,a,0, 0,0,0,0,0, depth, stencil, 0,PFD_MAIN_PLANE, 0,0,0,0 };
			SetPixelFormat(dc, ChoosePixelFormat(dc, &pfd), &pfd);

			rc = wglCreateContext(dc);
			wglMakeCurrent(dc, rc);
			glewInit();
		}

		void title(const string& s)
		{
			wchar_t title16[s.size() + 1];
			*utf8to16(s.begin(), s.end(), title16) = 0;
			SetWindowTextW(wnd, title16);
		}

		void fullscreen(bool b)
		{
			if((fs = b))
			{
				ShowWindow(wnd, SW_SHOWNORMAL);
				ShowWindow(wnd, SW_SHOWMAXIMIZED);
			}
			else
			{
				SetWindowLongW(wnd, GWL_STYLE, WS_TILEDWINDOW);
				ShowWindow(wnd, SW_SHOWMAXIMIZED);
			}
		}

		void mci(const string& s) { mciSendStringA(s.c_str(), 0, 0, 0); }
	}

	void log(const string& s) { std::cout << s << endl; }
	void err(const string& s) { std::cerr << s << endl; }

	namespace events
	{
		int n = 0;
		HANDLE fds[64];
		function<void()> fns[64];

		void add(HANDLE fd, function<void()> fn) { fds[n] = fd; fns[n] = fn; ++n; }
		void remove(HANDLE fd) { for(int i = 0; i < n; ++i) if(fds[i] == fd) { --n; swap(fds[i],fds[n]); swap(fns[i],fns[n]); return; } }

		void process(int timeout)
		{
			DWORD r = 0;
			while((r = MsgWaitForMultipleObjects(n, fds, false, timeout, QS_ALLINPUT|QS_ALLPOSTMESSAGE)) != WAIT_TIMEOUT)
			{
				if(r == WAIT_FAILED) return;
				if(r == n)
				{
					MSG msg;
					while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
					{
						if(msg.message == WM_QUIT) break;
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				else if(r < n) fns[r]();
				timeout = 0;
			}
		}
	}

	application::application(const string& title, int w, int h) { assert(!app); app = this; winapi::init(title, w, h); }
	application::~application() { winapi::clear(); app = 0; }
	void application::process(int timeout) { events::process(timeout); }
	bool application::alive() const { return winapi::wnd; }
	bool application::visible() const { return winapi::visible; }
	void application::title(const string& s) { winapi::title(s); }
	void application::fullscreen(bool b) { winapi::fullscreen(b); }
	void application::togglefullscreen() { fullscreen(!winapi::fs); }
	void application::flip() { SwapBuffers(winapi::dc); }
	void application::show() { ShowWindow(winapi::wnd, SW_SHOWMAXIMIZED); }
	void application::hide() { ShowWindow(winapi::wnd, SW_SHOWMINIMIZED); }
	//void application::close() { winapi::clear(); }

	struct nativeaudiotrack
	{
		string name;
		bool isplaying;

		void clear()
		{
			isplaying = false;
			if(!name.empty()) winapi::mci("close " + name);
		}
		void set(string&& nm) { clear(); name = forward<string>(nm); winapi::mci("open " + name); }
		void setvolume(int v) { winapi::mci("setaudio " + name + " volume to " + to_string(v)); }
		void play(bool b)
		{
			isplaying = b;
			winapi::mci((b ? "play ":"stop ") + name + (b ? " repeat":""));
		}
	};

	audiotrack::audiotrack() : native(new nativeaudiotrack) {}
	audiotrack::~audiotrack() { clear(); }
	bool audiotrack::operator!() const { return native->name.empty(); }
	void audiotrack::clear() { native->clear(); }
	void audiotrack::set(string&& nm) { native->set(forward<string>(nm)); }
	void audiotrack::setvolume(int v) { native->setvolume(v); }
	void audiotrack::play(bool b) { native->play(b); }
	bool audiotrack::isplaying() { return native->isplaying; }

	int watch(const char *, function<void(uint32_t,const char*)>) { return -1; } // unimplemented yet

	string resource(const string& name)
	{
		ifstream f(name, ios::binary);
		return string(istreambuf_iterator<char>(f), istreambuf_iterator<char>());
	}

	application *app = 0;
}
