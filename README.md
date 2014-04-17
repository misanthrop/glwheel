# glwheel

C++ header-only cross-platform OpenGL windowing and event handling.

## Motivation

- Simplicity and minimalism
- Same makefile for multiple platforms and cross-compilation
- Single-threaded event processing
- Wheel reinvention

## Features

- Linux, Windows and Android support
- Multiple file descriptor / handle event loop
- Unicode support
- Flexible GNU make rules for multiple platforms and cross-compilation
- File system notification for Linux and Android

## Planned soon

- iOS, Mac OS X, Blackberry support
- Cross-platform file system notification support
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
	while(app)						// while have windows
	{
		wnd.update();				// does nothing by default
		if(wnd.show())				// if window is visible
			wnd.draw();				// clear, draw children, swap buffers
		app.process(1000);			// parameter is timeout in millisecond
	}
	return 0;
}
```

### makefile

```makefile
# additional flags
LDFLAGS += -s

# executable name
target := hello

# space separated .c or .cpp file names without extension
sources	:= hello

include wheel/default.mk
```
