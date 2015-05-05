#pragma once
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <string>
#include <queue>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <condition_variable>
#include <sched.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>
#include <android/bitmap.h>
#include "log.h"
#include "app.hpp"

extern int main();

namespace wheel
{
	using namespace std;

	application *app = 0;

	namespace events
	{
		int n = 2;
		int fds[64]; // in honor of Windows
		function<void()> fns[64];

		void add(int fd, function<void()>);
		void remove(int fd);
	}

	namespace android
	{
		constexpr const key key[] =
		{
			key::unknown,
			key::lsuper,		// AKEYCODE_SOFT_LEFT       = 1,
			key::rsuper,		// AKEYCODE_SOFT_RIGHT      = 2,
			key::home,			// AKEYCODE_HOME            = 3,
			key::esc,			// AKEYCODE_BACK            = 4,
			key::call,			// AKEYCODE_CALL            = 5,
			key::endcall,		// AKEYCODE_ENDCALL         = 6,
			key::n0, key::n1, key::n2, key::n3, key::n4, key::n5, key::n6, key::n7, key::n8, key::n9,	// AKEYCODE_0 = 7 .. AKEYCODE_9 = 16,
			key::star,			// AKEYCODE_STAR            = 17,
			key::pound,			// AKEYCODE_POUND           = 18,
			key::up,			// AKEYCODE_DPAD_UP         = 19,
			key::down,			// AKEYCODE_DPAD_DOWN       = 20,
			key::left,			// AKEYCODE_DPAD_LEFT       = 21,
			key::right,			// AKEYCODE_DPAD_RIGHT      = 22,
			key::center,		// AKEYCODE_DPAD_CENTER     = 23,
			key::volumeup,		// AKEYCODE_VOLUME_UP       = 24,
			key::volumedown,	// AKEYCODE_VOLUME_DOWN     = 25,
			key::power,			// AKEYCODE_POWER           = 26,
			key::camera,		// AKEYCODE_CAMERA          = 27,
			key::clear,			// AKEYCODE_CLEAR           = 28,
			key::a, key::b, key::c, key::d, key::e, key::f, key::g, key::h, key::i, key::j, key::k, key::l, key::m,	// AKEYCODE_A = 29,
			key::n, key::o, key::p, key::q, key::r, key::s, key::t, key::u, key::v, key::w, key::x, key::y, key::z,	// AKEYCODE_Z = 54,
			key::comma,			// AKEYCODE_COMMA           = 55,
			key::period,		// AKEYCODE_PERIOD          = 56,
			key::lalt,			// AKEYCODE_ALT_LEFT        = 57,
			key::ralt,			// AKEYCODE_ALT_RIGHT       = 58,
			key::lshift,		// AKEYCODE_SHIFT_LEFT      = 59,
			key::rshift,		// AKEYCODE_SHIFT_RIGHT     = 60,
			key::tab,			// AKEYCODE_TAB             = 61,
			key::space,			// AKEYCODE_SPACE           = 62,
			key::sym,			// AKEYCODE_SYM             = 63,
			key::explorer,		// AKEYCODE_EXPLORER        = 64,
			key::envelope,		// AKEYCODE_ENVELOPE        = 65,
			key::enter,			// AKEYCODE_ENTER           = 66,
			key::del,			// AKEYCODE_DEL             = 67,
			key::grave,			// AKEYCODE_GRAVE           = 68,
			key::minus,			// AKEYCODE_MINUS           = 69,
			key::equals,		// AKEYCODE_EQUALS          = 70,
			key::lbracket,		// AKEYCODE_LEFT_BRACKET    = 71,
			key::rbracket,		// AKEYCODE_RIGHT_BRACKET   = 72,
			key::backslash,		// AKEYCODE_BACKSLASH       = 73,
			key::semicolon,		// AKEYCODE_SEMICOLON       = 74,
			key::apostrophe,	// AKEYCODE_APOSTROPHE      = 75,
			key::slash,			// AKEYCODE_SLASH           = 76,
			key::at,			// AKEYCODE_AT              = 77,
			key::num,			// AKEYCODE_NUM             = 78,
			key::headset,		// AKEYCODE_HEADSETHOOK     = 79,
			key::focus,			// AKEYCODE_FOCUS           = 80,   // *Camera* focus
			key::plus,			// AKEYCODE_PLUS            = 81,
			key::menu,			// AKEYCODE_MENU            = 82,
			key::notify,		// AKEYCODE_NOTIFICATION    = 83,
			key::search,		// AKEYCODE_SEARCH          = 84,
			key::play,			// AKEYCODE_MEDIA_PLAY_PAUSE= 85,
			key::stop,			// AKEYCODE_MEDIA_STOP      = 86,
			key::next,			// AKEYCODE_MEDIA_NEXT      = 87,
			key::prev,			// AKEYCODE_MEDIA_PREVIOUS  = 88,
			key::rewind,		// AKEYCODE_MEDIA_REWIND    = 89,
			key::forward,		// AKEYCODE_MEDIA_FAST_FORWARD = 90,
			key::mute,			// AKEYCODE_MUTE            = 91,
			key::pageup,		// AKEYCODE_PAGE_UP         = 92,
			key::pagedown,		// AKEYCODE_PAGE_DOWN       = 93,
			key::picsym,		// AKEYCODE_PICTSYMBOLS     = 94,
			key::switchcharset,	// AKEYCODE_SWITCH_CHARSET  = 95,
			key::ajoy,			// AKEYCODE_BUTTON_A        = 96,
			key::bjoy,			// AKEYCODE_BUTTON_B        = 97,
			key::cjoy,			// AKEYCODE_BUTTON_C        = 98,
			key::xjoy,			// AKEYCODE_BUTTON_X        = 99,
			key::yjoy,			// AKEYCODE_BUTTON_Y        = 100,
			key::zjoy,			// AKEYCODE_BUTTON_Z        = 101,
			key::l1joy,			// AKEYCODE_BUTTON_L1       = 102,
			key::r1joy,			// AKEYCODE_BUTTON_R1       = 103,
			key::l2joy,			// AKEYCODE_BUTTON_L2       = 104,
			key::r2joy,			// AKEYCODE_BUTTON_R2       = 105,
			key::lthumb,		// AKEYCODE_BUTTON_THUMBL   = 106,
			key::rthumb,		// AKEYCODE_BUTTON_THUMBR   = 107,
			key::start,			// AKEYCODE_BUTTON_START    = 108,
			key::select,		// AKEYCODE_BUTTON_SELECT   = 109,
			key::mode,			// AKEYCODE_BUTTON_MODE     = 110,
		};

