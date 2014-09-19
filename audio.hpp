#pragma once
#include <string>
#include <memory>

namespace wheel
{
	using namespace std;

	struct nativeaudiotrack;

	struct audiotrack
	{
		unique_ptr<nativeaudiotrack> native;		

		audiotrack();
		~audiotrack();
		bool operator!() const;
		void clear();
		void set(string&&);
		void setvolume(int v);
		bool isplaying();
		void play(bool = 1);
		void stop() { play(0); }
	};
}
