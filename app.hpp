#pragma once
#include <cmath>
#include <cstdlib>
#include <string>
#include "widget.hpp"
#include "signals.hpp"

namespace wheel
{
	using namespace std;

	struct application : widget
	{
		sig<void()> resumed, paused;
		sig<void()> resized;
		sig<void(uint32_t)> keycoded;
		sig<void(float)> scrolled;
		sig<void()> pointermoved;
		sig<void(uint8_t)> pressed, released;
		sig<void(float,float,float)> accel;

		~application() { close(); }
		void process(int timeout = -1);
		bool alive() const;
		void init(const string& title, int w = 0, int h = 0);
		void destroy();
		void title(const string&);
		void fullscreen(bool);
		void togglefullscreen();
		void minimize();
		void flip();
		string resource(const string&);
		int pointercount(int time = 0) const;
		point pointer(int i = 0, int time = 0) const;
		void update();
		bool show(bool);
		void draw();
		void close();
		void pointermove() { pointermoved(); widget::pointermove(); }
		void scroll(float d) { scrolled(d); widget::scroll(d); }
		void keycode(uint32_t k) { keycoded(k); widget::keycode(k); }
		void press(uint8_t k) { pressed(k); widget::press(k); }
		void release(uint8_t k) { released(k); widget::release(k); }
		void resize() { resized(); widget::resize(); }
		int orientation() const;
	};

	int watch(const char *, function<void(uint32_t,const char*)>);

	extern application app;
}
