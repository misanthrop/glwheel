#pragma once
#include <list>
#include <functional>

namespace wheel
{
    using namespace std;

	template<class signature> struct sig;

	template<class... args> struct sig<void(args...)>
    {
		using slot = function<void(args...)>;
		using connection = typename list<slot>::iterator;
		list<slot> slots;
        connection begin() { return slots.begin(); }
        connection end() { return slots.end(); }
		connection operator+=(slot fn) { return slots.insert(slots.end(), fn); }
		connection connect(slot fn, connection c) { return slots.insert(c, fn); }
        connection operator-=(connection c) { return slots.erase(c); }
        void clear() { slots.clear(); }
        void operator()(args... a) { for(auto& s : slots) s(a...); }
    };
}
