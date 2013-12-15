#pragma once
#include <algorithm>

namespace wheel
{
	using namespace std;

	struct point
	{
		int x,y;

		point() {}
		point(int v) : x(v), y(v) {}
		point(int x, int y) : x(x), y(y) {}

		operator int*() { return &x; }
		operator const int*() const { return &x; }

		point operator+() const { return *this; }
		point operator-() const { return point(-x, -y); }

		point operator+(const point &v) const { return point(x + v.x, y + v.y); }
		point operator-(const point &v) const { return point(x - v.x, y - v.y); }
		point operator*(const point &v) const { return point(x*v.x, y*v.y); }
		point operator/(const point &v) const { return point(x/v.x, y/v.y); }
		point operator%(const point &v) const { return point(x%v.x, y%v.y); }
		point operator*(int f) const { return point(x*f, y*f); }
		point operator/(int f) const { return point(x/f, y/f); }
		point operator%(int f) const { return point(x%f, y%f); }
		int operator&(const point &v) const { return x*v.x + y*v.y; }
		int operator^(const point &v) const { return x*v.y - y*v.x; }

		point& operator+=(const point &v) { x += v.x; y += v.y; return *this; }
		point& operator-=(const point &v) { x -= v.x; y -= v.y; return *this; }
		point& operator*=(const point &v) { x *= v.x; y *= v.y; return *this; }
		point& operator/=(const point &v) { x /= v.x; y /= v.y; return *this; }
		point& operator%=(const point &v) { x %= v.x; y %= v.y; return *this; }
		point& operator*=(int f) { x *= f; y *= f; return *this; }
		point& operator/=(int f) { x /= f; y /= f; return *this; }
		point& operator%=(int f) { x %= f; y %= f; return *this; }

		bool operator==(const point &v) const { return x == v.x && y == v.y; }
		bool operator!=(const point &v) const { return x != v.x || y != v.y; }
		bool operator< (const point &v) const { return x <  v.x && y <  v.y; }
		bool operator<=(const point &v) const { return x <= v.x && y <= v.y; }
		bool operator> (const point &v) const { return x >  v.x && y >  v.y; }
		bool operator>=(const point &v) const { return x >= v.x && y >= v.y; }

		int operator~() const { return x*x + y*y; }

		point& operator=(int v) { x = y = v; return *this; }
		template<class t> point& operator=(const t& p) { x = p[0]; y = p[1]; return *this; }
	};

	struct rect
	{
		point p, q;

		rect() : p(0), q(0) {}
		rect(int x, int y, int w, int h) : p(x,y), q(x+w,y+h) {}
		rect(const point &pp, const point &qq) : p(pp), q(qq) {}
		rect(const rect &r) : p(r.p), q(r.q) {}

		point size() const { return q - p; }
		void size(const point& sz) { q = p + sz; }
		void move(const point& t) { point sz = size(); p = t; q = p + sz; }
		point pq() const { return point(p.x,q.y); }
		point qp() const { return point(q.x,p.y); }
		int x() const { return p.x; }
		int y() const { return p.y; }
		int width() const { return q.x - p.x; }
		int height() const { return q.y - p.y; }
		void set(int x, int y, int w, int h) { p = point(x,y); q = p + point(w,h); }
		operator bool() const { return p < q; }
		bool contains(int x, int y) const { return p.x <= x && p.y <= y && x < q.x && y < q.y; }
		bool contains(const point &v) const { return contains(v.x,v.y); }
		bool contains(const rect &r) const { return p <= r.p && r.q < q; }
		bool intersects(const rect &r) const { return max(p.x,r.p.x) < min(q.x,r.q.x) && max(p.y,r.p.y) < min(q.y,r.q.y); }
		bool operator==(const rect &r) const { return p == r.p && q == r.q; }
		bool operator!=(const rect &r) const { return p != r.p || q != r.q; }
		rect operator+(const point &t) const { return rect(p+t,q+t); }
		rect operator-(const point &t) const { return rect(p-t,q-t); }
		rect& operator&=(const rect &r) { if(p.x < r.p.x) p.x = r.p.x; if(p.y < r.p.y) p.y = r.p.y; if(q.x > r.q.x) q.x = r.q.x; if(q.y > r.q.y) q.y = r.q.y; return *this; }
		rect& operator|=(const rect &r) { if(p.x > r.p.x) p.x = r.p.x; if(p.y > r.p.y) p.y = r.p.y; if(q.x < r.q.x) q.x = r.q.x; if(q.y < r.q.y) q.y = r.q.y; return *this; }
		rect operator&(const rect &r) const { return rect(*this) &= r; }
		rect operator|(const rect &r) const { return rect(*this) |= r; }
		rect& operator|=(const point &v) { if(p.x > v.x) p.x = v.x; if(p.y > v.y) p.y = v.y; if(q.x < v.x) q.x = v.x; if(q.y < v.y) q.y = v.y; return *this; }
		rect operator|(const point &v) const { return rect(*this) |= v; }
	};
}
