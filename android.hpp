#pragma once
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <poll.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sched.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>
#include <android/log.h>
#include "widget.hpp"

extern int main();

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "app", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "app", __VA_ARGS__))

namespace wheel
{
	namespace android
	{
		constexpr const key::type key[] =
		{
			key::unknown,
			key::unknown,	// AKEYCODE_SOFT_LEFT       = 1,
			key::unknown,	// AKEYCODE_SOFT_RIGHT      = 2,
			key::home,		// AKEYCODE_HOME            = 3,
			//key::esc,		// AKEYCODE_BACK            = 4,
			key::unknown,	// AKEYCODE_BACK            = 4,
			key::unknown,	// AKEYCODE_CALL            = 5,
			key::unknown,	// AKEYCODE_ENDCALL         = 6,
			key::n0, key::n1, key::n2, key::n3, key::n4, key::n5, key::n6, key::n7, key::n8, key::n9,	// AKEYCODE_0 = 7 .. AKEYCODE_9 = 16,
			key::unknown,	// AKEYCODE_STAR            = 17,
			key::unknown,	// AKEYCODE_POUND           = 18,
			key::up,		// AKEYCODE_DPAD_UP         = 19,
			key::down,		// AKEYCODE_DPAD_DOWN       = 20,
			key::left,		// AKEYCODE_DPAD_LEFT       = 21,
			key::right,		// AKEYCODE_DPAD_RIGHT      = 22,
			key::center,	// AKEYCODE_DPAD_CENTER     = 23,
			key::volumeup,	// AKEYCODE_VOLUME_UP       = 24,
			key::volumedown,// AKEYCODE_VOLUME_DOWN     = 25,
			key::power,		// AKEYCODE_POWER           = 26,
			key::unknown,	// AKEYCODE_CAMERA          = 27,
			key::unknown,	// AKEYCODE_CLEAR           = 28,
			key::a, key::b, key::c, key::d, key::e, key::f, key::g, key::h, key::i, key::j, key::k, key::l, key::m,	// AKEYCODE_A = 29,
			key::n, key::o, key::p, key::q, key::r, key::s, key::t, key::u, key::v, key::w, key::x, key::y, key::z,	// AKEYCODE_Z = 54,
			key::comma,		// AKEYCODE_COMMA           = 55,
			key::period,	// AKEYCODE_PERIOD          = 56,
			key::unknown,	// AKEYCODE_ALT_LEFT        = 57,
			key::unknown,	// AKEYCODE_ALT_RIGHT       = 58,
			key::unknown,	// AKEYCODE_SHIFT_LEFT      = 59,
			key::unknown,	// AKEYCODE_SHIFT_RIGHT     = 60,
			key::tab,		// AKEYCODE_TAB             = 61,
			key::space,		// AKEYCODE_SPACE           = 62,
			key::unknown,	// AKEYCODE_SYM             = 63,
			key::unknown,	// AKEYCODE_EXPLORER        = 64,
			key::unknown,	// AKEYCODE_ENVELOPE        = 65,
			key::enter,		// AKEYCODE_ENTER           = 66,
			key::del,		// AKEYCODE_DEL             = 67,
			key::grave,		// AKEYCODE_GRAVE           = 68,
			key::minus,		// AKEYCODE_MINUS           = 69,
			key::equals,	// AKEYCODE_EQUALS          = 70,
			key::lbracket,	// AKEYCODE_LEFT_BRACKET    = 71,
			key::rbracket,	// AKEYCODE_RIGHT_BRACKET   = 72,
			key::backslash,	// AKEYCODE_BACKSLASH       = 73,
			key::semicolon,	// AKEYCODE_SEMICOLON       = 74,
			key::apostrophe,// AKEYCODE_APOSTROPHE      = 75,
			key::slash,		// AKEYCODE_SLASH           = 76,
			key::unknown,	// AKEYCODE_AT              = 77,
			key::unknown,	// AKEYCODE_NUM             = 78,
			key::unknown,	// AKEYCODE_HEADSETHOOK     = 79,
			key::unknown,	// AKEYCODE_FOCUS           = 80,   // *Camera* focus
			key::unknown,	// AKEYCODE_PLUS            = 81,
			key::unknown,	// AKEYCODE_MENU            = 82,
			key::unknown,	// AKEYCODE_NOTIFICATION    = 83,
			key::unknown,	// AKEYCODE_SEARCH          = 84,
			key::play,		// AKEYCODE_MEDIA_PLAY_PAUSE= 85,
			key::stop,		// AKEYCODE_MEDIA_STOP      = 86,
			key::next,		// AKEYCODE_MEDIA_NEXT      = 87,
			key::prev,		// AKEYCODE_MEDIA_PREVIOUS  = 88,
			key::rewind,	// AKEYCODE_MEDIA_REWIND    = 89,
			key::forward,	// AKEYCODE_MEDIA_FAST_FORWARD = 90,
			key::mute,		// AKEYCODE_MUTE            = 91,
			key::pageup,	// AKEYCODE_PAGE_UP         = 92,
			key::pagedown,	// AKEYCODE_PAGE_DOWN       = 93,
			key::unknown,	// AKEYCODE_PICTSYMBOLS     = 94,
			key::unknown,	// AKEYCODE_SWITCH_CHARSET  = 95,
			key::unknown,	// AKEYCODE_BUTTON_A        = 96,
			key::unknown,	// AKEYCODE_BUTTON_B        = 97,
			key::unknown,	// AKEYCODE_BUTTON_C        = 98,
			key::unknown,	// AKEYCODE_BUTTON_X        = 99,
			key::unknown,	// AKEYCODE_BUTTON_Y        = 100,
			key::unknown,	// AKEYCODE_BUTTON_Z        = 101,
			key::unknown,	// AKEYCODE_BUTTON_L1       = 102,
			key::unknown,	// AKEYCODE_BUTTON_R1       = 103,
			key::unknown,	// AKEYCODE_BUTTON_L2       = 104,
			key::unknown,	// AKEYCODE_BUTTON_R2       = 105,
			key::unknown,	// AKEYCODE_BUTTON_THUMBL   = 106,
			key::unknown,	// AKEYCODE_BUTTON_THUMBR   = 107,
			key::unknown,	// AKEYCODE_BUTTON_START    = 108,
			key::unknown,	// AKEYCODE_BUTTON_SELECT   = 109,
			key::unknown,	// AKEYCODE_BUTTON_MODE     = 110,
		};

