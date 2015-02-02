#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <sys/poll.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include "app.hpp"
#include "audio.hpp"
#include "utf.hpp"

namespace wheel
{
	namespace events
	{
		int n = 0;
		pollfd fds[64]; // in honor of Windows
		function<void()> fns[64];

		void add(int fd, function<void()> fn) { fds[n] = pollfd{fd,POLLIN}; fns[n] = fn; ++n; }
		void remove(int fd) { for(int i = 0; i < n; ++i) if(fds[i].fd == fd) { --n; swap(fds[i],fds[n]); swap(fns[i],fns[n]); return; } }
	}

	namespace xlib
	{
		constexpr char const*const atomnames[] = { "UTF8_STRING", "WM_DELETE_WINDOW", "_NET_WM_NAME", "_NET_WM_STATE", "_NET_WM_STATE_FULLSCREEN" };

		struct atoms
		{
			static constexpr size_t n = sizeof(atomnames)/sizeof(atomnames[0]);
			Atom id[n];

			static constexpr bool isequal(const char *a, const char *b) { return *a && *b ? isequal(a+1,b+1) : !*a && !*b; }
			static constexpr int find(const char *nm, int i = 0) { return isequal(nm,atomnames[i]) ? i : find(nm,i+1); }
			Atom& operator[](const char *nm) { return id[find(nm)]; }
		};

		inline uint8_t key(XKeyEvent *e)
		{
			KeySym k = XLookupKeysym(e,1);
			switch(k)
			{
				case XK_KP_0: return key::kp0;
				case XK_KP_1: return key::kp1;
				case XK_KP_2: return key::kp2;
				case XK_KP_3: return key::kp3;
				case XK_KP_4: return key::kp4;
				case XK_KP_5: return key::kp5;
				case XK_KP_6: return key::kp6;
				case XK_KP_7: return key::kp7;
				case XK_KP_8: return key::kp8;
				case XK_KP_9: return key::kp9;
				case XK_KP_Separator:
				case XK_KP_Decimal:   return key::del;
				case XK_KP_Equal:     return key::equals;
				case XK_KP_Enter:     return key::kpenter;
				default:              break;
			}
			k = XLookupKeysym(e,0);
			switch(k)
			{
				// Special keys (non character keys)
				case XK_Escape:       return key::esc;
				case XK_Tab:          return key::tab;
				case XK_Shift_L:      return key::lshift;
				case XK_Shift_R:      return key::rshift;
				case XK_Control_L:    return key::lcontrol;
				case XK_Control_R:    return key::rcontrol;
				case XK_Meta_L:
				case XK_Alt_L:        return key::lalt;
				case XK_Mode_switch:  // Mapped to Alt_R on many keyboards
				case XK_Meta_R:
				case XK_ISO_Level3_Shift: // AltGr on at least some machines
				case XK_Alt_R:        return key::ralt;
				case XK_Super_L:      return key::lmenu;
				case XK_Super_R:      return key::rmenu;
				case XK_Menu:         return key::menu;
				case XK_Num_Lock:     return key::numlock;
				case XK_Caps_Lock:    return key::capslock;
				case XK_Scroll_Lock:  return key::scroll;
				case XK_Pause:        return key::pause;
				case XK_KP_Delete:
				case XK_Delete:       return key::del;
				case XK_BackSpace:    return key::backspace;
				case XK_Return:       return key::enter;
				case XK_KP_Home:
				case XK_Home:         return key::home;
				case XK_KP_End:
				case XK_End:          return key::end;
				case XK_KP_Page_Up:
				case XK_Page_Up:      return key::pageup;
				case XK_KP_Page_Down:
				case XK_Page_Down:    return key::pagedown;
				case XK_KP_Insert:
				case XK_Insert:       return key::insert;
				case XK_KP_Left:
				case XK_Left:         return key::left;
				case XK_KP_Right:
				case XK_Right:        return key::right;
				case XK_KP_Down:
				case XK_Down:         return key::down;
				case XK_KP_Up:
				case XK_Up:           return key::up;
				case XK_F1:           return key::f1;
				case XK_F2:           return key::f2;
				case XK_F3:           return key::f3;
				case XK_F4:           return key::f4;
				case XK_F5:           return key::f5;
				case XK_F6:           return key::f6;
				case XK_F7:           return key::f7;
				case XK_F8:           return key::f8;
				case XK_F9:           return key::f9;
				case XK_F10:          return key::f10;
				case XK_F11:          return key::f11;
				case XK_F12:          return key::f12;
				case XK_F13:          return key::f13;
				case XK_F14:          return key::f14;
				case XK_F15:          return key::f15;
				case XK_F16:          return key::f16;
				case XK_F17:          return key::f17;
				case XK_F18:          return key::f18;
				case XK_F19:          return key::f19;
				case XK_F20:          return key::f20;
				case XK_F21:          return key::f21;
				case XK_F22:          return key::f22;
				case XK_F23:          return key::f23;
				case XK_F24:          return key::f24;
				case XK_F25:          return key::f25;
				case XK_KP_Divide:    return key::divide;
				case XK_KP_Multiply:  return key::multiply;
				case XK_KP_Subtract:  return key::minus;
				case XK_KP_Add:       return key::plus;
				case XK_KP_Equal:     return key::equals;
				case XK_KP_Enter:     return key::kpenter;
				default:
				{
					KeySym uc;
					XConvertCase(k,&k,&uc);
					if(32 <= k && k < 255) return (uint8_t)k;
					return key::unknown;
				}
			}
		}