		enum class cmd : uint8_t { setinput, setwindow, focus, unfocus, cfgchanged, lowmem, resume, stop, destroy, nocmd = 0xff };

		ANativeActivity *act;
		std::mutex mutex;
		std::condition_variable cond;
		int msgpipe[2];
		bool running, destroyed;
		AInputQueue* pendingInputQueue = 0;
		ANativeWindow* pendingWindow = 0;
		ALooper *looper = 0;

		AConfiguration *config = 0;
		AInputQueue *inputQueue = 0;
		ANativeWindow *window = 0;

		ASensorManager *sensorManager = 0;
		const ASensor *accelerometer = 0;
		ASensorEventQueue *sensorEventQueue = 0;
		EGLDisplay dpy = EGL_NO_DISPLAY;
		EGLContext context = EGL_NO_CONTEXT;
		EGLSurface surface;
		EGLConfig glconfig;
		size_t mn[2] = {0,0};
		bool alive, visible;
		int orientation;

		JNIEnv *env; // native thread (C++ side)
		struct { jclass cls; jmethodID moveToBack, fullscreen, screenOrientation; } WheelActivity;

		cmd readcmd()
		{
			cmd c;
			if(read(msgpipe[0], &c, sizeof(c)) == sizeof(c)) return c;
			logerr("No data on command pipe!");
			return cmd::nocmd;
		}

		void writecmd(cmd c) { if(write(msgpipe[1], &c, sizeof(c)) != sizeof(c)) logerr("write failed: %s", strerror(errno)); }

		void create(ANativeActivity *a)
		{
			logdbg("create()");
			act = a;
			alive = 1;
			orientation = 0;
			mn[0] = mn[1] = 0;
			running = 0;
			destroyed = 0;
			pendingInputQueue = 0;
			pendingWindow = 0;
			if(pipe(msgpipe)) { logerr("could not create pipe %s", strerror(errno)); return; }
			std::thread(main).detach();
			std::unique_lock<std::mutex> lock(mutex);
			cond.wait(lock, []{ return running; });
		}

		void destroy()
		{
			logdbg("destroy()");
			{
				std::unique_lock<std::mutex> lock(mutex);
				writecmd(cmd::destroy);
				cond.wait(lock, []{ return destroyed; });
			}
			close(msgpipe[0]);
			close(msgpipe[1]);
		}

		void setinput(AInputQueue* iq)
		{
			std::unique_lock<std::mutex> lock(mutex);
			pendingInputQueue = iq;
			writecmd(cmd::setinput);
			cond.wait(lock, [iq]{ return inputQueue == iq; });
		}

