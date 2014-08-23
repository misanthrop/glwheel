#pragma once
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
#include <stdexcept>
#include <functional>
#include "widget.hpp"
#include "utf.hpp"

inline bool operator==(const pollfd& a, const pollfd& b) { return a.fd == b.fd; }

namespace wheel
{
	struct eventloop
	{
		int n = 0;
		pollfd fds[64]; // in honor of Windows
		function<void(pollfd)> fns[64];

		void add(pollfd fd, function<void(pollfd)> fn) { fds[n] = fd; fns[n] = fn; ++n; }
		void remove(pollfd fd) { for(int i = 0; i < n; ++i) if(fds[i] == fd) { --n; swap(fds[i],fds[n]); swap(fns[i],fns[n]); return; } }

		void process(int timeout)
		{
			while(int r = poll(fds, n, timeout))
			{
				if(r < 0) break;
				for(int i = 0; i < n; ++i) if(fds[i].revents) { fns[i](fds[i]); fds[i].revents = 0; }
				timeout = 0;
			}
		}
	};

	namespace xlib
	{
		struct display
		{
			Display *dpy = XOpenDisplay(0);
			XIM im = XOpenIM(dpy, 0, 0, 0);
			XIC ic = XCreateIC(im, XNInputStyle, XIMPreeditNothing|XIMStatusNothing, nullptr);
			unsigned NumLockMask;
			Atom tray;

			display() { NumLockMask = numlock(); XSync(dpy, 0); }
			~display() { XSync(dpy, 0); XDestroyIC(ic); XCloseIM(im); XCloseDisplay(dpy); }

			operator Display*() const { return dpy; }
			int fd() const { return ConnectionNumber(dpy); }
			int screen() const { return DefaultScreen(dpy); }
			Window root() const { return RootWindow(dpy,screen()); }
			int width() const { return DisplayWidth(dpy,screen()); }
			int height() const { return DisplayHeight(dpy,screen()); }
			int depth() const { return DefaultDepth(dpy,screen()); }
			Visual *visual() const { return DefaultVisual(dpy,screen()); }
			Colormap colormap() const { return DefaultColormap(dpy,screen()); }
			int qlen() const { return QLength(dpy); }

			unsigned numlock() const
			{
				XModifierKeymap *modmap = XGetModifierMapping(dpy);
				for(unsigned i = 0; i < 8; ++i) for(unsigned j = 0; j < modmap->max_keypermod; ++j)
					if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock)) return 1 << i;
				return 0;
			}

			inline unsigned cleanmask(unsigned mask)
			{
				return mask & ~(NumLockMask|LockMask) & (ShiftMask|ControlMask|Mod1Mask|Mod2Mask|Mod3Mask|Mod4Mask|Mod5Mask);
			}

			bool grabkey(KeySym sym, unsigned mod = AnyModifier)
			{
				KeyCode code = XKeysymToKeycode(dpy, sym); if(!code) return 0;
				XGrabKey(dpy, code, mod, root(), True, GrabModeAsync, GrabModeAsync);
				if(mod == AnyModifier) return true;
				XGrabKey(dpy, code, mod|LockMask, root(), True, GrabModeAsync, GrabModeAsync);
				XGrabKey(dpy, code, mod|NumLockMask, root(), True, GrabModeAsync, GrabModeAsync);
				XGrabKey(dpy, code, mod|LockMask|NumLockMask, root(), True, GrabModeAsync, GrabModeAsync);
				return true;
			}

			bool supported(Window w, Atom proto)
			{
				bool exists = false;
				Atom *protocols;
				int n;
				if(XGetWMProtocols(dpy, w, &protocols, &n))
				{
					while(!exists && n--) exists = protocols[n] == proto;
					XFree(protocols);
				}
				return exists;
			}

			void sendclient(Window wnd, Atom type, long mask = 0, long d0 = 0, long d1 = 0, long d2 = 0, long d3 = 0, long d4 = 0)
			{
				XClientMessageEvent e;
				e.type = ClientMessage;
				e.window = wnd;
				e.message_type = type;
				e.format = 32;
				e.data.l[0] = d0; e.data.l[1] = d1; e.data.l[2] = d2; e.data.l[3] = d3; e.data.l[4] = d4;
				XSendEvent(dpy, root(), 0, mask, (XEvent*)&e);
			}