		struct activity
		{
			struct cmd
			{
				enum : uint8_t
				{
					inputchanged, initwnd, termwnd, resize, redraw, contentresize, focus, unfocus,
					cfgchanged, lowmem, start, resume, save, pause, stop, destroy
				};
			};

			ANativeActivity *a;
			std::mutex mutex;
			std::condition_variable cond;
			int msgpipe[2];
			uint8_t state;
			bool running, destroyed, saved/*, redrawNeeded = 0*/;
			AInputQueue* pendingInputQueue = 0;
			ANativeWindow* pendingWindow = 0;

			void create(ANativeActivity *act)
			{
				a = act;
				state = 0;
				running = destroyed = saved = 0;
				pendingInputQueue = 0;
				pendingWindow = 0;
				if(pipe(msgpipe)) { LOGE("could not create pipe: %s", strerror(errno)); return; }
				std::thread(main).detach();
				std::unique_lock<std::mutex> lock(mutex);
				cond.wait(lock, [this]{ return running; });
			}

			void destroy()
			{
				{
					std::unique_lock<std::mutex> lock(mutex);
					writecmd(cmd::destroy);
					cond.wait(lock, [this]{ return destroyed; });
				}
				close(msgpipe[0]);
				close(msgpipe[1]);
			}

			int8_t readcmd()
			{
				int8_t cmd;
				if(read(msgpipe[0], &cmd, sizeof(cmd)) == sizeof(cmd)) return cmd;
				LOGE("No data on command pipe!");
				return -1;
			}

			void writecmd(int8_t cmd) { if(write(msgpipe[1], &cmd, sizeof(cmd)) != sizeof(cmd)) LOGE("write failed: %s\n", strerror(errno)); }

			void setinput(AInputQueue* inputQueue)
			{
				std::unique_lock<std::mutex> lock(mutex);
				pendingInputQueue = inputQueue;
				writecmd(cmd::inputchanged);
				cond.wait(lock, [this,inputQueue]{ return inputQueue == pendingInputQueue; });
			}

			void setwindow(ANativeWindow* window)
			{
				std::unique_lock<std::mutex> lock(mutex);
				if(pendingWindow) writecmd(cmd::termwnd);
				pendingWindow = window;
				if(window) writecmd(cmd::initwnd);
				cond.wait(lock, [this,window]{ return window == pendingWindow; });
			}

			void setstate(int8_t cmd)
			{
				std::unique_lock<std::mutex> lock(mutex);
				writecmd(cmd);
				cond.wait(lock, [this,cmd]{ return state == cmd; });
			}
		};