		void setwindow(ANativeWindow* w)
		{
			std::unique_lock<std::mutex> lock(mutex);
			pendingWindow = w;
			writecmd(cmd::setwindow);
			cond.wait(lock, [w]{ return window == w; });
		}

		void stop()
		{
			logdbg("stop()");
			std::unique_lock<std::mutex> lock(mutex);
			writecmd(cmd::stop);
		}

		void processinput()
		{
			AInputEvent* event = 0;
			if(AInputQueue_getEvent(inputQueue, &event) >= 0)
			{
				if(AInputQueue_preDispatchEvent(inputQueue, event)) return;
				bool processed = 0;
				switch(AInputEvent_getType(event))
				{
					case AINPUT_EVENT_TYPE_KEY:
					{
						wheel::key k = android::key[AKeyEvent_getKeyCode(event)];
						switch(AKeyEvent_getAction(event))
						{
							case AKEY_EVENT_ACTION_DOWN: *k = true; if(app->pressed) app->pressed(k); break;
							case AKEY_EVENT_ACTION_UP: *k = false; if(app->released) app->released(k); break;
						}
						processed = 1;
						break;
					}

					case AINPUT_EVENT_TYPE_MOTION:
					{
						int prevcount = app->pointercount;
						pointer prevpointers[32];
						memcpy(prevpointers, app->pointers, sizeof(app->pointers));
						app->pointercount = std::min((size_t)32, AMotionEvent_getPointerCount(event));
						for(size_t i = 0; i < app->pointercount; ++i) app->pointers[i] = { AMotionEvent_getX(event, i), AMotionEvent_getY(event, i) };
						if(app->pointermoved) app->pointermoved();
						if(app->pointercount == 1)
						{
							switch(AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK)
							{
								case AMOTION_EVENT_ACTION_DOWN:
									if(!*key::lbutton)
									{
										*key::lbutton = true;
										if(app->pressed) app->pressed(key::lbutton);
									}
									break;
								case AMOTION_EVENT_ACTION_UP:
									*key::lbutton = false;
									if(app->released) app->released(key::lbutton);
									app->pointercount = 0;
									break;
							}
						}
						if(app->pointercount == 2 && prevcount == 2)
						{
							int dx = app->pointers[0].x - app->pointers[1].x;
							int dy = app->pointers[0].y - app->pointers[1].y;
							int pdx = prevpointers[0].x - prevpointers[1].x;
							int pdy = prevpointers[0].y - prevpointers[1].y;
							float diff2 = pdx*pdx + pdy*pdy - dx*dx - dy*dy;
							float absdiff = sqrt(abs(diff2));
							if(absdiff > 16) if(app->scrolled) app->scrolled(diff2*4/absdiff/std::min(app->width, app->height));
						}
						processed = 1;
						break;
					}
				}
				AInputQueue_finishEvent(inputQueue, event, processed);
			}
			else logerr("Failure reading next input event: %s", strerror(errno));
		}

