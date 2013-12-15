#pragma once
#include <cstdint>

namespace wheel
{
	namespace key
	{
		enum type : uint8_t
		{
			unknown = 0, backspace = 8, tab, enter, esc = 0x1b, space = ' ',
			apostrophe = 0x27, multiply = '*', plus, comma, minus, period, slash,
			n0, n1, n2, n3, n4, n5, n6, n7, n8, n9, semicolon = 0x3b, equals = 0x3d,
			lbracket = '[', backslash, rbracket,
			a = 'a', b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
			del = 0x7f, lbutton, rbutton, mbutton, xbutton1, xbutton2, pause, brk, clear,
			control, lcontrol, rcontrol, shift, lshift, rshift, alt, lalt, ralt, menu, lmenu, rmenu,
			up, down, left, right, center, home, end, pageup, pagedown, insert, separator,
			capslock, scroll, numlock, select, print, sleep, exec, help,
			f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
			f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25,
			kp0, kp1, kp2, kp3, kp4, kp5, kp6, kp7, kp8, kp9, divide, kpenter,
			grave, backtick, power, volumeup, volumedown, play, stop, next, prev, rewind, forward, mute, last
		};
		inline uint8_t& state(uint8_t k) { static uint8_t state[last]; return state[k]; }
	}
}

inline uint8_t operator*(wheel::key::type i) { return wheel::key::state(i); }
inline uint8_t operator!(wheel::key::type i) { return !wheel::key::state(i); }
