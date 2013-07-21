#pragma once
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xproto.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include "keys.hpp"

namespace wheel
{
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

		template<char const*const* name, size_t n> struct atoms
		{
			Atom id[n];

			atoms(Display *dpy) { XInternAtoms(dpy, (char**)name, n, 0, id); }
			static constexpr bool isequal(const char *a, const char *b) { return *a && *b ? isequal(a+1,b+1) : !*a && !*b; }
			static constexpr int find(const char *nm, int i = 0) { return isequal(nm,name[i]) ? i : find(nm,i+1); }
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
	}
}
