/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <jni.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include "android_native_app_glue.h"
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "threaded_app", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "threaded_app", __VA_ARGS__))

/* For debug builds, always enable the debug traces in this library */
#ifndef NDEBUG
#  define LOGV(...)  ((void)__android_log_print(ANDROID_LOG_VERBOSE, "threaded_app", __VA_ARGS__))
#else
#  define LOGV(...)  ((void)0)
#endif

static void free_saved_state(struct android_app* android_app)
{
	pthread_mutex_lock(&android_app->mutex);
	if (android_app->savedState != NULL)
	{
		free(android_app->savedState);
		android_app->savedState = NULL;
		android_app->savedStateSize = 0;
	}
	pthread_mutex_unlock(&android_app->mutex);
}

int8_t android_app_read_cmd(struct android_app* android_app)
{
	int8_t cmd;
	if (read(android_app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd))
	{
		switch (cmd)
		{
			case APP_CMD_SAVE_STATE:
				free_saved_state(android_app);
				break;
		}
		return cmd;
	} else {
		LOGE("No data on command pipe!");
	}
	return -1;
}

static void print_cur_config(struct android_app* android_app)
{
	char lang[2], country[2];
	AConfiguration_getLanguage(android_app->config, lang);
	AConfiguration_getCountry(android_app->config, country);

	LOGV("Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d "
		 "keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d "
		 "modetype=%d modenight=%d",
		 AConfiguration_getMcc(android_app->config),
		 AConfiguration_getMnc(android_app->config),
		 lang[0], lang[1], country[0], country[1],
			AConfiguration_getOrientation(android_app->config),
			AConfiguration_getTouchscreen(android_app->config),
			AConfiguration_getDensity(android_app->config),
			AConfiguration_getKeyboard(android_app->config),
			AConfiguration_getNavigation(android_app->config),
			AConfiguration_getKeysHidden(android_app->config),
			AConfiguration_getNavHidden(android_app->config),
			AConfiguration_getSdkVersion(android_app->config),
			AConfiguration_getScreenSize(android_app->config),
			AConfiguration_getScreenLong(android_app->config),
			AConfiguration_getUiModeType(android_app->config),
			AConfiguration_getUiModeNight(android_app->config));
}

void app_dummy()
{

}

static void android_app_destroy(struct android_app* android_app)
{
	LOGV("android_app_destroy!");
	free_saved_state(android_app);
	pthread_mutex_lock(&android_app->mutex);
	if (android_app->inputQueue != NULL)
	{
		AInputQueue_detachLooper(android_app->inputQueue);
	}
	AConfiguration_delete(android_app->config);
	android_app->destroyed = 1;
	pthread_cond_broadcast(&android_app->cond);
	pthread_mutex_unlock(&android_app->mutex);
	// Can't touch android_app object after this.
}

static void process_input(struct android_app* app, struct android_poll_source* source)
{
	AInputEvent* event = NULL;
	if (AInputQueue_getEvent(app->inputQueue, &event) >= 0)
	{
		LOGV("New input event: type=%d\n", AInputEvent_getType(event));
		if (AInputQueue_preDispatchEvent(app->inputQueue, event))
		{
			return;
		}
		int32_t handled = 0;
		if (app->onInputEvent != NULL) handled = app->onInputEvent(app, event);
		AInputQueue_finishEvent(app->inputQueue, event, handled);
	} else {
		LOGE("Failure reading next input event: %s\n", strerror(errno));
	}
}

