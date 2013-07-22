#pragma once
#include <GL/glew.h>
#include <GL/gl.h>
#include <windows.h>
#include <windowsx.h>
#include "widget.hpp"
#include "events.hpp"
#include "utf.hpp"

namespace wheel
{
	namespace winapi
	{
		constexpr const key::type vkkey[256] =
		{
			key::unknown,	key::lbutton,	key::rbutton,	key::brk,		//00
			key::mbutton,	key::xbutton1,	key::xbutton2,	key::unknown,
			key::backspace,	key::tab,		key::unknown,	key::unknown,
			key::clear,		key::enter,		key::unknown,	key::unknown,
			key::shift,		key::control,	key::alt,		key::pause,		//10
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
			key::x,			key::y,			key::z,			key::lmenu,
			key::rmenu,		key::menu,		key::unknown,	key::sleep,
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

		inline key::type key(WPARAM wp, LPARAM lp)
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
			key::state(key::lbutton) = (bool)(wp&MK_LBUTTON);
			key::state(key::mbutton) = (bool)(wp&MK_MBUTTON);
			key::state(key::rbutton) = (bool)(wp&MK_RBUTTON);
			key::state(key::xbutton1) = (bool)(wp&MK_XBUTTON1);
			key::state(key::xbutton2) = (bool)(wp&MK_XBUTTON2);
		}
	}

	constexpr const wchar_t classname[] = L"vgwindow";

	struct application : widget
	{
		eventloop events;
		HINSTANCE inst = GetModuleHandle(0);
		WNDCLASSEX wcex = WNDCLASSEX{
			sizeof(WNDCLASSEX), CS_OWNDC, wndproc, 0,0, inst, LoadIcon(0, IDI_APPLICATION), LoadCursor(0, IDC_ARROW),
			(HBRUSH)GetStockObject(BLACK_BRUSH), 0, classname, 0};
		PIXELFORMATDESCRIPTOR pfd;
		point m;

		application(uint8_t depth = 16, uint8_t stencil = 0, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0)
			: pfd{sizeof(PIXELFORMATDESCRIPTOR),1, PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
				   32, r,0,g,0,b,0,a,0, 0,0,0,0,0, depth, stencil, 0,PFD_MAIN_PLANE, 0,0,0,0}
		{
			set(0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
			RegisterClassEx(&wcex);
		}

		point pointer() const { return m; }
		void process(int timeout = -1) { if(!children.empty()) events.process(timeout); }
		inline static LRESULT __stdcall wndproc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
	};

	struct window : widget
	{
		HWND wnd;
		HDC dc;
		HGLRC rc;
		bool fs = 0;

		operator HWND() const { return wnd; }
		application& app() { return *(application*)parent; }

		window(application& app, const char *title, int w = 0, int h = 0)
		{
			if(!w) w = app.width();
			if(!h) h = app.height();
			RECT wr = {0,0,w,h}; AdjustWindowRectEx(&wr, WS_TILEDWINDOW, 0, 0);
			w = wr.right - wr.left;
			h = wr.bottom - wr.top;
			size_t len = strlen(title);
			wchar_t title16[len + 1];
			*utf8to16(title, title + len, title16) = 0;
			wnd = CreateWindowEx(0, classname, title16, WS_TILEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, w, h, 0, 0, app.inst, this);
			dc = GetDC(wnd);
			SetPixelFormat(dc, ChoosePixelFormat(dc, &app.pfd), &app.pfd);
			rc = wglCreateContext(dc);
			wglMakeCurrent(dc, rc);
			glewInit();
			parent = &app;
			app.children.push_back(this);
		}

		~window()
		{
			wglMakeCurrent(0, 0);
			wglDeleteContext(rc);
			ReleaseDC(wnd, dc);
			DestroyWindow(wnd);
		}

		void show(bool b) { ShowWindow(wnd, b ? SW_SHOWMAXIMIZED:SW_SHOWMINIMIZED); }

