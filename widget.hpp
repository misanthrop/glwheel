#pragma once
#include <vector>
#include "rect.hpp"
#include "keys.hpp"

namespace wheel
{
	struct widget : rect
	{
		widget *parent = 0;
		std::vector<widget*> children;
		bool active = 1;

		operator bool() const { if(parent && !parent->active) return 0; return active; }

		virtual ~widget() {}
		widget *at(const point& x) { for(widget *w : children) if(w->active) if(w->contains(x)) return w; return 0; }

		virtual bool show(bool b) { swap(active,b); return b; }
		virtual void update() { for(widget *c : children) if(*c) c->update(); }
		virtual widget *focus() { return at(pointer()); }
		virtual void focus(widget *) {}
		virtual point pointer() const { if(parent) return parent->pointer() - p; return point(); }
		virtual void pointermove() { if(widget *c = at(pointer())) c->pointermove(); }
		virtual void scroll(float d) { if(widget *c = focus()) c->scroll(d); }
		virtual void keycode(uint32_t k) { if(widget *c = focus()) c->keycode(k); }
		virtual void press(uint8_t k) { if(widget *c = focus()) c->press(k); }
		virtual void release(uint8_t k) { if(widget *c = focus()) c->release(k); }
		virtual void resize() { for(widget *c : children) if(*c) c->resize(); }
		virtual void draw() { for(widget *c : children) if(*c) c->draw(); }
		virtual void close() { if(parent) parent->remove(this); }
		virtual void clear() { for(widget *c : children) c->clear(); }
		virtual void reload() { for(widget *c : children) c->reload(); }

		void add(widget *w) { children.push_back(w); w->parent = this; }
		void remove(widget *w) { for(widget*& c : children) if(c == w) { c = children.back(); children.pop_back(); break; } }

		widget& operator<<(widget& w) { add(&w); return *this; }
	};
}