static void process_cmd(struct android_app* app, struct android_poll_source* source)
{
	int8_t cmd = android_app_read_cmd(app);
	switch(cmd)
	{
		case APP_CMD_INPUT_CHANGED:
			LOGV("APP_CMD_INPUT_CHANGED\n");
			pthread_mutex_lock(&app->mutex);
			if (app->inputQueue != NULL)
			{
				AInputQueue_detachLooper(app->inputQueue);
			}
			app->inputQueue = app->pendingInputQueue;
			if (app->inputQueue != NULL)
			{
				LOGV("Attaching input queue to looper");
				AInputQueue_attachLooper(app->inputQueue, app->looper, LOOPER_ID_INPUT, NULL, &app->inputPollSource);
			}
			pthread_cond_broadcast(&app->cond);
			pthread_mutex_unlock(&app->mutex);
			break;

		case APP_CMD_INIT_WINDOW:
			LOGV("APP_CMD_INIT_WINDOW\n");
			pthread_mutex_lock(&app->mutex);
			app->window = app->pendingWindow;
			pthread_cond_broadcast(&app->cond);
			pthread_mutex_unlock(&app->mutex);
			break;

		case APP_CMD_TERM_WINDOW:
			LOGV("APP_CMD_TERM_WINDOW\n");
			pthread_cond_broadcast(&app->cond);
			break;

		case APP_CMD_RESUME:
		case APP_CMD_START:
		case APP_CMD_PAUSE:
		case APP_CMD_STOP:
			LOGV("activityState=%d\n", cmd);
			pthread_mutex_lock(&app->mutex);
			app->activityState = cmd;
			pthread_cond_broadcast(&app->cond);
			pthread_mutex_unlock(&app->mutex);
			break;

		case APP_CMD_CONFIG_CHANGED:
			LOGV("APP_CMD_CONFIG_CHANGED\n");
			AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
			print_cur_config(app);
			break;

		case APP_CMD_DESTROY:
			LOGV("APP_CMD_DESTROY\n");
			app->destroyRequested = 1;
			break;
	}

	if(app->onAppCmd != NULL) app->onAppCmd(app, cmd);

	switch (cmd)
	{
		case APP_CMD_TERM_WINDOW:
			LOGV("APP_CMD_TERM_WINDOW\n");
			pthread_mutex_lock(&app->mutex);
			app->window = NULL;
			pthread_cond_broadcast(&app->cond);
			pthread_mutex_unlock(&app->mutex);
			break;

		case APP_CMD_SAVE_STATE:
			LOGV("APP_CMD_SAVE_STATE\n");
			pthread_mutex_lock(&app->mutex);
			app->stateSaved = 1;
			pthread_cond_broadcast(&app->cond);
			pthread_mutex_unlock(&app->mutex);
			break;

		case APP_CMD_RESUME:
			free_saved_state(app);
			break;
	}
}

int main();

static void* android_app_entry(void* param)
{
	struct android_app* app = (struct android_app*)param;

	app->config = AConfiguration_new();
	AConfiguration_fromAssetManager(app->config, app->activity->assetManager);

	print_cur_config(app);

	app->cmdPollSource.id = LOOPER_ID_MAIN;
	app->cmdPollSource.app = app;
	app->cmdPollSource.process = process_cmd;
	app->inputPollSource.id = LOOPER_ID_INPUT;
	app->inputPollSource.app = app;
	app->inputPollSource.process = process_input;

	ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	ALooper_addFd(looper, app->msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, &app->cmdPollSource);
	app->looper = looper;

	pthread_mutex_lock(&app->mutex);
	app->running = 1;
	pthread_cond_broadcast(&app->cond);
	pthread_mutex_unlock(&app->mutex);

	main();

	android_app_destroy(app);
	return NULL;
}

// --------------------------------------------------------------------
// Native activity interaction (called from main thread)
// --------------------------------------------------------------------

struct android_app* aapp;

static struct android_app* android_app_create(ANativeActivity* activity, void* savedState, size_t savedStateSize)
{
	aapp = (struct android_app*)malloc(sizeof(struct android_app));
	memset(aapp, 0, sizeof(struct android_app));
	aapp->activity = activity;

	pthread_mutex_init(&aapp->mutex, NULL);
	pthread_cond_init(&aapp->cond, NULL);

	if (savedState != NULL)
	{
		aapp->savedState = malloc(savedStateSize);
		aapp->savedStateSize = savedStateSize;
		memcpy(aapp->savedState, savedState, savedStateSize);
	}

	int msgpipe[2];
	if (pipe(msgpipe))
	{
		LOGE("could not create pipe: %s", strerror(errno));
		return NULL;
	}
	aapp->msgread = msgpipe[0];
	aapp->msgwrite = msgpipe[1];

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&aapp->thread, &attr, android_app_entry, aapp);

	// Wait for thread to start.
	pthread_mutex_lock(&aapp->mutex);
	while (!aapp->running)
	{
		pthread_cond_wait(&aapp->cond, &aapp->mutex);
	}
	pthread_mutex_unlock(&aapp->mutex);

	return aapp;
}

