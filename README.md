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

### Build instructions on Linux host

Build for Linux using default compiler:
```
make
```
Build for Windows. Requires cross-compiler *i686-w64-mingw32-g++*, tested on Arch Linux:
```
make windows
```
Build for Android. Requires [Android NDK](https://developer.android.com/tools/sdk/ndk/index.html) r10. ANDROID_NDK environment variable should be set properly:
```
make install-android
```
### Build instructions on Windows host

Requires [MinGW](http://www.mingw.org) and [GLEW](http://glew.sourceforge.net). Suggested compiler distribution is [Nuwen MinGW](http://nuwen.net/mingw.html). It already includes [GLEW](http://glew.sourceforge.net) library. Build should be run from Bash-shell (e.g. *git-bash*), *rm* and *uname* tools should be accessible in $PATH:
```
make
```
