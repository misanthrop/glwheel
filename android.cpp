#pragma once
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <mutex>
#include <string>
#include <sstream>
#include <cassert>
#include <algorithm>
#include <condition_variable>
#include <sched.h>
#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/sensor.h>
#include <android/log.h>
#include "app.hpp"
#include "audio.hpp"

extern int main();

namespace wheel
{
	using namespace std;

	void log(const string& s) { __android_log_print(ANDROID_LOG_INFO, "glwheel", "%s", s.c_str()); }
	void err(const string& s) { __android_log_print(ANDROID_LOG_ERROR, "glwheel", "%s", s.c_str()); }

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
			key::camera,	// AKEYCODE_CAMERA          = 27,
			key::clear,		// AKEYCODE_CLEAR           = 28,
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
			key::plus,		// AKEYCODE_PLUS            = 81,
			key::menu,		// AKEYCODE_MENU            = 82,
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

		struct cmd
		{
			enum : uint8_t
			{
				inputchanged, initwnd, termwnd, resize, redraw, contentresize, focus, unfocus,
				cfgchanged, lowmem, start, resume, save, pause, stop, destroy
			};
		};

		ANativeActivity *act;
		std::mutex mutex;
		std::condition_variable cond;
		int msgpipe[2];
		uint8_t state;
		bool running, destroyed, saved;
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
		size_t mn = 1;
		point m[32];
		bool alive = 1;

		int8_t readcmd()
		{
			int8_t cmd;
			if(read(msgpipe[0], &cmd, sizeof(cmd)) == sizeof(cmd)) return cmd;
			err("No data on command pipe!");
			return -1;
		}

		void writecmd(int8_t cmd) { if(write(msgpipe[1], &cmd, sizeof(cmd)) != sizeof(cmd)) err(string("write failed: ") + strerror(errno)); }

		void create(ANativeActivity *a)
		{
			act = a;
			state = 0;
			running = destroyed = saved = 0;
			pendingInputQueue = 0;
			pendingWindow = 0;
			if(pipe(msgpipe)) { err(string("could not create pipe: ") + strerror(errno)); return; }
			std::thread(main).detach();
			std::unique_lock<std::mutex> lock(mutex);
			cond.wait(lock, []{ return running; });
		}

		void destroy()
		{
			{
				std::unique_lock<std::mutex> lock(mutex);
				writecmd(cmd::destroy);
				cond.wait(lock, []{ return destroyed; });
			}
			close(msgpipe[0]);
			close(msgpipe[1]);
		}

		void setinput(AInputQueue* inputQueue)
		{
			std::unique_lock<std::mutex> lock(mutex);
			pendingInputQueue = inputQueue;
			writecmd(cmd::inputchanged);
			cond.wait(lock, [inputQueue]{ return inputQueue == pendingInputQueue; });
		}

		void setwindow(ANativeWindow* window)
		{
			std::unique_lock<std::mutex> lock(mutex);
			if(pendingWindow) writecmd(cmd::termwnd);
			pendingWindow = window;
			if(window) writecmd(cmd::initwnd);
			cond.wait(lock, [window]{ return window == pendingWindow; });
		}

