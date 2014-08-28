#include <utility>

namespace wheel
{
	using namespace std;

	template<class t, size_t sz> struct storage
	{
		unsigned char data[sz];

		t& get() { return *(t*)data; }
		const t& get() const { return *(t*)data; }

		t* operator->() { return &get(); }
		const t* operator->() const { return &get(); }

		storage() { new(data) t(); }
		storage(const storage& x) { new(data) t(x.get()); }
		storage(storage&& x) { new(data) t(move(x.get())); }
		storage(t&& x) { new(data) t(move(x)); }
		template<class... args> storage(args&&... v) { new(data) t(forward<args>(v)...); }

		storage& operator=(const storage& x) { get() = x.get(); return *this; }
		storage& operator=(const t& x) { get() = x; return *this; }
		storage& operator=(storage&& x) { get() = move(x.get()); return *this; }
		storage & operator=(t&& x) { get() = move(x); return *this; }
		~storage() { get().~t(); }
	};
}