			unsigned char kbstate() { XkbStateRec kbstate; XkbGetState(dpy, XkbUseCoreKbd, &kbstate); return  kbstate.group; }

			void place(Window wnd, int x, int y, int w, int h, int bw = 1)
			{
				XConfigureEvent ce;
				ce.type = ConfigureNotify;
				ce.display = dpy;
				ce.event = ce.window = wnd;
				ce.x = x; ce.y = y; ce.width = w; ce.height = h;
				ce.border_width = bw; ce.above = None; ce.override_redirect = 0;
				XSendEvent(dpy, wnd, 0, StructureNotifyMask, (XEvent *)&ce);
				XMoveResizeWindow(dpy, wnd, x, y, w, h);
			}

			Atom atomproperty(Window w, Atom prop) const
			{
				int format;
				unsigned long n, left;
				uint8_t *data = 0;
				Atom type, atom = 0;
				if(XGetWindowProperty(dpy, w, prop, 0, sizeof atom, 0, XA_ATOM, &type, &format, &n, &left, &data) == Success && data)
				{
					atom = *(Atom *)data;
					XFree(data);
				}
				return atom;
			}
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

		constexpr char const*const atomnames[] = { "UTF8_STRING", "WM_DELETE_WINDOW", "_NET_WM_NAME", "_NET_WM_STATE", "_NET_WM_STATE_FULLSCREEN" };

		struct atoms
		{
			static constexpr size_t n = sizeof(atomnames)/sizeof(atomnames[0]);
			Atom id[n];

			atoms(Display *dpy) { XInternAtoms(dpy, (char**)atomnames, n, 0, id); }
			static constexpr bool isequal(const char *a, const char *b) { return *a && *b ? isequal(a+1,b+1) : !*a && !*b; }
			static constexpr int find(const char *nm, int i = 0) { return isequal(nm,atomnames[i]) ? i : find(nm,i+1); }
			Atom& operator[](const char *nm) { return id[find(nm)]; }
		};
	}

	struct application : widget, xlib::display
	{
		eventloop events;
		xlib::atoms atom;
		GLXContext ctx = 0;
		Visual *vis = 0;
		point m;

		application(uint8_t depth = 16, uint8_t stencil = 0, uint8_t r = 1, uint8_t g = 1, uint8_t b = 1, uint8_t a = 1) : atom(dpy)
		{
			set(0,0,DisplayWidth(dpy,screen()),DisplayHeight(dpy,screen()));
			int attr[] = { GLX_RGBA, GLX_DOUBLEBUFFER, True,
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
			bool active, visible = 0;
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

			bool show() const { return visible; }

			void show(bool b)
			{
				if(b)
				{
					makecurrent();
					XMapWindow(app(), wnd);
					XFlush(app());
				}
				else
				{
					glXMakeCurrent(app(),0,0);
					XUnmapWindow(app(), wnd);
				}
				visible = b;
			}

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
				m = point(event.xbutton.x, w->height() - event.xbutton.y);
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
				w->set(0, 0, event.xconfigure.width, event.xconfigure.height);
				w->resize();
				break;

			case MotionNotify: m = point(event.xmotion.x, w->height() - event.xmotion.y); w->pointermove(); break;
			case ClientMessage: if((Atom)event.xclient.data.l[0] == atom["WM_DELETE_WINDOW"]) w->close(); break;
			case MapNotify: ((native::window*)w)->visible = true; break;
			case UnmapNotify: ((native::window*)w)->visible = false; break;
			case DestroyNotify: w->close(); break;
		}
	}

	struct audiotrack
	{
		~audiotrack() { clear(); }
		bool operator!() const { return 1; }
		void clear() {}
		void set(string&&) {}
		void play(bool = 1) {}
		void stop() { play(0); }
	};
}
