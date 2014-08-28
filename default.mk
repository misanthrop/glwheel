CXXFLAGS += -std=c++11
objs	 := $(addsuffix .o,$(sources))

.PHONY: all clean
.PRECIOUS: %.o.d
.SUFFIXES:

os := $(shell uname -s)

ifneq (,$(findstring Linux,$(os)))
	os := linux
endif

ifneq (,$(findstring MINGW,$(os)))
	os := windows
endif

allplatforms		:= linux windows android
platform			:= $(os)

all:: $(platform)

linux-arch			:= x86_64
linux-x86-CC		:= $(CC)
linux-x86-CXX		:= $(CXX)
linux-x86_64-CC		:= $(CC)
linux-x86_64-CXX	:= $(CXX)
linux-x86-FLAGS		+= -m32
linux-x86_64-FLAGS	+= -m64
linux-CPPFLAGS		+= -O2
linux-LDFLAGS		+= -lX11 -lGL
linux-x86-target	:= $(target)32
linux-x86_64-target := $(target)
linux-archive		:= $(target).tar.gz

windows-arch		  := x86
ifneq (,$(findstring windows,$(os)))
windows-x86-CC		  := $(CC)
windows-x86-CXX		  := $(CXX)
windows-x86_64-CC	  := $(CC)
windows-x86_64-CXX	  := $(CXX)
else
windows-x86-CC		  := i686-w64-mingw32-gcc
windows-x86-CXX		  := i686-w64-mingw32-g++
windows-x86_64-CC	  := x86_64-w64-mingw32-gcc
windows-x86_64-CXX	  := x86_64-w64-mingw32-g++
endif
windows-CPPFLAGS	  += -O2 -D_WIN32_WINNT=0x0500 -DUNICODE -DGLEW_STATIC
windows-LDFLAGS		  += -static -Xlinker -subsystem=windows -lglew32 -lgdi32 -lopengl32 -lwinmm
windows-x86-target	  := $(target).exe
windows-x86_64-target := $(target)64.exe
windows-archive		  := $(target).zip