		void fullscreen(bool b)
		{
			if((fs = b))
			{
				ShowWindow(wnd, SW_SHOWNORMAL);
				ShowWindow(wnd, SW_SHOWMAXIMIZED);
			}
			else
			{
				SetWindowLong(wnd, GWL_STYLE, WS_TILEDWINDOW);
				ShowWindow(wnd, SW_SHOWMAXIMIZED);
			}
		}
		void togglefullscreen() { fullscreen(!fs); }

		void makecurrent() { wglMakeCurrent(dc, rc); }
		void draw() { glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); widget::draw(); flip(); }
		void flip() { SwapBuffers(dc); }
	};

	LRESULT __stdcall application::wndproc(HWND wnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		window* w = (window*)GetWindowLongPtr(wnd, GWLP_USERDATA);
		switch(msg)
		{
			case WM_CREATE: SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lp)->lpCreateParams); return 0;
			case WM_DESTROY: return 0;
			case WM_CLOSE: w->close(); break;
			case WM_PAINT: return 0;

			case WM_ACTIVATE:
				if(w->fs) w->show(wp != WA_INACTIVE);
				for(uint8_t k = 0; k < key::last; ++k) key::state(k) = 0;
				return 0;

			case WM_MOVE: w->move(point(LOWORD(lp), HIWORD(lp))); return 0;

			case WM_SIZE:
				w->size(point(LOWORD(lp), HIWORD(lp)));
				switch(wp)
				{
					//case SIZE_MINIMIZED: w->cur = vg::window::minimized; break;
					//case SIZE_RESTORED: w->cur = vg::window::normal; break;
					case SIZE_MAXIMIZED:
						if(w->fs)
						{
							DEVMODE mode;
							mode.dmSize = sizeof(mode);
							mode.dmFields = DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
							EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &mode);
							w->set(0, 0, mode.dmPelsWidth, mode.dmPelsHeight);
							SetWindowLong(wnd, GWL_STYLE, WS_POPUP);
							SetWindowPos(wnd, HWND_TOPMOST, 0, 0, w->width(), w->height(), SWP_SHOWWINDOW);
						}
						break;
				}
				w->resize();
				return 0;

			case WM_KEYDOWN: case WM_SYSKEYDOWN:
			{
				//if(lp&(1<<30)) return 0;
				uint8_t k = winapi::key(wp,lp);
				key::state(k) = 1;
				w->press(k);
				break;
			}

			case WM_CHAR:
			{
				uint16_t* i = (uint16_t*)&wp;
				uint32_t uc; utf16to32(i, i+1, &uc);
				w->keycode(uc);
				return 0;
			}

			case WM_KEYUP: case WM_SYSKEYUP:
			{
				uint8_t k = winapi::key(wp,lp);
				key::state(k) = 0;
				w->release(k);
				break;
			}

			case WM_LBUTTONDOWN: winapi::updatebuttons(wp); w->press(key::lbutton); return 1;
			case WM_RBUTTONDOWN: winapi::updatebuttons(wp); w->press(key::rbutton); return 1;
			case WM_MBUTTONDOWN: winapi::updatebuttons(wp); w->press(key::mbutton); return 1;
			case WM_XBUTTONDOWN: winapi::updatebuttons(wp); w->press(HIWORD(wp) == 1 ? key::xbutton1:key::xbutton2); return 1;
			case WM_LBUTTONUP: winapi::updatebuttons(wp); w->release(key::lbutton); return 0;
			case WM_RBUTTONUP: winapi::updatebuttons(wp); w->release(key::rbutton); return 0;
			case WM_MBUTTONUP: winapi::updatebuttons(wp); w->release(key::mbutton); return 0;
			case WM_XBUTTONUP: winapi::updatebuttons(wp); w->release(HIWORD(wp) == 1 ? key::xbutton1:key::xbutton2); return 0;
			case WM_MOUSEMOVE: w->app().m = w->p + point(GET_X_LPARAM(lp), GET_Y_LPARAM(lp)); w->pointermove(); return 0;
			case WM_MOUSEWHEEL: w->scroll(GET_WHEEL_DELTA_WPARAM(wp)/WHEEL_DELTA); return 0;
		}
		return DefWindowProc(wnd, msg, wp, lp);
	}
}
