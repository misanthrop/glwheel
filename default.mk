CXXFLAGS += -std=c++11

os := $(shell uname -s)

ifneq (,$(findstring MINGW,$(os)))
	os := Windows
endif

ifeq ($(os),Linux)
libs	 += GL X11
else ifeq ($(os),Android)
$(shell mkdir -p libs/armeabi)
all:: $(target).apk

target	 := ./libs/armeabi/lib$(target).so
libs	 += gcc log android EGL GLESv1_CM c m
# /opt/android-ndk/sources/android/native_app_glue/
sources  += android_native_app_glue
CC		 := /opt/android-ndk/toolchains/llvm-3.2/prebuilt/linux-x86_64/bin/clang
CXX		 := /opt/android-ndk/toolchains/llvm-3.2/prebuilt/linux-x86_64/bin/clang++

FLAGS	 += -gcc-toolchain /opt/android-ndk/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64
FLAGS	 += -no-canonical-prefixes -target armv5te-none-linux-androideabi
CPPFLAGS += -fpic -ffunction-sections -funwind-tables -fstack-protector
CPPFLAGS += -march=armv5te -mtune=xscale -msoft-float -mthumb
CPPFLAGS += -DANDROID -DNDEBUG -fomit-frame-pointer -fno-strict-aliasing -Wa,--noexecstack
# CPPFLAGS += -I/opt/android-ndk/sources/android/native_app_glue
CPPFLAGS += -I/opt/android-ndk/platforms/android-14/arch-arm/usr/include

CXXFLAGS += -fno-exceptions -fno-rtti
CXXFLAGS += -I/opt/android-ndk/sources/cxx-stl/gnu-libstdc++/4.6/include
CXXFLAGS += -I/opt/android-ndk/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi/include

LDFLAGS	 += -shared --sysroot=/opt/android-ndk/platforms/android-14/arch-arm
LDFLAGS	 += /opt/android-ndk/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi/libgnustl_static.a
LDFLAGS	 += -Wl,-soname,lib$(target).so -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now
LDFLAGS	 += -L/opt/android-ndk/platforms/android-14/arch-arm/usr/lib
else ifeq ($(os),Windows)
target	 := $(target).exe
libs	 += glew32 gdi32 opengl32
CXXFLAGS += -D_WIN32_WINNT=0x0500 -DUNICODE
LDFLAGS  += -Xlinker -subsystem=windows
endif

ifeq ($(CDEBUG),-g)
CXXFLAGS := $(filter-out -O1 -O2 -O3 -Os -s, $(CXXFLAGS))
LDFLAGS  := $(filter-out -O1 -O2 -O3 -Os -s, $(LDFLAGS))
endif

objs	:= $(addsuffix .o,$(sources))
depends	:= $(addsuffix .d,$(objs))

.PHONY: all clean
.PRECIOUS: %.dp
.SUFFIXES:

all:: $(target)

clean::
	$(RM) $(objs) $(depends) $(target)

%.o: %.c
	$(CC) -MMD -MP -MF $@.d $(CDEBUG) $(FLAGS) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) -MMD -MP -MF $@.d $(CDEBUG) $(FLAGS) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.apk: AndroidManifest.xml
	/opt/android-sdk/build-tools/17.0.0/aapt package --no-crunch -f --debug-mode -M $< -I /opt/android-sdk/platforms/android-17/android.jar -F $@

-include $(depends)

$(target): $(objs)
	$(CXX) $(objs) $(CDEBUG) $(FLAGS) $(LDFLAGS) $(addprefix -l,$(libs)) -o $(target)
