#pragma once
#include <vector>
#include <string>
#include <functional>

namespace wheel
{
	using namespace std;

	enum class key : uint8_t
	{
		unknown = 0, backspace = 8, tab, enter, esc = 0x1b, space = ' ',
		apostrophe = 0x27, multiply = '*', plus, comma, minus, period, slash,
		n0, n1, n2, n3, n4, n5, n6, n7, n8, n9, semicolon = 0x3b, equals = 0x3d,
		lbracket = '[', backslash, rbracket,
		a = 'a', b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z, del = 0x7f,
		lbutton, rbutton, mbutton, xbutton1, xbutton2, pause, brk, clear,
		lcontrol, rcontrol, lshift, rshift, lalt, ralt, lsuper, rsuper,
		up, down, left, right, center, home, end, pageup, pagedown, insert, separator,
		capslock, scroll, numlock, select, print, sleep, exec, help,
		f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
		f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25,
		kp0, kp1, kp2, kp3, kp4, kp5, kp6, kp7, kp8, kp9, divide, kpenter,
		grave, backtick, power, volumeup, volumedown, play, stop, next, prev, rewind, forward, mute,
		ajoy, bjoy, cjoy, xjoy, yjoy, zjoy, l1joy, r1joy, l2joy, r2joy, lthumb, rthumb, start, mode,
		sym, explorer, envelope, notify, search, picsym, switchcharset,
		menu, star, pound, call, endcall, camera, at, num, headset, focus, last,
	};

	struct pointer { float x, y; };

	struct application
	{
		int width, height;
		bool keystate[uint8_t(key::last)];
		int pointercount;
		pointer pointers[32];

		function<void()> loaded, cleared, resumed, paused;
		function<void()> resized;
		function<void(float)> scrolled;
		function<void()> pointermoved;
		function<void(key)> pressed, released;
		function<void(uint32_t)> keytyped;
		function<void(float, float, float)> accel;

		application(const string& title, int w = 0, int h = 0);
		~application();
		void process(int timeout = -1);
		bool alive() const;
		bool visible() const;
		int orientation() const;
		void fullscreen(bool);
		void togglefullscreen();
		void title(const string&);
		void show();
		void hide();
		void flip();
		void close();
	};

	string resource(const string&);
	int watch(const char *, function<void(uint32_t,const char*)>);

	extern application *app;
}

inline bool& operator*(wheel::key i) { return wheel::app->keystate[uint8_t(i)]; }
inline bool operator!(wheel::key i) { return !wheel::app->keystate[uint8_t(i)]; }