		inline activity& act() { static activity a; return a; }

		inline void onDestroy(ANativeActivity*) { act().destroy(); }
		inline void onStart(ANativeActivity*) { act().setstate(activity::cmd::start); }
		inline void onResume(ANativeActivity*) { act().setstate(activity::cmd::resume); }
		inline void onPause(ANativeActivity*) { act().setstate(activity::cmd::pause); }
		inline void onStop(ANativeActivity*) { act().setstate(activity::cmd::stop); }
		inline void onCfgChanged(ANativeActivity*) { act().writecmd(activity::cmd::cfgchanged); }
		inline void onLowMemory(ANativeActivity*) { act().writecmd(activity::cmd::lowmem); }
		inline void onWindowFocus(ANativeActivity*, int focused) { act().writecmd(focused ? activity::cmd::focus : activity::cmd::unfocus); }
		inline void onWindowCreated(ANativeActivity*, ANativeWindow* window) { act().setwindow(window); }
		inline void onWindowDestroyed(ANativeActivity*, ANativeWindow* window) { act().setwindow(0); }
		inline void onInputQueueCreated(ANativeActivity*, AInputQueue* queue) { act().setinput(queue); }
		inline void onInputQueueDestroyed(ANativeActivity*, AInputQueue*) { act().setinput(0); }
		inline void* onSaveInstanceState(ANativeActivity*, size_t*) { return 0; }
	}

	struct application : widget
	{
		enum { main_id = 1, input_id, sensor_id }; // Looper ids
		using cmd = android::activity::cmd;

		AConfiguration *config = AConfiguration_new();
		ALooper *looper = 0;
		AInputQueue *inputQueue = 0;
		ANativeWindow *window = 0;

		ASensorManager *sensorManager = 0;
		const ASensor *accelerometer = 0;
		ASensorEventQueue *sensorEventQueue = 0;
		EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		EGLContext context = EGL_NO_CONTEXT;
		EGLSurface surface;
		EGLConfig glconfig;
		point m;
		bool alive = 1;

		application()
		{
			LOGI("fn()");
			android::act().a->instance = this;
			AConfiguration_fromAssetManager(config, android::act().a->assetManager);

			looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
			ALooper_addFd(looper, android::act().msgpipe[0], main_id, ALOOPER_EVENT_INPUT, 0, 0);

			eglInitialize(dpy, 0, 0);

			uint8_t depth = 16, stencil = 0, r = 8, g = 8, b = 8, a = 8;
			const EGLint attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_DEPTH_SIZE, depth, EGL_STENCIL_SIZE, stencil,
				EGL_RED_SIZE,r, EGL_GREEN_SIZE,g, EGL_BLUE_SIZE,b, EGL_ALPHA_SIZE,a, EGL_NONE };
			EGLint numConfigs;
			eglChooseConfig(dpy, attribs, &glconfig, 1, &numConfigs);

			{
				std::unique_lock<std::mutex> lock(android::act().mutex);
				android::act().running = 1;
				android::act().cond.notify_all();
			}
		}

		~application()
		{
			eglTerminate(dpy);
			std::unique_lock<std::mutex> lock(android::act().mutex);
			if(inputQueue) AInputQueue_detachLooper(inputQueue);
			AConfiguration_delete(config);
			android::act().destroyed = 1;
			android::act().cond.notify_all();
		}

		operator bool() const { return alive; }

		point pointer() const { return m; }
		virtual void accel(float,float,float) {}