		void setstate(int8_t cmd)
		{
			std::unique_lock<std::mutex> lock(mutex);
			writecmd(cmd);
			cond.wait(lock, [cmd]{ return state == cmd; });
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
						if(key::type k = android::key[AKeyEvent_getKeyCode(event)])
						{
							switch(AKeyEvent_getAction(event))
							{
								case AKEY_EVENT_ACTION_DOWN: key::state(k) = 1; app.press(k); break;
								case AKEY_EVENT_ACTION_UP: key::state(k) = 0; app.release(k); break;
							}
							processed = 1;
						}
						break;

					case AINPUT_EVENT_TYPE_MOTION:
					{
						size_t prevn = mn;
						point prevm[32]; memcpy(prevm, m, sizeof(m));
						mn = std::min((size_t)32, AMotionEvent_getPointerCount(event));
						int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
//						constexpr uint8_t button[] = { key::lbutton, key::rbutton, key::mbutton };
//						if(action == AMOTION_EVENT_ACTION_DOWN && mn != 1 && prevn == 1) if(key::state(*key::lbutton))
//							key::state(key::lbutton) = 0;
						for(size_t i = 0; i < mn; ++i) m[i] = point(AMotionEvent_getX(event, i), app.height() - AMotionEvent_getY(event, i));
						app.pointermove();
						if(mn == 1)
						{
							//uint8_t b = button[mn-1];
							switch(action)
							{
								case AMOTION_EVENT_ACTION_DOWN:
									if(!*key::lbutton)
									{
										key::state(key::lbutton) = 1;
										app.press(key::lbutton);
									}
									break;
								case AMOTION_EVENT_ACTION_UP:
									key::state(key::lbutton) = 0;
									app.release(key::lbutton);
									break;
							}
						}
						if(mn == 2 && prevn == 2)
						{
							point d = m[1] - m[0], prevd = prevm[1] - prevm[0];
							float diff2 = ~prevd - ~d;
							float absdiff = sqrt(abs(diff2));
							if(absdiff > 16) app.scroll(diff2*4/absdiff/std::min(app.width(),app.height()));
						}
						processed = 1;
						break;
					}
				}
				AInputQueue_finishEvent(inputQueue, event, processed);
			}
			else err(string("Failure reading next input event: ") + strerror(errno));
		}

		void processsensor()
		{
			if(accelerometer)
			{
				ASensorEvent event;
				while(ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
					app.accel(event.acceleration.x, event.acceleration.y, event.acceleration.z);
			}
		}

		void initwnd()
		{
			log("initwnd()");

			EGLint format;
			eglGetConfigAttrib(dpy, glconfig, EGL_NATIVE_VISUAL_ID, &format);
			ANativeWindow_setBuffersGeometry(window, 0, 0, format);

			surface = eglCreateWindowSurface(dpy, glconfig, window, 0);
			const EGLint attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
			context = eglCreateContext(dpy, glconfig, 0, attribs);

			eglMakeCurrent(dpy, surface, surface, context);
			log((const char*)glGetString(GL_VERSION));
			app.update();
			sensorManager = ASensorManager_getInstance();
			accelerometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
			sensorEventQueue = ASensorManager_createEventQueue(sensorManager, looper, 1, 0, 0);
			events::fns[1] = processsensor;

			app.active = 1;
			app.reload();
		}

		void termwnd()
		{
			log("termwnd()");
			app.active = 0;
			app.clear();
			eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroySurface(dpy, surface);
			if(context != EGL_NO_CONTEXT) eglDestroyContext(dpy, context);
		}

		int8_t processcmd()
		{
			int8_t c = readcmd();
			switch(c)
			{
				case cmd::inputchanged:
				{
					std::unique_lock<std::mutex> lock(mutex);
					if(inputQueue)
					{
						AInputQueue_detachLooper(inputQueue);
						events::fns[0] = 0;
					}
					inputQueue = pendingInputQueue;
					if(inputQueue)
					{
						events::fns[0] = processinput;
						AInputQueue_attachLooper(inputQueue, looper, 0, 0, 0);
					}
					cond.notify_all();
					break;
				}

				case cmd::initwnd:
					{
						std::unique_lock<std::mutex> lock(mutex);
						window = pendingWindow;
						cond.notify_all();
					}
					if(window) initwnd();
					break;

				case cmd::termwnd:
					{
						std::unique_lock<std::mutex> lock(mutex);
						window = 0;
						cond.notify_all();
					}
					termwnd();
					break;

				case cmd::resume: case cmd::start: case cmd::pause: case cmd::stop:
				{
					std::unique_lock<std::mutex> lock(mutex);
					state = c;
					cond.notify_all();
					break;
				}

				case cmd::focus:
					if(accelerometer)
					{
						ASensorEventQueue_enableSensor(sensorEventQueue, accelerometer);
						ASensorEventQueue_setEventRate(sensorEventQueue, accelerometer, (1000L/60)*1000);
					}
					app.active = 1;
					break;

				case cmd::unfocus:
					app.active = 0;
					if(accelerometer) ASensorEventQueue_disableSensor(sensorEventQueue, accelerometer);
					break;

				case cmd::cfgchanged:
					AConfiguration_fromAssetManager(config, act->assetManager);
					//print_cur_config(app);
					break;

				case cmd::destroy:
					alive = 0;
					break;

				case cmd::save:
				{
					std::unique_lock<std::mutex> lock(mutex);
					saved = 1;
					cond.notify_all();
					break;
				}
			}
			return c;
		}

		void init()
		{
			config = AConfiguration_new();
			looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
			dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);

			android::act->instance = &app;
			AConfiguration_fromAssetManager(config, act->assetManager);

			events::add(android::msgpipe[0], [&]() { processcmd(); });

			eglInitialize(dpy, 0, 0);

			//uint8_t depth = 16, stencil = 0, r = 5, g = 6, b = 5, a = 8;
			const EGLint attribs[] = { EGL_RED_SIZE,8, EGL_GREEN_SIZE,8, EGL_BLUE_SIZE,8, EGL_ALPHA_SIZE,8, EGL_DEPTH_SIZE,16,
									   EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE };

			EGLint numConfigs;
			eglChooseConfig(dpy, attribs, &glconfig, 1, &numConfigs);

			{
				std::unique_lock<std::mutex> lock(mutex);
				running = 1;
				cond.notify_all();
			}

			// I hate crap like the following
			mkdir(act->internalDataPath,0770);
			chdir(act->internalDataPath);
			const char *dirs[] = { "scripts" };
			for(const char *dirname : dirs) if(AAssetDir *dir = AAssetManager_openDir(act->assetManager, dirname))
			{
				log(string("dir: ") + dirname);
				mkdir(dirname,0770);
				while(const char *fn = AAssetDir_getNextFileName(dir))
				{
					string spath = dirname;
					if(spath.size()) spath += "/";
					spath += fn;
					const char *path = spath.c_str();
					AAsset *a = AAssetManager_open(act->assetManager, path, AASSET_MODE_BUFFER);
					int f = open(path, O_CREAT|O_TRUNC|O_WRONLY, 0660);
					log(string("file: ") + path);
					if(f != -1) write(f, AAsset_getBuffer(a), AAsset_getLength(a));
					else log(strerror(errno));
					::close(f);
					AAsset_close(a);
				}
				AAssetDir_close(dir);
			}

			while(processcmd() != cmd::initwnd);
			log("go!");
		}

		void clear()
		{
			if(dpy != EGL_NO_DISPLAY) eglTerminate(dpy);
			std::unique_lock<std::mutex> lock(mutex);
			if(inputQueue) { AInputQueue_detachLooper(inputQueue); inputQueue = 0; }
			if(config) { AConfiguration_delete(config); config = 0; }
			destroyed = 1;
			cond.notify_all();
		}

		inline void onDestroy(ANativeActivity*) { destroy(); }
		inline void onStart(ANativeActivity*) { setstate(cmd::start); }
		inline void onResume(ANativeActivity*) { setstate(cmd::resume); }
		inline void onPause(ANativeActivity*) { setstate(cmd::pause); }
		inline void onStop(ANativeActivity*) { setstate(cmd::stop); }
		inline void onCfgChanged(ANativeActivity*) { writecmd(cmd::cfgchanged); }
		inline void onLowMemory(ANativeActivity*) { writecmd(cmd::lowmem); }
		inline void onWindowFocus(ANativeActivity*, int focused) { writecmd(focused ? cmd::focus : cmd::unfocus); }
		inline void onWindowCreated(ANativeActivity*, ANativeWindow* window) { setwindow(window); }
		inline void onWindowDestroyed(ANativeActivity*, ANativeWindow* window) { setwindow(0); }
		inline void onInputQueueCreated(ANativeActivity*, AInputQueue* queue) { setinput(queue); }
		inline void onInputQueueDestroyed(ANativeActivity*, AInputQueue*) { setinput(0); }
		inline void* onSaveInstanceState(ANativeActivity*, size_t*) { return 0; }
	}

	namespace sl
	{
		inline SLEngineItf engine()
		{
			static SLObjectItf obj = 0;
			static SLEngineItf instance;
			if(!obj)
			{
				assert(slCreateEngine(&obj, 0, 0, 0, 0, 0) == SL_RESULT_SUCCESS);
				assert((*obj)->Realize(obj, SL_BOOLEAN_FALSE) == SL_RESULT_SUCCESS);
				// get the engine interface, which is needed in order to create other objects
				assert((*obj)->GetInterface(obj, SL_IID_ENGINE, &instance) == SL_RESULT_SUCCESS);
			}
			return instance;
		}

		inline SLObjectItf mix()
		{
			static SLObjectItf obj = 0;
			SLEngineItf eng = engine();
			if(!obj)
			{
				// create output mix, with environmental reverb specified as a non-required interface
				const SLInterfaceID ids[] = { SL_IID_VOLUME };
				const SLboolean req[] = { SL_BOOLEAN_FALSE };
				assert((*eng)->CreateOutputMix(eng, &obj, 1, ids, req) == SL_RESULT_SUCCESS);
				assert((*obj)->Realize(obj, SL_BOOLEAN_FALSE) == SL_RESULT_SUCCESS);
			}
			return obj;
		}
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

//	struct stream
//	{
//		AAsset *a = 0;

//		stream() {}
//		stream(const char *name) { open(name); }
//		stream(stream&& s) : a(s.a) { s.a = 0; }
//		stream(const stream&) = delete;
//		~stream() { if(a) AAsset_close(a); }

//		operator bool() const { return a; }
//		bool eof() const { return !remain(); }

//		void open(const char *name) { a = AAssetManager_open(android::act->assetManager, name, 0); }
//		int read(char *buf, size_t n) { return AAsset_read(a, buf, n); }
//		off_t seek(off_t off, int whence) { return AAsset_seek(a, off, whence); }
//		off_t length() const { return AAsset_getLength(a); }
//		off_t remain() const { return AAsset_getRemainingLength(a); }
//	};

	void application::process(int timeout)
	{
		int i, events;
		while((i = ALooper_pollAll(timeout, 0, &events, 0)) >= 0) { events::fns[i](); timeout = 0; }
	}

	bool application::alive() const { return android::alive; }

	void application::init(const string&, int, int)
	{
		android::init();
		set(0,0,0,0);
		EGLint w,h;
		eglQuerySurface(android::dpy, android::surface, EGL_WIDTH, &w);
		eglQuerySurface(android::dpy, android::surface, EGL_HEIGHT, &h);
		set(0,0,w,h);
	}

	void application::title(const string&) {}
	void application::fullscreen(bool) {}
	void application::togglefullscreen() {}
	void application::flip() { eglSwapBuffers(android::dpy, android::surface); }
	point application::pointer() const { return android::m[0]; }

	void application::update()
	{
		EGLint w,h;
		eglQuerySurface(android::dpy, android::surface, EGL_WIDTH, &w);
		eglQuerySurface(android::dpy, android::surface, EGL_HEIGHT, &h);
		if(width() != w || height() != h)
		{
			set(0,0,w,h);
			resize();
		}
		widget::update();
	}

	bool application::show(bool) { return active; }
	void application::draw() { glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); widget::draw(); flip(); }
	void application::close() { log("close()"); android::writecmd(android::cmd::destroy); }

	string loadasset(const string& name)
	{
		if(AAsset *a = AAssetManager_open(android::act->assetManager, name.c_str(), AASSET_MODE_BUFFER))
		{
			const uint8_t *start = (const uint8_t *)AAsset_getBuffer(a);
			size_t size = AAsset_getLength(a);
			return string(start, start + size);
		}
		err("Failed to load asset: " + name);
		return string();
	}

	string application::resource(const string& name) { return loadasset(name); }

	struct nativeaudiotrack
	{
		SLObjectItf obj = 0;
		SLPlayItf playitf;
		SLSeekItf seekitf;

		void clear() { if(obj) { (*obj)->Destroy(obj); obj = 0; } }

		void set(string&& fn)
		{
			AAsset* asset = AAssetManager_open(android::act->assetManager, fn.c_str(), AASSET_MODE_UNKNOWN);
			off_t start, length;
			int fd = AAsset_openFileDescriptor(asset, &start, &length);
			assert(0 <= fd);
			AAsset_close(asset);
			// open asset as file descriptor

			// configure audio source
			SLDataLocator_AndroidFD loc_fd = { SL_DATALOCATOR_ANDROIDFD, fd, start, length };
			SLDataFormat_MIME format_mime = { SL_DATAFORMAT_MIME, 0, SL_CONTAINERTYPE_UNSPECIFIED };
			SLDataSource audioSrc = {&loc_fd, &format_mime};

			// configure audio sink
			SLObjectItf outputMix = sl::mix();
			SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, outputMix };
			SLDataSink audioSnk = { &loc_outmix, 0 };

			// create audio player
			const SLInterfaceID ids[] = { SL_IID_PLAY, SL_IID_SEEK };
			const SLboolean req[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
			SLEngineItf eng = sl::engine();
			assert((*eng)->CreateAudioPlayer(eng, &obj, &audioSrc, &audioSnk, 2, ids, req) == SL_RESULT_SUCCESS);
			assert((*obj)->Realize(obj, SL_BOOLEAN_FALSE) == SL_RESULT_SUCCESS);
			assert((*obj)->GetInterface(obj, SL_IID_PLAY, &playitf) == SL_RESULT_SUCCESS);
			assert((*obj)->GetInterface(obj, SL_IID_SEEK, &seekitf) == SL_RESULT_SUCCESS);
			assert((*seekitf)->SetLoop(seekitf, SL_BOOLEAN_TRUE, 0, SL_TIME_UNKNOWN) == SL_RESULT_SUCCESS);
		}

		void play(bool b = 1)
		{
			assert((*playitf)->SetPlayState(playitf, b ? SL_PLAYSTATE_PLAYING : SL_PLAYSTATE_PAUSED) == SL_RESULT_SUCCESS);
		}
	};

	bool audiotrack::operator!() const { return !native->obj; }
	void audiotrack::clear() { native->clear(); }
	void audiotrack::set(string&& s) { native->set(forward<string>(s)); }
	void audiotrack::play(bool b) { native->play(b); }

	application app;
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
	wheel::android::create(activity);
}
