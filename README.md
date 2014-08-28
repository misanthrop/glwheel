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
#include "impl.hpp"
using namespace wheel;
int main()
{
    app.init(u8"hello");         // u8 is optional and shows possibility to use Unicode
    app.show(true);
    while(app.alive())
    {
        if(app)                  // if application is active
        {
            app.update();        // updates parameters if needed
            app.draw();          // clear, draw children, swap buffers
            app.process(100);    // parameter is timeout in milliseconds
        }
        else app.process();      // default value is -1, it means wait for next event
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

include glwheel/default.mk
```
