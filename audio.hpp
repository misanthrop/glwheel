#pragma once
#include <string>
#include "storage.hpp"

namespace wheel
{
	using namespace std;

	struct nativeaudiotrack;

	struct audiotrack
	{
		storage<nativeaudiotrack,64> native;

		~audiotrack() { clear(); }
		bool operator!() const;
		void clear();
		void set(string&&);
		void play(bool = 1);
		void stop() { play(0); }
	};
}