static void android_app_write_cmd(struct android_app* app, int8_t cmd)
{
	if (write(app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
	{
		LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
	}
}

static void android_app_set_input(struct android_app* app, AInputQueue* inputQueue)
{
	pthread_mutex_lock(&app->mutex);
	app->pendingInputQueue = inputQueue;
	android_app_write_cmd(app, APP_CMD_INPUT_CHANGED);
	while (app->inputQueue != app->pendingInputQueue)
	{
		pthread_cond_wait(&app->cond, &app->mutex);
	}
	pthread_mutex_unlock(&app->mutex);
}

static void android_app_set_window(struct android_app* app, ANativeWindow* window)
{
	pthread_mutex_lock(&app->mutex);
	if (app->pendingWindow != NULL)
	{
		android_app_write_cmd(app, APP_CMD_TERM_WINDOW);
	}
	app->pendingWindow = window;
	if (window != NULL)
	{
		android_app_write_cmd(app, APP_CMD_INIT_WINDOW);
	}
	while (app->window != app->pendingWindow)
	{
		pthread_cond_wait(&app->cond, &app->mutex);
	}
	pthread_mutex_unlock(&app->mutex);
}

static void android_app_set_activity_state(struct android_app* app, int8_t cmd)
{
	pthread_mutex_lock(&app->mutex);
	android_app_write_cmd(app, cmd);
	while (app->activityState != cmd)
	{
		pthread_cond_wait(&app->cond, &app->mutex);
	}
	pthread_mutex_unlock(&app->mutex);
}

static void android_app_free(struct android_app* app)
{
	pthread_mutex_lock(&app->mutex);
	android_app_write_cmd(app, APP_CMD_DESTROY);
	while (!app->destroyed)
	{
		pthread_cond_wait(&app->cond, &app->mutex);
	}
	pthread_mutex_unlock(&app->mutex);

	close(app->msgread);
	close(app->msgwrite);
	pthread_cond_destroy(&app->cond);
	pthread_mutex_destroy(&app->mutex);
	free(app);
}

static void onDestroy(ANativeActivity* activity)
{
	LOGV("Destroy: %p\n", activity);
	android_app_free((struct android_app*)activity->instance);
}

static void onStart(ANativeActivity* activity)
{
	LOGV("Start: %p\n", activity);
	android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_START);
}

static void onResume(ANativeActivity* activity)
{
	LOGV("Resume: %p\n", activity);
	android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_RESUME);
}

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen)
{
	struct android_app* app = (struct android_app*)activity->instance;
	void* savedState = NULL;

	LOGV("SaveInstanceState: %p\n", activity);
	pthread_mutex_lock(&app->mutex);
	app->stateSaved = 0;
	android_app_write_cmd(app, APP_CMD_SAVE_STATE);
	while (!app->stateSaved)
	{
		pthread_cond_wait(&app->cond, &app->mutex);
	}

	if (app->savedState != NULL)
	{
		savedState = app->savedState;
		*outLen = app->savedStateSize;
		app->savedState = NULL;
		app->savedStateSize = 0;
	}

	pthread_mutex_unlock(&app->mutex);

	return savedState;
}

static void onPause(ANativeActivity* activity)
{
	LOGV("Pause: %p\n", activity);
	android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_PAUSE);
}

static void onStop(ANativeActivity* activity)
{
	LOGV("Stop: %p\n", activity);
	android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_STOP);
}

static void onConfigurationChanged(ANativeActivity* activity)
{
	struct android_app* android_app = (struct android_app*)activity->instance;
	LOGV("ConfigurationChanged: %p\n", activity);
	android_app_write_cmd(android_app, APP_CMD_CONFIG_CHANGED);
}

static void onLowMemory(ANativeActivity* activity)
{
	struct android_app* android_app = (struct android_app*)activity->instance;
	LOGV("LowMemory: %p\n", activity);
	android_app_write_cmd(android_app, APP_CMD_LOW_MEMORY);
}

static void onWindowFocusChanged(ANativeActivity* activity, int focused)
{
	LOGV("WindowFocusChanged: %p -- %d\n", activity, focused);
	android_app_write_cmd((struct android_app*)activity->instance,
						  focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window)
{
	LOGV("NativeWindowCreated: %p -- %p\n", activity, window);
	android_app_set_window((struct android_app*)activity->instance, window);
}

static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window)
{
	LOGV("NativeWindowDestroyed: %p -- %p\n", activity, window);
	android_app_set_window((struct android_app*)activity->instance, NULL);
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue)
{
	LOGV("InputQueueCreated: %p -- %p\n", activity, queue);
	android_app_set_input((struct android_app*)activity->instance, queue);
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue)
{
	LOGV("InputQueueDestroyed: %p -- %p\n", activity, queue);
	android_app_set_input((struct android_app*)activity->instance, NULL);
}

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize)
{
	LOGV("Creating: %p\n", activity);
	activity->callbacks->onDestroy = onDestroy;
	activity->callbacks->onStart = onStart;
	activity->callbacks->onResume = onResume;
	activity->callbacks->onSaveInstanceState = onSaveInstanceState;
	activity->callbacks->onPause = onPause;
	activity->callbacks->onStop = onStop;
	activity->callbacks->onConfigurationChanged = onConfigurationChanged;
	activity->callbacks->onLowMemory = onLowMemory;
	activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
	activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
	activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
	activity->callbacks->onInputQueueCreated = onInputQueueCreated;
	activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

	activity->instance = android_app_create(activity, savedState, savedStateSize);
}
