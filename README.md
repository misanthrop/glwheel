# glwheel

C++ header-only cross-platform OpenGL windowing and event handling.

## Motivation

- Simplicity and minimalism
- Same makefile for multiple platforms and cross-compilation
- Single-threaded event processing
- Wheel reinvention

## Features

- Linux and Windows support
- Multiple file descriptor / handle event loop
- Unicode support
- Cross-platform make rules for GNU make (works with GCC and Clang)

## Planned soon

- Android, iOS, Mac OS X, Blackberry support
- File system notification support
- Socket notification support

## Hello World example

### hello.cpp

```cpp
#include <glwheel/window.hpp>
using namespace wheel;
int main()
{
	application app;				// includes event loop
	window wnd(app, u8"hello");		// u8 is optional and shows possibility to use Unicode
	wnd.show(true);
	while(!app.children.empty())	// while have windows
	{
		wnd.update();				// does nothing by default
		wnd.draw();					// clear, draw children, swap buffers
		app.process(1000);			// parameter is timeout in millisecond
	}
	return 0;
}
```

### makefile

```makefile
# additional compiler flags
CXXFLAGS += -O2

# executable name
target := hello

# space separated .cpp file names without extension
sources	:= hello

include wheel/default.mk
```