		Display *dpy = 0;
		XIM im;
		XIC ic;
		xlib::atoms atom;
		unsigned NumLockMask;

		// window
		GLXContext ctx = 0;
		Visual *vis = 0;
		Window wnd;
		point m;

		inline int fd() { return ConnectionNumber(dpy); }
		inline int screen() { return DefaultScreen(dpy); }
		inline Window root() { return RootWindow(dpy,screen()); }
		inline int width() { return DisplayWidth(dpy,screen()); }
		inline int height() { return DisplayHeight(dpy,screen()); }
		inline int depth() { return DefaultDepth(dpy,screen()); }
		inline Visual *visual() { return DefaultVisual(dpy,screen()); }
		inline Colormap colormap() { return DefaultColormap(dpy,screen()); }
		inline int qlen() { return QLength(dpy); }

		inline unsigned numlock()
		{
			XModifierKeymap *modmap = XGetModifierMapping(dpy);
			for(unsigned i = 0; i < 8; ++i) for(unsigned j = 0; j < modmap->max_keypermod; ++j)
				if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock)) return 1 << i;
			return 0;
		}

		inline void sendclient(Window wnd, Atom type, long mask = 0, long d0 = 0, long d1 = 0, long d2 = 0, long d3 = 0, long d4 = 0)
		{
			XClientMessageEvent e;
			e.type = ClientMessage;
			e.window = wnd;
			e.message_type = type;
			e.format = 32;
			e.data.l[0] = d0; e.data.l[1] = d1; e.data.l[2] = d2; e.data.l[3] = d3; e.data.l[4] = d4;
			XSendEvent(dpy, root(), 0, mask, (XEvent*)&e);
		}

		void fullscreen(int b) { sendclient(wnd, atom["_NET_WM_STATE"], StructureNotifyMask, b, atom["_NET_WM_STATE_FULLSCREEN"]); }

		void nextevent()
		{
			XEvent event; XNextEvent(dpy, &event);
			switch(event.type)
			{
				case KeyPress:
				{
					uint8_t k = xlib::key(&event.xkey);
					key::state(k) = 1; app.press(k);
					if(!key::lalt && !key::lcontrol && !key::lmenu && !key::ralt && !key::rcontrol && !key::rmenu) if(' ' <= k && k < key::del)
					{
						char c[16];
						Status stat;
						if(int n = Xutf8LookupString(ic, (XKeyPressedEvent*)&event, c, 16, 0, &stat))
						{
							uint32_t uc;
							utf8to32(c, c + n, &uc);
							app.keycode(uc);
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
					key::state(k) = 0; app.release(k);
					break;
				}

				case ButtonPress:
					m = point(event.xbutton.x, app.height() - event.xbutton.y);
					switch(event.xbutton.button)
					{
						case Button1: key::state(key::lbutton) = 1; app.press(key::lbutton); break;
						case Button2: key::state(key::mbutton) = 1; app.press(key::mbutton); break;
						case Button3: key::state(key::rbutton) = 1; app.press(key::rbutton); break;
						case Button4: app.scroll(-1); break;
						case Button5: app.scroll(+1); break;
					}
					break;

				case ButtonRelease:
					switch(event.xbutton.button)
					{
						case Button1: key::state(key::lbutton) = 0; app.release(key::lbutton); break;
						case Button2: key::state(key::mbutton) = 0; app.release(key::mbutton); break;
						case Button3: key::state(key::rbutton) = 0; app.release(key::rbutton); break;
					}
					break;

				case ConfigureNotify:
					app.set(0, 0, event.xconfigure.width, event.xconfigure.height);
					app.resize();
					break;

				case MotionNotify: m = point(event.xmotion.x, app.height() - event.xmotion.y); app.pointermove(); break;
				case ClientMessage: if((Atom)event.xclient.data.l[0] == atom["WM_DELETE_WINDOW"]) app.close(); break;
				case MapNotify: app.active = true; break;
				case UnmapNotify: app.active = false; break;
				case DestroyNotify: app.close(); break;
			}
		}

		void init(int& w, int& h)
		{
			dpy = XOpenDisplay(0);
			im = XOpenIM(dpy, 0, 0, 0);
			ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing|XIMStatusNothing, nullptr);
			XInternAtoms(dpy, (char**)atomnames, atom.n, 0, atom.id);
			NumLockMask = numlock();

			XSync(dpy, 0);
			events::add(fd(), nextevent);

			int r = 1, g = 1, b = 1, a = 1, dpt = 16, stencil = 0;
			int attr[] = { GLX_RGBA, GLX_DOUBLEBUFFER, True,
				GLX_RED_SIZE, r, GLX_GREEN_SIZE, g, GLX_BLUE_SIZE, b, GLX_ALPHA_SIZE, a, GLX_DEPTH_SIZE, dpt, GLX_STENCIL_SIZE, stencil, 0 };
			if(XVisualInfo *vi = glXChooseVisual(dpy, screen(), attr))
			{
				vis = vi->visual;
				ctx = glXCreateContext(dpy, vi, 0, True);
			}
			else throw std::runtime_error("glXChooseVisual failed");

			XSetWindowAttributes wa;
			wa.border_pixel = 0;
			wa.colormap = XCreateColormap(dpy, root(), vis, AllocNone);
			wa.event_mask = KeyPressMask|KeyReleaseMask|PointerMotionMask|ButtonPressMask|ButtonReleaseMask|
				StructureNotifyMask|ExposureMask|FocusChangeMask|VisibilityChangeMask;

			if(!w) w = xlib::width();
			if(!h) h = xlib::height();
			wnd = XCreateWindow(dpy, root(), 0,0,w,h, 0, depth(), InputOutput, vis, CWBorderPixel|CWColormap|CWEventMask, &wa);
			if(!wnd) throw std::runtime_error("Failed to create window");

			XSetWMProtocols(dpy, wnd, &atom["WM_DELETE_WINDOW"], 1);
		}

		void clear()
		{
			if(ctx) { glXMakeCurrent(dpy, 0, 0); glXDestroyContext(dpy, ctx); ctx = 0; }
			if(dpy) { events::remove(fd()); XSync(dpy, 0); XDestroyIC(ic); XCloseIM(im); XCloseDisplay(dpy); dpy = 0; }
		}
	}

	void application::process(int timeout)
	{
		while(xlib::dpy && xlib::qlen()) xlib::nextevent();
		while(int r = poll(events::fds, events::n, timeout))
		{
			if(r < 0) break;
			for(int i = 0; i < events::n; ++i) if(events::fds[i].revents) { events::fns[i](); events::fds[i].revents = 0; }
			timeout = 0;
		}
	}

	bool application::alive() const { return xlib::dpy && xlib::ctx; }

	void application::init(const string& s, int w, int h)
	{
		xlib::init(w,h);
		set(0,0,w,h);
		title(s);
	}

	int application::pointercount(int) const { return *key::lbutton; }
	point application::pointer(int, int) const { return xlib::m; }
	void application::close() { xlib::clear(); }

	void application::title(const string& s)
	{
		XChangeProperty(xlib::dpy, xlib::wnd, xlib::atom["_NET_WM_NAME"], xlib::atom["UTF8_STRING"], 8, PropModeReplace, (uint8_t*)s.data(), s.size());
	}

	bool application::show(bool b)
	{
		if(b)
		{
			glXMakeCurrent(xlib::dpy, xlib::wnd, xlib::ctx);
			XMapWindow(xlib::dpy, xlib::wnd);
			XFlush(xlib::dpy);
		}
		else
		{
			glXMakeCurrent(xlib::dpy, 0, 0);
			XUnmapWindow(xlib::dpy, xlib::wnd);
		}
		return widget::show(b);
	}

	void application::update() { widget::update(); }
	void application::fullscreen(bool b) { xlib::fullscreen(b); }
	void application::togglefullscreen() { xlib::fullscreen(2); }
	void application::draw() { glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); widget::draw(); flip(); }
	void application::flip() { glXSwapBuffers(xlib::dpy, xlib::wnd); }

	int application::orientation() const { return 1; }
	void application::destroy() {}

	string application::resource(const string& name)
	{
		ifstream f(name, ios::binary);
		return string(istreambuf_iterator<char>(f), istreambuf_iterator<char>());
	}

	struct nativeaudiotrack {};

	audiotrack::audiotrack() : native(new nativeaudiotrack) {}
	audiotrack::~audiotrack() { clear(); }
	bool audiotrack::operator!() const { return 1; }
	void audiotrack::clear() {}	
	void audiotrack::set(string&&) {}
	void audiotrack::setvolume(int v) {}
	void audiotrack::play(bool) {}
	bool audiotrack::isplaying(){ return 0; }

	application app;
}
