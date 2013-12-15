CXXFLAGS += -std=c++11

os := $(shell uname -s)

ifneq (,$(findstring MINGW,$(os)))
	os := Windows
endif

ifeq ($(os),Linux)
libs	 += GL X11
else ifeq ($(os),Android)
# $(shell mkdir -p libs/armeabi)
$(shell mkdir -p libs/x86)
# all:: bin/Example-debug.apk

libs	 += gcc log android EGL GLESv1_CM c m

CC		 := /opt/android-ndk/toolchains/x86-4.7/prebuilt/linux-x86_64/bin/i686-linux-android-gcc
CXX		 := /opt/android-ndk/toolchains/x86-4.7/prebuilt/linux-x86_64/bin/i686-linux-android-g++

# FLAGS	 += -gcc-toolchain /opt/android-ndk/toolchains/arm-linux-androideabi-4.6/prebuilt/linux-x86_64
# FLAGS	 += -gcc-toolchain /opt/android-ndk/toolchains/x86-4.7/prebuilt/linux-x86_64
# FLAGS	 += -no-canonical-prefixes -target armv5te-none-linux-androideabi
FLAGS	 += -no-canonical-prefixes
# -target i686-none-linux-android
CPPFLAGS += -ffunction-sections -funwind-tables -fstack-protector -Wa,--noexecstack
# CPPFLAGS += -march=armv5te -mtune=xscale -msoft-float -mthumb
# -DNDEBUG
CPPFLAGS += -DANDROID
CPPFLAGS += -fomit-frame-pointer -fstrict-aliasing -funswitch-loops -finline-limit=300
# CPPFLAGS += -I/opt/android-ndk/sources/android/native_app_glue
# CPPFLAGS += -I/opt/android-ndk/platforms/android-14/arch-arm/usr/include
CPPFLAGS += -I/opt/android-ndk/platforms/android-9/arch-x86/usr/include

CXXFLAGS += -fno-exceptions -fno-rtti
CXXFLAGS += -I/opt/android-ndk/sources/cxx-stl/gnu-libstdc++/4.7/include
# CXXFLAGS += -I/opt/android-ndk/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi/include
CXXFLAGS += -I/opt/android-ndk/sources/cxx-stl/gnu-libstdc++/4.7/libs/x86/include

# LDFLAGS	 += -shared --sysroot=/opt/android-ndk/platforms/android-14/arch-arm
LDFLAGS	 += -shared --sysroot=/opt/android-ndk/platforms/android-9/arch-x86
# LDFLAGS	 += /opt/android-ndk/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi/libgnustl_static.a
LDFLAGS	 += /opt/android-ndk/sources/cxx-stl/gnu-libstdc++/4.7/libs/x86/libgnustl_static.a
LDFLAGS	 += -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now
# LDFLAGS	 += -L/opt/android-ndk/platforms/android-14/arch-arm/usr/lib
LDFLAGS	 += -L/opt/android-ndk/platforms/android-9/arch-x86/usr/lib
target	 := ./libs/x86/lib$(target).so
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

%.so: $(objs)
	$(CXX) $(objs) $(CDEBUG) -shared -fpic -Wl,-soname,$(notdir $@) $(FLAGS) $(LDFLAGS) $(addprefix -l,$(libs)) -o $@

$(target): $(objs)
	$(CXX) $(objs) $(CDEBUG) $(FLAGS) $(LDFLAGS) $(addprefix -l,$(libs)) -o $(target)

%.apk: AndroidManifest.xml libs/x86/libexample.so
	ant debug
#	/opt/android-sdk/build-tools/17.0.0/aapt package --no-crunch -f --debug-mode -M $< -I /opt/android-sdk/platforms/android-17/android.jar -F $@

-include $(depends)

