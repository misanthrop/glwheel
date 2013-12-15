#pragma once
#include "xlib.hpp"
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include "widget.hpp"
#include "events.hpp"
#include "utf.hpp"
#include <cstring>

namespace wheel
{
	constexpr char const*const atomnames[] = { "UTF8_STRING", "WM_DELETE_WINDOW", "_NET_WM_NAME", "_NET_WM_STATE", "_NET_WM_STATE_FULLSCREEN" };

	struct application : widget, xlib::display
	{
		eventloop events;
		xlib::atoms<atomnames,5> atom;
		GLXContext ctx = 0;
		Visual *vis = 0;
		point m;

		application(uint8_t depth = 16, uint8_t stencil = 0, uint8_t r = 1, uint8_t g = 1, uint8_t b = 1, uint8_t a = 1) : atom(dpy)
		{
			set(0,0,DisplayWidth(dpy,screen()),DisplayHeight(dpy,screen()));
			int attr[] = { GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, GLX_DOUBLEBUFFER, True,
				GLX_RED_SIZE,r, GLX_GREEN_SIZE,g, GLX_BLUE_SIZE,b, GLX_ALPHA_SIZE,a, GLX_DEPTH_SIZE, depth, GLX_STENCIL_SIZE, stencil, 0 };
			if(XVisualInfo *vi = glXChooseVisual(dpy, screen(), attr))
			{
				vis = vi->visual;
				ctx = glXCreateContext(dpy, vi, 0, True);
			}
			else throw std::runtime_error("glXChooseVisual failed");
			events.add(pollfd{fd(),POLLIN}, [&](pollfd) { nextevent(); });
		}

		~application()
		{
			events.remove(pollfd{fd()});
			if(ctx) { glXMakeCurrent(dpy, 0, 0); glXDestroyContext(dpy, ctx); }
		}

		operator bool() const { return !children.empty(); }

		int width() const { return rect::width(); }
		int height() const { return rect::height(); }
		point pointer() const { return m; }

		void process(int timeout = -1)
		{
			while(qlen()) nextevent();
			events.process(timeout);
		}

		inline void nextevent();
	};

	namespace native
	{
		struct window : widget
		{
			bool active;
			Window wnd;

			operator Window() const { return wnd; }
			application& app() { return *(application*)parent; }

			window(application& app, const char *title, int w = 0, int h = 0)
			{
				XSetWindowAttributes wa;
				wa.border_pixel = 0;
				wa.colormap = XCreateColormap(app, app.root(), app.vis, AllocNone);
				wa.event_mask = KeyPressMask|KeyReleaseMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|
					StructureNotifyMask|ExposureMask|FocusChangeMask|VisibilityChangeMask;

				if(!w) w = app.width();
				if(!h) h = app.height();
				wnd = XCreateWindow(app, app.root(), 0,0,w,h, 0, app.depth(), InputOutput, app.vis, CWBorderPixel|CWColormap|CWEventMask, &wa);
				if(!wnd) throw std::runtime_error("Failed to create window");
				set(0,0,w,h);

				XChangeProperty(app, wnd, app.atom["_NET_WM_NAME"], app.atom["UTF8_STRING"], 8, PropModeReplace, (uint8_t*)title, strlen(title));
				XSetWMProtocols(app, wnd, &app.atom["WM_DELETE_WINDOW"], 1);

				parent = &app;
				app.children.push_back(this);
			}

			void show(bool b) { if(b) { makecurrent(); XMapWindow(app(), wnd); } else { glXMakeCurrent(app(),0,0); XUnmapWindow(app(), wnd); } }

			void fullscreen(int b)
			{
				app().sendclient(wnd, app().atom["_NET_WM_STATE"], StructureNotifyMask, b, app().atom["_NET_WM_STATE_FULLSCREEN"]);
			}

			void togglefullscreen() { fullscreen(2); }
			void makecurrent() { glXMakeCurrent(app(), wnd, app().ctx); }
			void draw() { glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); widget::draw(); flip(); }
			void flip() { glXSwapBuffers(app(), wnd); }
		};
	}

	void application::nextevent()
	{
		XEvent event; XNextEvent(dpy, &event);
		Window wnd = event.xany.window;
		widget *w = 0;
		if(wnd == root()) w = this; else for(widget *c : children) if(((native::window*)c)->wnd == wnd) { w = c; break; }
		if(w) switch(event.type)
		{
			case KeyPress:
			{
				uint8_t k = xlib::key(&event.xkey);
				key::state(k) = 1; w->press(k);
				if(!key::lalt && !key::lcontrol && !key::lmenu && !key::ralt && !key::rcontrol && !key::rmenu) if(' ' <= k && k < key::del)
				{
					char c[16];
					Status stat;
					if(int n = Xutf8LookupString(ic, (XKeyPressedEvent*)&event, c, 16, 0, &stat))
					{
						uint32_t uc;
						utf8to32(c, c + n, &uc);
						w->keycode(uc);
					}
				}
				break;
			}

			case KeyRelease:
			{
				if(XEventsQueued(dpy, QueuedAfterReading))
				{
					XEvent nextEvent;
					XPeekEvent(dpy, &nextEvent);
					if(nextEvent.type == KeyPress && nextEvent.xkey.window == event.xkey.window &&
					   nextEvent.xkey.keycode == event.xkey.keycode && nextEvent.xkey.time - event.xkey.time < 20) break;
				}
				uint8_t k = xlib::key(&event.xkey);
				key::state(k) = 0; w->release(k);
				break;
			}

			case ButtonPress:
				m = point(event.xbutton.x_root, event.xbutton.y_root);
				switch(event.xbutton.button)
				{
					case Button1: key::state(key::lbutton) = 1; w->press(key::lbutton); break;
					case Button2: key::state(key::mbutton) = 1; w->press(key::mbutton); break;
					case Button3: key::state(key::rbutton) = 1; w->press(key::rbutton); break;
					case Button4: w->scroll(-1); break;
					case Button5: w->scroll(+1); break;
				}
				break;

			case ButtonRelease:
				switch(event.xbutton.button)
				{
					case Button1: key::state(key::lbutton) = 0; w->release(key::lbutton); break;
					case Button2: key::state(key::mbutton) = 0; w->release(key::mbutton); break;
					case Button3: key::state(key::rbutton) = 0; w->release(key::rbutton); break;
				}
				break;

			case ConfigureNotify:
				w->set(event.xconfigure.x, event.xconfigure.y, event.xconfigure.width, event.xconfigure.height);
				w->resize();
				break;

			case MotionNotify: m = point(event.xmotion.x_root,event.xmotion.y_root); w->pointermove(); break;
			case ClientMessage: if((Atom)event.xclient.data.l[0] == atom["WM_DELETE_WINDOW"]) w->close(); break;
			case DestroyNotify: w->close(); break;
		}
	}
}
