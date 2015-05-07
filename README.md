# glwheel

C++ header-only cross-platform OpenGL windowing and event handling.

## Motivation

- Simplicity and minimalism
- Same makefile for multiple platforms and cross-compilation
- Wheel reinvention

## Features

- Linux, Windows and Android support
- Unicode support
- Flexible GNU make rules for multiple platforms and cross-compilation
- File system notification for Linux and Android

## Planned soon

- iOS, OS X, Blackberry support
- Cross-platform file system notification support
- Socket notification support

## Hello World example

### examble.cpp

```cpp
#include "impl.hpp"					// include to one of your .cpp files
using namespace wheel;

int main()
{
	application app("glwheel example");	// UTF-8 string are supported
	app.pressed = [&](key k) { if(k == key::f11) app.togglefullscreen(); };
	app.show();
	while(app.alive())				// while not closed
	{
		if(app.visible())			// draw only when visible
		{
			glClearColor(app.pointers[0].x/app.width, 0, app.pointers[0].y/app.height, 1);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			app.flip();				// flips pages
			app.process(0);			// processes events, argument is timeout
		}
		else app.process();			// default value is -1, means to wait for the next event
	}
	return 0;
}
```

### makefile

```makefile
# target executable or apk name
target := example

# application name, now used as Android activity name
name := GLwheel Example

# space separated .c or .cpp file names without extension
sources := example

include default.mk
```

### Build instructions on Linux host

Build for Linux using default compiler.
```
make
```
Build for Windows. Requires cross-compilers *i686-w64-mingw32-g++* and *x86_64-w64-mingw32-gcc*,
for *i686* and *x86_64* respectivelly. Tested on Arch Linux.
```
make windows
```
Build for Android. Requires [Android NDK](https://developer.android.com/tools/sdk/ndk/index.html) r10.
ANDROID_NDK environment variable should be set properly.
```
make archive-android
```
### Build instructions on Windows host

Requires [MinGW](http://www.mingw.org) and [GLEW](http://glew.sourceforge.net).
Suggested compiler distribution is [Nuwen MinGW](http://nuwen.net/mingw.html).
It already includes [GLEW](http://glew.sourceforge.net) library.
Build should be run from Bash-shell (e.g. *git-bash*),
*rm* and *uname* tools should be accessible in $PATH.
```
make
```