		void process(int timeout = -1)
		{
			//events.process(timeout);
			LOGI("process(%d)", timeout);
			int ident, events;
			while((ident = ALooper_pollAll(timeout, 0, &events, 0)) >= 0)
			{
				switch(ident)
				{
					case main_id:
					{
						int8_t c = android::act().readcmd();
						LOGI("process_cmd(%d)", c);
						switch(c)
						{
							case cmd::inputchanged:
							{
								std::unique_lock<std::mutex> lock(android::act().mutex);
								if(inputQueue)
								{
									LOGI("AInputQueue_detachLooper");
									AInputQueue_detachLooper(inputQueue);
								}
								inputQueue = android::act().pendingInputQueue;
								if(inputQueue)
								{
									LOGI("AInputQueue_attachLooper");
									AInputQueue_attachLooper(inputQueue, looper, input_id, 0, 0);
								}
								android::act().cond.notify_all();
								break;
							}

							case cmd::initwnd:
								{
									std::unique_lock<std::mutex> lock(android::act().mutex);
									window = android::act().pendingWindow;
									android::act().cond.notify_all();
								}
								if(window) initwnd();
								break;

							case cmd::termwnd:
								{
									std::unique_lock<std::mutex> lock(android::act().mutex);
									window = 0;
									android::act().cond.notify_all();
								}
								termwnd();
								break;

							case cmd::resume: case cmd::start: case cmd::pause: case cmd::stop:
							{
								std::unique_lock<std::mutex> lock(android::act().mutex);
								android::act().state = c;
								android::act().cond.notify_all();
								break;
							}

							case cmd::focus:
								if(accelerometer)
								{
									ASensorEventQueue_enableSensor(sensorEventQueue, accelerometer);
									ASensorEventQueue_setEventRate(sensorEventQueue, accelerometer, (1000L/60)*1000);
								}
								break;

							case cmd::unfocus:
								if(accelerometer) ASensorEventQueue_disableSensor(sensorEventQueue, accelerometer);
								break;

							case cmd::cfgchanged:
								AConfiguration_fromAssetManager(config, android::act().a->assetManager);
								//print_cur_config(app);
								break;

							case cmd::destroy:
								alive = 0;
								break;

							case cmd::save:
							{
								std::unique_lock<std::mutex> lock(android::act().mutex);
								android::act().saved = 1;
								android::act().cond.notify_all();
								break;
							}
						}
						break;
					}

					case input_id:
					{
						AInputEvent* event = 0;
						if(AInputQueue_getEvent(inputQueue, &event) >= 0)
						{
							if(AInputQueue_preDispatchEvent(inputQueue, event)) return;
							bool processed = 0;
							switch(AInputEvent_getType(event))
							{
								case AINPUT_EVENT_TYPE_KEY:
									LOGI("key %d", AKeyEvent_getKeyCode(event));
									if(key::type k = android::key[AKeyEvent_getKeyCode(event)])
									{
										key::state(k);
										press(k);
										processed = 1;
									}
									break;

								case AINPUT_EVENT_TYPE_MOTION:
									LOGI("motion");
									m = point(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));
									pointermove();
									processed = 1;
									break;
							}
							AInputQueue_finishEvent(inputQueue, event, processed);
						}
						else LOGE("Failure reading next input event: %s\n", strerror(errno));
						break;
					}

					case sensor_id:
						if(accelerometer)
						{
							ASensorEvent event;
							while(ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
								accel(event.acceleration.x, event.acceleration.y, event.acceleration.z);
						}
						break;
				}
				timeout = 0;
			}
		}

		void close() { android::act().writecmd(cmd::destroy); }
		void flip() { eglSwapBuffers(dpy, surface); }
		void draw() { glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); for(widget *c : children) if(*c) c->draw(); flip(); }

		void initwnd()
		{
			LOGI("initwnd()");
			EGLint format;
			eglGetConfigAttrib(dpy, glconfig, EGL_NATIVE_VISUAL_ID, &format);
			context = eglCreateContext(dpy, glconfig, 0, 0);
			ANativeWindow_setBuffersGeometry(window, 0, 0, format);
			surface = eglCreateWindowSurface(dpy, glconfig, window, 0);
			eglMakeCurrent(dpy, surface, surface, context);
			EGLint w,h;
			eglQuerySurface(dpy, surface, EGL_WIDTH, &w);
			eglQuerySurface(dpy, surface, EGL_HEIGHT, &h);
			set(0,0,w,h);
			resize();
			sensorManager = ASensorManager_getInstance();
			accelerometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
			sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, sensor_id, 0, 0);
		}

		void termwnd()
		{
			LOGI("termwnd()");
			eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroySurface(dpy, surface);
			if(context != EGL_NO_CONTEXT) eglDestroyContext(dpy, context);
		}
	};

	namespace native
	{
		struct window : widget
		{
			application& app() { return *(application*)parent; }

			window(application& app, const char *title, int w = 1, int h = 1)
			{
				if(!w) w = app.width();
				if(!h) h = app.height();
				set(0,0,w,h);
				parent = &app;
				app.children.push_back(this);
			}

			void resize() { size(parent->size()); widget::resize(); }
			void show(bool) {}
			void fullscreen(bool) {}
			void togglefullscreen() {}
			void makecurrent() {}
			void flip() { app().flip(); }
		};
	}
}

void ANativeActivity_onCreate(ANativeActivity* activity, void*, size_t)
{
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
	wheel::android::act().create(activity);
}
