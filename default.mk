glwheel  ?= $(dir $(lastword $(MAKEFILE_LIST)))
CXXFLAGS += -std=c++11
objs	 := $(addsuffix .o,$(sources))

.PHONY: all clean
.PRECIOUS: %.o.d
.SUFFIXES:

os := $(shell uname -s)
arch := $(shell uname -m)

ifneq (,$(findstring Linux,$(os)))
	os := linux
endif

ifneq (,$(findstring MINGW,$(os)))
	os := windows
endif

allplatforms		:= linux windows android wayland
platform			:= $(os)
name				?= $(target)
build				?= 1
version				?= 1.0

all:: $(platform)-$(arch)

linux-arch			:= i686 x86_64
linux-i686-CC		:= $(CC)
linux-i686-CXX		:= $(CXX)
linux-x86_64-CC		:= $(CC)
linux-x86_64-CXX	:= $(CXX)
linux-i686-FLAGS	+= -m32
linux-x86_64-FLAGS	+= -m64
linux-CPPFLAGS		+= -O2
linux-LDFLAGS		+= -s -lX11 -lGL
linux-i686-target	:= $(target).32
linux-x86_64-target := $(target)
linux-archive		:= $(target).tar.xz

windows-arch		  := i686 x86_64
ifneq (,$(findstring windows,$(os)))
windows-i686-CC		  := $(CC)
windows-i686-CXX	  := $(CXX)
windows-x86_64-CC	  := $(CC)
windows-x86_64-CXX	  := $(CXX)
else
windows-i686-CC		  := i686-w64-mingw32-gcc
windows-i686-CXX	  := i686-w64-mingw32-g++
windows-x86_64-CC	  := x86_64-w64-mingw32-gcc
windows-x86_64-CXX	  := x86_64-w64-mingw32-g++
endif
windows-CPPFLAGS	  += -O2 -D_WIN32_WINNT=0x0500 -DUNICODE -DGLEW_STATIC
windows-LDFLAGS		  += -static -Xlinker -subsystem=windows -lglew32 -lgdi32 -lopengl32 -lwinmm
windows-i686-target	  := $(target).exe
windows-x86_64-target := $(target).64.exe
windows-archive		  := $(target).zip