android-apilevel			 := 10
android-package				 := com.glwheel
android-arch				 := x86 armeabi armeabi-v7a mips
android-x86-CC				 := $(ANDROID_NDK)/toolchains/x86-4.9/prebuilt/linux-x86_64/bin/i686-linux-android-gcc
android-x86-CXX				 := $(ANDROID_NDK)/toolchains/x86-4.9/prebuilt/linux-x86_64/bin/i686-linux-android-g++
android-armeabi-CC			 := $(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc
android-armeabi-CXX			 := $(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++
android-armeabi-v7a-CC		 := $(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-gcc
android-armeabi-v7a-CXX		 := $(ANDROID_NDK)/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-g++
android-mips-CC				 := $(ANDROID_NDK)/toolchains/mipsel-linux-android-4.9/prebuilt/linux-x86_64/bin/mipsel-linux-android-gcc
android-mips-CXX			 := $(ANDROID_NDK)/toolchains/mipsel-linux-android-4.9/prebuilt/linux-x86_64/bin/mipsel-linux-android-g++
android-FLAGS				 += -no-canonical-prefixes
android-CPPFLAGS			 += -ffunction-sections -funwind-tables -fstack-protector -fomit-frame-pointer -Wa,--noexecstack -Wformat -Werror=format-security -DANDROID
android-x86-CPPFLAGS		 += -O2 -fstrict-aliasing -finline-limit=300 -funswitch-loops -I$(ANDROID_NDK)/platforms/android-L/arch-x86/usr/include
android-armeabi-CPPFLAGS     += -Os -fno-strict-aliasing -finline-limit=64 -fpic -march=armv5te -mtune=xscale -msoft-float -mthumb -I$(ANDROID_NDK)/platforms/android-L/arch-arm/usr/include
android-armeabi-v7a-CPPFLAGS += -Os -fno-strict-aliasing -finline-limit=64 -fpic -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -mthumb -I$(ANDROID_NDK)/platforms/android-L/arch-arm/usr/include
android-mips-CPPFLAGS		 += -O2 -fno-strict-aliasing -finline-limit=300 -fpic -finline-functions -fmessage-length=0 -fno-inline-functions-called-once -fgcse-after-reload -frerun-cse-after-loop -frename-registers -funswitch-loops -I$(ANDROID_NDK)/platforms/android-L/arch-mips/usr/include
android-CXXFLAGS			 += -Wno-literal-suffix -I$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/include -fno-rtti -I$(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$2/include
android-LDFLAGS				 += -Wl,-soname,$(notdir $$@) -shared $(ANDROID_NDK)/sources/cxx-stl/gnu-libstdc++/4.9/libs/$2/libgnustl_static.a -lgcc -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -llog -landroid -lEGL -lGLESv2 -lOpenSLES -lc -lm
android-x86-LDFLAGS			 += --sysroot=$(ANDROID_NDK)/platforms/android-L/arch-x86 -L$(ANDROID_NDK)/platforms/android-L/arch-x86/usr/lib
android-armeabi-LDFLAGS		 += --sysroot=$(ANDROID_NDK)/platforms/android-L/arch-arm -L$(ANDROID_NDK)/platforms/android-L/arch-arm/usr/lib
android-armeabi-v7a-LDFLAGS	 += --sysroot=$(ANDROID_NDK)/platforms/android-L/arch-arm -L$(ANDROID_NDK)/platforms/android-L/arch-arm/usr/lib
android-mips-LDFLAGS		 += --sysroot=$(ANDROID_NDK)/platforms/android-L/arch-mips -L$(ANDROID_NDK)/platforms/android-L/arch-mips/usr/lib
android-x86-target			 := .build/android/libs/x86/lib$(target).so
android-armeabi-target		 := .build/android/libs/armeabi/lib$(target).so
android-armeabi-v7a-target	 := .build/android/libs/armeabi-v7a/lib$(target).so
android-mips-target			 := .build/android/libs/mips/lib$(target).so
android-archive				 := $(target)-debug.apk

define rules =
$1:: $($1-$2-target)

clean-$1:: clean-$1-$2

clean-$1-$2::
	$(RM) -r .build/$1/$2

.build/$1/$2/%.o: %.c
	@mkdir -p $$(@D)
	$($1-$2-CC) -MMD -MP -MF $$@.d \
		$(FLAGS) $($1-FLAGS) $($1-$2-FLAGS) \
		$(CPPFLAGS) $($1-CPPFLAGS) $($1-$2-CPPFLAGS) \
		$(CFLAGS) $($1-CFLAGS) $($1-$2-CFLAGS) -c $$< -o $$@

.build/$1/$2/%.o: %.cpp
	@mkdir -p $$(@D)
	$($1-$2-CXX) -MMD -MP -MF $$@.d \
		$(FLAGS) $($1-FLAGS) $($1-$2-FLAGS) \
		$(CPPFLAGS) $($1-CPPFLAGS) $($1-$2-CPPFLAGS) \
		$(CXXFLAGS) $($1-CXXFLAGS) $($1-$2-CXXFLAGS) -c $$< -o $$@

$($1-$2-target): $(addprefix .build/$1/$2/,$(objs))
	@mkdir -p $$(@D)
	$($1-$2-CXX) $$^ $(FLAGS) $($1-FLAGS) $($1-$2-FLAGS) $(LDFLAGS) $($1-LDFLAGS) $($1-$2-LDFLAGS) -o $$@
endef

$(foreach p,$(allplatforms), $(foreach a,$($p-arch), $(eval $(call rules,$p,$a))))

define androidmanifest
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
		package="$(android-package).$(target)"
		android:versionCode="1"
		android:versionName="1.0">
	<uses-sdk android:minSdkVersion="9" />
	<uses-feature android:glEsVersion="0x00020000" android:required="true" />
	<application android:label="$(target)" android:hasCode="false">
		<activity android:name="android.app.NativeActivity"
				android:label="$(target)"
				android:theme="@android:style/Theme.NoTitleBar.Fullscreen"
				android:configChanges="orientation|keyboardHidden">
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

define link =
$1$2: $2
	@mkdir -p $$(@D)
	ln -s $$(realpath $$^) $$@
endef

$(foreach x,$(data),$(eval $(call link,.build/android/assets/,$x)))

.build/android/AndroidManifest.xml:
	@mkdir -p $(@D)
	echo "$$androidmanifest" >$@

.build/android/default.properties:
	@mkdir -p $(@D)
	@echo "target=android-$(android-apilevel)" >$@

.build/android/build.xml: .build/android/AndroidManifest.xml .build/android/default.properties
	android update project -p .build/android -n $(target) -s

$(windows-archive): windows
	zip -r $@ $(foreach a,$(windows-arch),$(windows-$a-target)) $(data)

$(android-archive): .build/android/build.xml $(addprefix .build/android/assets/,$(data)) android
	ant debug -f $<
	@mv .build/android/bin/$@ $@

install-linux: linux

install-windows: windows

install-android: $(android-archive)
	adb install -r $^

clean:: $(addprefix clean-,$(platform))

install:: $(addprefix install-,$(platform))

rwildcard=$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
-include $(call rwildcard, .build/, *.o.d)
