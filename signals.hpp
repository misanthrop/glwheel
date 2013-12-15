#pragma once
#include <list>
#include <functional>

namespace wheel
{
    using namespace std;

    template<class signature> struct signal;

    template<class... args> struct signal<void(args...)>
    {
        typedef function<void(args...)> slot_type;
        typedef typename list<slot_type>::iterator connection;
        list<slot_type> slots;
        connection begin() { return slots.begin(); }
        connection end() { return slots.end(); }
        connection operator+=(slot_type slot) { return slots.insert(slots.end(), slot); }
        connection connect(slot_type slot, connection c) { return slots.insert(c, slot); }
        connection operator-=(connection c) { return slots.erase(c); }
        void clear() { slots.clear(); }
        void operator()(args... a) { for(auto& s : slots) s(a...); }
    };
}