		void processsensor()
		{
			if(accelerometer)
			{
				ASensorEvent event;
				while(ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
					if(app->accel) app->accel(event.acceleration.x, event.acceleration.y, event.acceleration.z);
				float magnitude = event.acceleration.x*event.acceleration.x + event.acceleration.y*event.acceleration.y;
				if(magnitude*4 > event.acceleration.z*event.acceleration.z)
				{
					const float pi = 3.14159265358979;
					float angle = atan2(event.acceleration.y, event.acceleration.x);
					if(abs(angle) < 0.5) orientation = 0;
					if(abs(angle - pi*0.5) < 0.5) orientation = 1;
					if(pi - abs(angle) < 0.5) orientation = 2;
					if(abs(angle + pi*0.5) < 0.5) orientation = 3;
				}
			}
		}

		void createsurface()
		{
			logdbg("createsurface()");
			EGLint format;
			eglGetConfigAttrib(dpy, glconfig, EGL_NATIVE_VISUAL_ID, &format);
			ANativeWindow_setBuffersGeometry(window, 0, 0, format);

			surface = eglCreateWindowSurface(dpy, glconfig, window, 0);
			const EGLint attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
			context = eglCreateContext(dpy, glconfig, 0, attribs);

			eglMakeCurrent(dpy, surface, surface, context);
			loginf("%s", (const char*)glGetString(GL_VERSION));
			visible = 1;
			if(app->loaded) app->loaded();
		}

		void clearsurface()
		{
			logdbg("clearsurface()");
			visible = 0;
			if(app->cleared) app->cleared();
			eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroySurface(dpy, surface);
			if(context != EGL_NO_CONTEXT) eglDestroyContext(dpy, context);
			context = EGL_NO_CONTEXT;
		}

		void clear()
		{
			logdbg("clear()");
			if(sensorEventQueue) ASensorManager_destroyEventQueue(sensorManager, sensorEventQueue);
			std::unique_lock<std::mutex> lock(mutex);
			if(dpy != EGL_NO_DISPLAY) { eglTerminate(dpy); dpy = EGL_NO_DISPLAY; }
			if(inputQueue) { AInputQueue_detachLooper(inputQueue); inputQueue = 0; }
			if(config) { AConfiguration_delete(config); config = 0; }
			env->DeleteGlobalRef(WheelActivity.cls);
			act->vm->DetachCurrentThread();
			destroyed = 1;
			cond.notify_all();
		}

		cmd processcmd()
		{
			cmd c = readcmd();
			switch(c)
			{
				case cmd::setinput:
				{
					std::unique_lock<std::mutex> lock(mutex);
					if(inputQueue)
					{
						AInputQueue_detachLooper(inputQueue);
						events::fns[0] = 0;
					}
					if((inputQueue = pendingInputQueue))
					{
						events::fns[0] = processinput;
						AInputQueue_attachLooper(inputQueue, looper, 0, 0, 0);
					}
					cond.notify_all();
					break;
				}

				case cmd::setwindow:
				{
					std::unique_lock<std::mutex> lock(mutex);
					if(window) clearsurface();
					if((window = pendingWindow)) createsurface();
					cond.notify_all();
				}

				case cmd::resume:
					if(context != EGL_NO_CONTEXT) visible = 1;
					if(app->resumed) app->resumed();
					break;

				case cmd::stop:
					visible = 0;
					if(app->paused) app->paused();
					break;

				case cmd::focus:
					if(accelerometer)
					{
						ASensorEventQueue_enableSensor(sensorEventQueue, accelerometer);
						ASensorEventQueue_setEventRate(sensorEventQueue, accelerometer, (1000L/60)*1000);
					}
					break;

				case cmd::unfocus:
//					app->visible = 0;
					if(accelerometer) ASensorEventQueue_disableSensor(sensorEventQueue, accelerometer);
					break;

				case cmd::cfgchanged:
					AConfiguration_fromAssetManager(config, act->assetManager);
					break;

				case cmd::destroy:
					alive = 0;
					break;

				default: break;
			}
			return c;
		}

		void init() // native thread (C++ side)
		{
			logdbg("init()");
			config = AConfiguration_new();
			looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
			dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

			act->vm->AttachCurrentThread(&env,0);
			WheelActivity.moveToBack = env->GetMethodID(WheelActivity.cls, "moveToBack", "()V");
			WheelActivity.fullscreen = env->GetMethodID(WheelActivity.cls, "fullscreen", "()V");
			WheelActivity.screenOrientation = env->GetMethodID(WheelActivity.cls, "screenOrientation", "()I");

			android::act->instance = &app;
			AConfiguration_fromAssetManager(config, act->assetManager);

			events::add(msgpipe[0], [&]() { processcmd(); });

			eglInitialize(dpy, 0, 0);
			const EGLint attribs[] = { EGL_RED_SIZE,8, EGL_GREEN_SIZE,8, EGL_BLUE_SIZE,8, EGL_ALPHA_SIZE,8, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE };

			EGLint numConfigs;
			eglChooseConfig(dpy, attribs, &glconfig, 1, &numConfigs);

			{
				std::unique_lock<std::mutex> lock(mutex);
				running = 1;
				cond.notify_all();
			}

			while(!window) processcmd();

			sensorManager = ASensorManager_getInstance();
			accelerometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
			sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, 1, 0, 0);
			events::fns[1] = processsensor;
		}

		void movetoback() { env->CallVoidMethod(act->clazz, WheelActivity.moveToBack); }
		void fullscreen() { env->CallVoidMethod(act->clazz, WheelActivity.fullscreen); }

		inline void onDestroy(ANativeActivity*) { destroy(); }
		inline void onStart(ANativeActivity*) {}
		inline void onResume(ANativeActivity*) { writecmd(cmd::resume); }
		inline void onPause(ANativeActivity*) { stop(); }
		inline void onStop(ANativeActivity*) { stop(); }
		inline void onCfgChanged(ANativeActivity*) { writecmd(cmd::cfgchanged); }
		inline void onLowMemory(ANativeActivity*) {  logdbg("onLowMemory"); writecmd(cmd::lowmem); }
		inline void onWindowFocus(ANativeActivity*, int focused) { writecmd(focused ? cmd::focus : cmd::unfocus); }
		inline void onWindowCreated(ANativeActivity*, ANativeWindow* window) {setwindow(window); }
		inline void onWindowDestroyed(ANativeActivity*, ANativeWindow*) { setwindow(0); }
		inline void onInputQueueCreated(ANativeActivity*, AInputQueue* queue) { setinput(queue); }
		inline void onInputQueueDestroyed(ANativeActivity*, AInputQueue*) { setinput(0); }
		inline void* onSaveInstanceState(ANativeActivity*, size_t*) { return 0; }
	}

