#pragma once
#include <jni.h>
#include <errno.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <android/sensor.h>
#include <android/log.h>
#include "android_native_app_glue.h"
#include "widget.hpp"
#include "events.hpp"
#include "utf.hpp"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

extern android_app *aapp;

namespace wheel
{
	struct application : widget
	{
		eventloop events;
		ASensorManager *sensorManager;
		const ASensor *accelerometer;
		ASensorEventQueue *sensorEventQueue;
		EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		EGLContext context = EGL_NO_CONTEXT;
		EGLConfig config;
		point m;

		application(uint8_t depth = 16, uint8_t stencil = 0, uint8_t r = 1, uint8_t g = 1, uint8_t b = 1, uint8_t a = 1)
		{
			eglInitialize(dpy, 0, 0);
			const EGLint attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_DEPTH_SIZE, depth, EGL_STENCIL_SIZE, stencil,
				EGL_RED_SIZE,r, EGL_GREEN_SIZE,g, EGL_BLUE_SIZE,b, EGL_ALPHA_SIZE,a, EGL_NONE };
			EGLint numConfigs;
			eglChooseConfig(dpy, attribs, &config, 1, &numConfigs);
			EGLint format;
			eglGetConfigAttrib(dpy, config, EGL_NATIVE_VISUAL_ID, &format);
			ANativeWindow_setBuffersGeometry(aapp->window, 0, 0, format);
			context = eglCreateContext(dpy, config, 0, 0);

			sensorManager = ASensorManager_getInstance();
			accelerometer = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
			sensorEventQueue = ASensorManager_createEventQueue(sensorManager, aapp->looper, LOOPER_ID_USER, NULL, NULL);
			aapp->userData = this;
			aapp->onAppCmd = cmd;
			aapp->onInputEvent = input;
		}

		~application()
		{
			eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			if(context != EGL_NO_CONTEXT) eglDestroyContext(dpy, context);
			eglTerminate(dpy);
		}

		point pointer() const { return m; }
		virtual void accel(float,float,float) {}

		void process(int timeout = -1)
		{
			//events.process(timeout);
			int ident, events;
			struct android_poll_source* source;
			while((ident = ALooper_pollAll(timeout, 0, &events, (void**)&source)) >= 0)
			{
				if(source) source->process(aapp, source);
				if(ident == LOOPER_ID_USER) // If a sensor has data, process it now.
				{
					if(accelerometer)
					{
						ASensorEvent event;
						while(ASensorEventQueue_getEvents(sensorEventQueue, &event, 1) > 0)
							accel(event.acceleration.x, event.acceleration.y, event.acceleration.z);
					}
				}
				if(aapp->destroyRequested)
				{
					for(widget *w : children) w->close();
					return;
				}
			}
		}

		static int32_t input(android_app* aapp, AInputEvent* event)
		{
			application &app = *(application*)aapp->userData;
			if(AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
			{
				app.m = point(AMotionEvent_getX(event, 0), AMotionEvent_getY(event, 0));
				for(widget *w : app.children) w->pointermove();
				return 1;
			}
			return 0;
		}

		static void cmd(struct android_app* app, int32_t cmd)
		{/*
			application* app = (application*)aapp->userData;
			switch(cmd)
			{
				case APP_CMD_SAVE_STATE:
					engine->app->savedState = malloc(sizeof(struct saved_state));
					*((struct saved_state*)engine->app->savedState) = engine->state;
					engine->app->savedStateSize = sizeof(struct saved_state);
					break;
				case APP_CMD_INIT_WINDOW:
					if (engine->app->window != NULL) {
						engine_init_dpy(engine);
						engine_draw_frame(engine);
					}
					break;
				case APP_CMD_TERM_WINDOW:
					// The window is being hidden or closed, clean it up.
					//engine_term_dpy(engine);
					break;
				case APP_CMD_GAINED_FOCUS:
					// When our app gains focus, we start monitoring the accelerometer.
					if (engine->accelerometerSensor != NULL) {
						ASensorEventQueue_enableSensor(engine->sensorEventQueue,
								engine->accelerometerSensor);
						// We'd like to get 60 events per second (in us).
						ASensorEventQueue_setEventRate(engine->sensorEventQueue,
								engine->accelerometerSensor, (1000L/60)*1000);
					}
					break;
				case APP_CMD_LOST_FOCUS:
					// When our app loses focus, we stop monitoring the accelerometer.
					// This is to avoid consuming battery while not being used.
					if (engine->accelerometerSensor != NULL) {
						ASensorEventQueue_disableSensor(engine->sensorEventQueue,
								engine->accelerometerSensor);
					}
					// Also stop animating.
					engine->animating = 0;
					engine_draw_frame(engine);
					break;
			}*/
		}
	};

	struct window : widget
	{
		EGLSurface surface;

		application& app() { return *(application*)parent; }

		window(application& app, const char*) : surface(eglCreateWindowSurface(app.dpy, app.config, aapp->window, 0))
		{
			parent = &app;
			app.children.push_back(this);
		}

		~window()
		{
			eglMakeCurrent(app().dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			eglDestroySurface(app().dpy, surface);
		}

		void show(bool) {}
		void fullscreen(bool) {}
		void togglefullscreen() {}
		void flip() { eglSwapBuffers(app().dpy, surface); }
	};
}