android-apilevel			 ?= 10
android-package				 ?= com.wheellabs.$(target)
android-arch				 := x86 armeabi armeabi-v7a mips
# comming soon x86_64 arm64-v8a mips64
android-CC					 := $(ANDROID_NDK)/toolchains/llvm-3.5/prebuilt/linux-x86_64/bin/clang
android-CXX					 := $(ANDROID_NDK)/toolchains/llvm-3.5/prebuilt/linux-x86_64/bin/clang++
android-x86-CC				 := $(android-CC)
android-x86-CXX				 := $(android-CXX)
android-armeabi-CC			 := $(android-CC)
android-armeabi-CXX			 := $(android-CXX)
android-armeabi-v7a-CC		 := $(android-CC)
android-armeabi-v7a-CXX		 := $(android-CXX)
android-mips-CC				 := $(android-CC)
android-mips-CXX			 := $(android-CXX)
android-FLAGS				 += -no-canonical-prefixes
android-CPPFLAGS			 += -ffunction-sections -funwind-tables -fstack-protector -fomit-frame-pointer -Wa,--noexecstack -Wformat -Werror=format-security -DANDROID
android-x86-FLAGS			 += -gcc-toolchain $(ANDROID_NDK)/toolchains/x86-4.8/prebuilt/linux-x86_64 -target i686-none-linux-android
android-armeabi-FLAGS		 += -gcc-toolchain $(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64 -target armv5te-none-linux-androideabi
android-armeabi-v7a-FLAGS	 += -gcc-toolchain $(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.8/prebuilt/linux-x86_64 -target armv7-none-linux-androideabi
android-mips-FLAGS			 += -gcc-toolchain $(ANDROID_NDK)/toolchains/mipsel-linux-android-4.8/prebuilt/linux-x86_64 -target mipsel-none-linux-android
android-x86-CPPFLAGS		 += -O2 -fstrict-aliasing -fPIC -fstack-protector -I$(ANDROID_NDK)/platforms/android-9/arch-x86/usr/include
android-armeabi-CPPFLAGS     += -Os -march=armv5te -mtune=xscale -msoft-float -mthumb -fno-strict-aliasing -fpic -fstack-protector -fno-integrated-as -I$(ANDROID_NDK)/platforms/android-9/arch-arm/usr/include
android-armeabi-v7a-CPPFLAGS += -Os -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -mthumb -fno-strict-aliasing -fpic -fstack-protector -fno-integrated-as -I$(ANDROID_NDK)/platforms/android-9/arch-arm/usr/include
android-mips-CPPFLAGS		 += -O2 -fmessage-length=0 -fno-strict-aliasing -fpic -I$(ANDROID_NDK)/platforms/android-9/arch-mips/usr/include
android-CXXFLAGS			 += -I$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/include -I$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/libs/$2/include -I$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/include/backward
android-LDFLAGS				 += -s -Wl,-soname,$(notdir $$@) -shared $(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.8/libs/$2/libgnustl_static.a -lgcc -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -lc -lm -llog -latomic -landroid -ljnigraphics -lEGL -lGLESv2 -lOpenSLES
android-x86-LDFLAGS			 += --sysroot=$(ANDROID_NDK)/platforms/android-14/arch-x86 -L$(ANDROID_NDK)/platforms/android-14/arch-x86/usr/lib
android-armeabi-LDFLAGS		 += --sysroot=$(ANDROID_NDK)/platforms/android-14/arch-arm -L$(ANDROID_NDK)/platforms/android-14/arch-arm/usr/lib
android-armeabi-v7a-LDFLAGS	 += --sysroot=$(ANDROID_NDK)/platforms/android-14/arch-arm -L$(ANDROID_NDK)/platforms/android-14/arch-arm/usr/lib
android-mips-LDFLAGS		 += --sysroot=$(ANDROID_NDK)/platforms/android-14/arch-mips -L$(ANDROID_NDK)/platforms/android-14/arch-mips/usr/lib
android-x86-target			 := .build/android/bin/lib/x86/lib$(target).so
android-armeabi-target		 := .build/android/bin/lib/armeabi/lib$(target).so
android-armeabi-v7a-target	 := .build/android/bin/lib/armeabi-v7a/lib$(target).so
android-mips-target			 := .build/android/bin/lib/mips/lib$(target).so
android-archive				 := $(target).apk

define rules
build-$1:: $1-$2

$1-$2:: $($1-$2-target)

archive-$1:: $($1-archive)

clean-$1:: clean-$1-$2
	$(RM) -r .build/$1

clean-$1-$2::
	$(RM) -r .build/$1/obj/$2

ifdef debug
$1-$2-cflags = -g $(filter-out -Os -O1 -O2 -O3 -O4 -O5, $(FLAGS) $($1-FLAGS) $($1-$2-FLAGS) $(CPPFLAGS) $($1-CPPFLAGS) $($1-$2-CPPFLAGS) $(CFLAGS) $($1-CFLAGS) $($1-$2-CFLAGS))
$1-$2-cxxflags = -g $(filter-out -Os -O1 -O2 -O3 -O4 -O5, $(FLAGS) $($1-FLAGS) $($1-$2-FLAGS) $(CPPFLAGS) $($1-CPPFLAGS) $($1-$2-CPPFLAGS) $(CXXFLAGS) $($1-CXXFLAGS) $($1-$2-CXXFLAGS))
$1-$2-ldflags =  $(filter-out -s, $(FLAGS) $($1-FLAGS) $($1-$2-FLAGS) $(LDFLAGS) $($1-LDFLAGS) $($1-$2-LDFLAGS))
else
$1-$2-cflags = -DNDEBUG $(FLAGS) $($1-FLAGS) $($1-$2-FLAGS) $(CPPFLAGS) $($1-CPPFLAGS) $($1-$2-CPPFLAGS) $(CFLAGS) $($1-CFLAGS) $($1-$2-CFLAGS)
$1-$2-cxxflags = -DNDEBUG $(FLAGS) $($1-FLAGS) $($1-$2-FLAGS) $(CXXFLAGS) $($1-CXXFLAGS) $($1-$2-CXXFLAGS) $(CPPFLAGS) $($1-CPPFLAGS) $($1-$2-CPPFLAGS)
$1-$2-ldflags = $(FLAGS) $($1-FLAGS) $($1-$2-FLAGS) $(LDFLAGS) $($1-LDFLAGS) $($1-$2-LDFLAGS)
endif

.build/$1/obj/$2/%.o: %.c
	@mkdir -p $$(@D)
	$($1-$2-CC) -MMD -MP -MF $$@.d $$($1-$2-cflags) -c $$< -o $$@

.build/$1/obj/$2/%.o: %.cpp
	@mkdir -p $$(@D)
	$($1-$2-CXX) -MMD -MP -MF $$@.d $$($1-$2-cxxflags) -c $$< -o $$@

$($1-$2-target): $(addprefix .build/$1/obj/$2/,$(objs))
	@mkdir -p $$(@D)
	$($1-$2-CXX) $$^ $$($1-$2-ldflags) -o $$@
endef

$(foreach p,$(allplatforms), $(foreach a,$($p-arch), $(eval $(call rules,$p,$a))))

define androidmanifest
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
		package="$(android-package)" android:versionCode="$(build)" android:versionName="$(version)">
	<uses-sdk android:minSdkVersion="15" />
	<uses-feature android:glEsVersion="0x00020000" android:required="true" />
	<application android:label="$(name)" android:icon="@drawable/icon">
		<activity android:name="com.wheel.WheelActivity"
				android:screenOrientation="landscape"
				android:icon="@drawable/icon"
				android:theme="@android:style/Theme.NoTitleBar.Fullscreen"
				android:configChanges="orientation|screenSize|keyboardHidden">
			<meta-data android:name="android.app.lib_name" android:value="$(target)" />
			<intent-filter>
				<action android:name="android.intent.action.MAIN" />
				<category android:name="android.intent.category.LAUNCHER" />
			</intent-filter>
		</activity>
	</application>
</manifest>
endef
export androidmanifest

define link
$1$2: $2
	@mkdir -p $$(@D)
	ln -s $$(realpath $$^) $$@
endef

$(foreach x,$(data),$(eval $(call link,.build/android/assets/,$x)))

$(eval $(call link,.build/android/res/drawable/,icon.png))

.build/android/AndroidManifest.xml: $(glwheel)default.mk
	@mkdir -p $(@D)
	@echo Generating manifest
	echo "$$androidmanifest" >$@

.build/android/cls/com/wheel/WheelActivity.class: $(glwheel)WheelActivity.java
	@mkdir -p $(@D)
	javac -d .build/android/cls -classpath .build/android/cls -bootclasspath /opt/android-sdk/platforms/android-15/android.jar -sourcepath $(glwheel) -target 1.5 -source 1.5 $^

.build/android/bin/classes.dex: .build/android/cls/com/wheel/WheelActivity.class
	@mkdir -p $(@D)
	$(ANDROID_HOME)/build-tools/21.1.2/dx --dex --output $@ .build/android/cls /opt/android-sdk/tools/support/annotations.jar

.build/android/$(target)-unsigned.apk: build-android .build/android/AndroidManifest.xml .build/android/res/drawable/icon.png $(addprefix .build/android/assets/,$(data)) .build/android/bin/classes.dex
	@mkdir -p $(@D)/assets
	$(ANDROID_HOME)/build-tools/21.1.2/aapt p -f -M $(@D)/AndroidManifest.xml -S $(@D)/res -A $(@D)/assets -I /opt/android-sdk/platforms/android-15/android.jar -F $@ $(@D)/bin

$(target)-debug.apk: .build/android/$(target)-unsigned.apk
	jarsigner -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/.android/debug.keystore -storepass android $< androiddebugkey
	$(ANDROID_HOME)/build-tools/21.1.2/zipalign -f 4 $< $@

$(target).apk: .build/android/$(target)-unsigned.apk
	jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore ~/fun/.keystore $< $(target)
	$(ANDROID_HOME)/build-tools/21.1.2/zipalign -f 4 $< $@

$(windows-archive): windows
	zip -r $@ $(foreach a,$(windows-arch),$(windows-$a-target)) $(data)

linux: build-linux

windows: build-windows

android: $(target)-debug.apk

install-linux: linux

install-windows: windows

install-android: $(target)-debug.apk
	adb install -r $^

clean:: $(addprefix clean-,$(platform))

clean-all:
	$(RM) -r .build

install:: $(addprefix install-,$(platform))

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
-include $(call rwildcard, .build/, *.o.d)
