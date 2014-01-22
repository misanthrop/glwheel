#pragma once
#include "application.hpp"
#include "signals.hpp"

namespace wheel
{
	struct window : native::window
	{
		signal<void()> resized;
		signal<void(uint32_t)> keycoded;
		signal<void(int)> scrolled;
		signal<void()> pointermoved;
		signal<void(uint8_t)> pressed, released;

		window(application &app, const char *title, int w = 0, int h = 0) : native::window(app, title, w,h) {}

		void pointermove() { pointermoved(); widget::pointermove(); }
		void scroll(int d) { scrolled(d); widget::scroll(d); }
		void keycode(uint32_t k) { keycoded(k); widget::keycode(k); }
		void press(uint8_t k) { pressed(k); widget::press(k); }
		void release(uint8_t k) { released(k); widget::release(k); }
		void resize() { resized(); widget::resize(); }
	};
}