	void events::add(int fd, function<void()> fn)
	{
		int i = 2;
		for(; i < n; ++i) if(!fns[i]) break;
		if(i == n) ++n;
		fds[i] = fd; fns[i] = fn;
		ALooper_addFd(android::looper, fd, i, ALOOPER_EVENT_INPUT, 0, 0);
	}

	void events::remove(int fd)
	{
		int i = 2;
		for(; i < n; ++i) if(fds[i] == fd) { fns[i] = 0; break; }
		if(i == n-1) --n;
		ALooper_removeFd(android::looper, fd);
	}

	application::application(const string&, int, int)
	{
		assert(!app);
		app = this;
		logdbg("application::application");
		android::init();
		EGLint w,h;
		eglQuerySurface(android::dpy, android::surface, EGL_WIDTH, &w);
		eglQuerySurface(android::dpy, android::surface, EGL_HEIGHT, &h);
		app->width = w;
		app->height = h;
	}

	application::~application() { android::clear(); app = 0; }

	void application::process(int timeout)
	{
		int i, events;
		while((i = ALooper_pollAll(timeout, 0, &events, 0)) >= 0) { events::fns[i](); timeout = 0; }
	}

	bool application::alive() const { return android::alive; }
	bool application::visible() const { return android::visible; }
	int application::orientation() const { return android::orientation; }
	void application::title(const string&) {}
	void application::fullscreen(bool) { android::fullscreen(); }
	void application::togglefullscreen() {}
	void application::hide() { android::movetoback(); }
	void application::show() {}
	void application::flip() { eglSwapBuffers(android::dpy, android::surface); }
	//void application::close() { logdbg("application::close()"); ANativeActivity_finish(android::act); }

	string resource(const string& name)
	{
		if(AAsset *a = AAssetManager_open(android::act->assetManager, name.c_str(), AASSET_MODE_BUFFER))
		{
			const uint8_t *start = (const uint8_t *)AAsset_getBuffer(a);
			size_t size = AAsset_getLength(a);
			return string(start, start + size);
		}
		logerr("Failed to load asset: %s", name.c_str());
		return string();
	}
}

void ANativeActivity_onCreate(ANativeActivity* activity, void*, size_t)
{
	logdbg("ANativeActivity_onCreate");
	activity->callbacks->onDestroy = wheel::android::onDestroy;
	activity->callbacks->onStart = wheel::android::onStart;
	activity->callbacks->onResume = wheel::android::onResume;
	activity->callbacks->onSaveInstanceState = wheel::android::onSaveInstanceState;
	activity->callbacks->onPause = wheel::android::onPause;
	activity->callbacks->onStop = wheel::android::onStop;
	activity->callbacks->onConfigurationChanged = wheel::android::onCfgChanged;
	activity->callbacks->onLowMemory = wheel::android::onLowMemory;
	activity->callbacks->onWindowFocusChanged = wheel::android::onWindowFocus;
	activity->callbacks->onNativeWindowCreated = wheel::android::onWindowCreated;
	activity->callbacks->onNativeWindowDestroyed = wheel::android::onWindowDestroyed;
	activity->callbacks->onInputQueueCreated = wheel::android::onInputQueueCreated;
	activity->callbacks->onInputQueueDestroyed = wheel::android::onInputQueueDestroyed;

	JNIEnv *env = activity->env;
	jclass NativeActivity = env->FindClass("android/app/NativeActivity");
	jmethodID NativeActivity_getClassLoader = env->GetMethodID(NativeActivity, "getClassLoader", "()Ljava/lang/ClassLoader;");
	jobject classLoader = env->CallObjectMethod(activity->clazz, NativeActivity_getClassLoader);
	jclass ClassLoader = env->FindClass("java/lang/ClassLoader");
	jmethodID ClassLoader_loadClass = env->GetMethodID(ClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

	wheel::android::WheelActivity.cls = (jclass)env->CallObjectMethod(classLoader, ClassLoader_loadClass, env->NewStringUTF("com/wheel/WheelActivity"));
	wheel::android::WheelActivity.cls = (jclass)env->NewGlobalRef(wheel::android::WheelActivity.cls);
	wheel::android::create(activity);
}
