#include <jni.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <poll.h>
#include <pthread.h>
#include <sched.h>

#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/log.h>

extern "C"
{
struct android_app;

/**
 * Data associated with an ALooper fd that will be returned as the "outData"
 * when that source has data ready.
 */
struct android_poll_source
{
	// The identifier of this source.  May be LOOPER_ID_MAIN or LOOPER_ID_INPUT
	int32_t id;

	// The android_app this ident is associated with.
	struct android_app* app;

	// Function to call to perform the standard processing of data from
	// this source.
	void (*process)(struct android_app* app, struct android_poll_source* source);
};

/**
 * This is the interface for the standard glue code of a threaded
 * application.  In this model, the application's code is running
 * in its own thread separate from the main thread of the process.
 * It is not required that this thread be associated with the Java
 * VM, although it will need to be in order to make JNI calls any
 * Java objects.
 */
struct android_app
{
	// The application can place a pointer to its own state object
	// here if it likes.
	void* userData;

	// Fill this in with the function to process main app commands (APP_CMD_*)
	void (*onAppCmd)(struct android_app* app, int32_t cmd);

	// Fill this in with the function to process input events.  At this point
	// the event has already been pre-dispatched, and it will be finished upon
	// return.  Return 1 if you have handled the event, 0 for any default
	// dispatching.
	int32_t (*onInputEvent)(struct android_app* app, AInputEvent* event);

	// The ANativeActivity object instance that this app is running in.
	ANativeActivity* activity;

	// The current configuration the app is running in.
	AConfiguration* config;

	// This is the last instance's saved state, as provided at creation time.
	// It is NULL if there was no state.  You can use this as you need; the
	// memory will remain around until you call android_app_exec_cmd() for
	// APP_CMD_RESUME, at which point it will be freed and savedState set to NULL.
	// These variables should only be changed when processing a APP_CMD_SAVE_STATE,
	// at which point they will be initialized to NULL and you can malloc your
	// state and place the information here.  In that case the memory will be
	// freed for you later.
	void* savedState;
	size_t savedStateSize;

	// The ALooper associated with the app's thread.
	ALooper* looper;

	// When non-NULL, this is the input queue from which the app will
	// receive user input events.
	AInputQueue* inputQueue;

	// When non-NULL, this is the window surface that the app can draw in.
	ANativeWindow* window;

	// Current content rectangle of the window; this is the area where the
	// window's content should be placed to be seen by the user.
	ARect contentRect;

	// Current state of the app's activity.  May be either APP_CMD_START,
	// APP_CMD_RESUME, APP_CMD_PAUSE, or APP_CMD_STOP; see below.
	int activityState;

	// This is non-zero when the application's NativeActivity is being
	// destroyed and waiting for the app thread to complete.
	int destroyRequested;

	// -------------------------------------------------
	// Below are "private" implementation of the glue code.

	pthread_mutex_t mutex;
	pthread_cond_t cond;

	int msgread;
	int msgwrite;

	pthread_t thread;

	struct android_poll_source cmdPollSource;
	struct android_poll_source inputPollSource;

	int running;
	int stateSaved;
	int destroyed;
	int redrawNeeded;

	AInputQueue* pendingInputQueue;
	ANativeWindow* pendingWindow;
	ARect pendingContentRect;
};

enum
{
	/**
	 * Looper data ID of commands coming from the app's main thread, which
	 * is returned as an identifier from ALooper_pollOnce().  The data for this
	 * identifier is a pointer to an android_poll_source structure.
	 * These can be retrieved and processed with android_app_read_cmd()
	 * and android_app_exec_cmd().
	 */
	LOOPER_ID_MAIN = 1,
	/**
	 * Looper data ID of events coming from the AInputQueue of the
	 * application's window, which is returned as an identifier from
	 * ALooper_pollOnce().  The data for this identifier is a pointer to an
	 * android_poll_source structure.  These can be read via the inputQueue
	 * object of android_app.
	 */
	LOOPER_ID_INPUT = 2,
	/**
	 * Start of user-defined ALooper identifiers.
	 */
	LOOPER_ID_USER = 3
};

enum {
	/**
	 * Command from main thread: the AInputQueue has changed.  Upon processing
	 * this command, android_app->inputQueue will be updated to the new queue
	 * (or NULL).
	 */
	APP_CMD_INPUT_CHANGED,

	/**
	 * Command from main thread: a new ANativeWindow is ready for use.  Upon
	 * receiving this command, android_app->window will contain the new window
	 * surface.
	 */
	APP_CMD_INIT_WINDOW,

	/**
	 * Command from main thread: the existing ANativeWindow needs to be
	 * terminated.  Upon receiving this command, android_app->window still
	 * contains the existing window; after calling android_app_exec_cmd
	 * it will be set to NULL.
	 */
	APP_CMD_TERM_WINDOW,

	/**
	 * Command from main thread: the current ANativeWindow has been resized.
	 * Please redraw with its new size.
	 */
	APP_CMD_WINDOW_RESIZED,

	/**
	 * Command from main thread: the system needs that the current ANativeWindow
	 * be redrawn.  You should redraw the window before handing this to
	 * android_app_exec_cmd() in order to avoid transient drawing glitches.
	 */
	APP_CMD_WINDOW_REDRAW_NEEDED,

	/**
	 * Command from main thread: the content area of the window has changed,
	 * such as from the soft input window being shown or hidden.  You can
	 * find the new content rect in android_app::contentRect.
	 */
	APP_CMD_CONTENT_RECT_CHANGED,

	/**
	 * Command from main thread: the app's activity window has gained
	 * input focus.
	 */
	APP_CMD_GAINED_FOCUS,

	/**
	 * Command from main thread: the app's activity window has lost
	 * input focus.
	 */
	APP_CMD_LOST_FOCUS,

	/**
	 * Command from main thread: the current device configuration has changed.
	 */
	APP_CMD_CONFIG_CHANGED,

	/**
	 * Command from main thread: the system is running low on memory.
	 * Try to reduce your memory use.
	 */
	APP_CMD_LOW_MEMORY,

	/**
	 * Command from main thread: the app's activity has been started.
	 */
	APP_CMD_START,

	/**
	 * Command from main thread: the app's activity has been resumed.
	 */
	APP_CMD_RESUME,

	/**
	 * Command from main thread: the app should generate a new saved state
	 * for itself, to restore from later if needed.  If you have saved state,
	 * allocate it with malloc and place it in android_app.savedState with
	 * the size in android_app.savedStateSize.  The will be freed for you
	 * later.
	 */
	APP_CMD_SAVE_STATE,

	/**
	 * Command from main thread: the app's activity has been paused.
	 */
	APP_CMD_PAUSE,

	/**
	 * Command from main thread: the app's activity has been stopped.
	 */
	APP_CMD_STOP,

	/**
	 * Command from main thread: the app's activity is being destroyed,
	 * and waiting for the app thread to clean up and exit before proceeding.
	 */
	APP_CMD_DESTROY,
};

/**
 * Call when ALooper_pollAll() returns LOOPER_ID_MAIN, reading the next
 * app command message.
 */
int8_t android_app_read_cmd(struct android_app* android_app);

/**
 * Call with the command returned by android_app_read_cmd() to do the
 * initial pre-processing of the given command.  You can perform your own
 * actions for the command after calling this function.
 */
void android_app_pre_exec_cmd(struct android_app* android_app, int8_t cmd);

/**
 * Call with the command returned by android_app_read_cmd() to do the
 * final post-processing of the given command.  You must have done your own
 * actions for the command before calling this function.
 */
void android_app_post_exec_cmd(struct android_app* android_app, int8_t cmd);

/**
 * Dummy function you can call to ensure glue code isn't stripped.
 */
void app_dummy();

/**
 * This is the function that application code must implement, representing
 * the main entry to the app.
 */
extern void android_main(struct android_app* app);

}

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

static void process_input(struct android_app* app, struct android_poll_source* source) {
	AInputEvent* event = NULL;
	if (AInputQueue_getEvent(app->inputQueue, &event) >= 0) {
		LOGV("New input event: type=%d\n", AInputEvent_getType(event));
		if (AInputQueue_preDispatchEvent(app->inputQueue, event)) {
			return;
		}
		int32_t handled = 0;
		if (app->onInputEvent != NULL) handled = app->onInputEvent(app, event);
		AInputQueue_finishEvent(app->inputQueue, event, handled);
	} else {
		LOGE("Failure reading next input event: %s\n", strerror(errno));
	}
}

static void process_cmd(struct android_app* app, struct android_poll_source* source) {
	int8_t cmd = android_app_read_cmd(app);
	android_app_pre_exec_cmd(app, cmd);
	if (app->onAppCmd != NULL) app->onAppCmd(app, cmd);
	android_app_post_exec_cmd(app, cmd);
}

static void* android_app_entry(void* param)
{
	android_app* app = (android_app*)param;

	app->config = AConfiguration_new();
	AConfiguration_fromAssetManager(app->config, app->activity->assetManager);

	app->cmdPollSource.id = LOOPER_ID_MAIN;
	app->cmdPollSource.app = app;
	app->cmdPollSource.process = process_cmd;
	app->inputPollSource.id = LOOPER_ID_INPUT;
	app->inputPollSource.app = app;
	app->inputPollSource.process = process_input;
	app->looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
	ALooper_addFd(app->looper, app->msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL, &app->cmdPollSource);

	pthread_mutex_lock(&app->mutex);
	app->running = 1;
	pthread_cond_broadcast(&app->cond);
	pthread_mutex_unlock(&app->mutex);

	android_main(app);

	free_saved_state(app);
	pthread_mutex_lock(&app->mutex);
	if(app->inputQueue != NULL) AInputQueue_detachLooper(app->inputQueue);
	AConfiguration_delete(app->config);
	app->destroyed = 1;
	pthread_cond_broadcast(&app->cond);
	pthread_mutex_unlock(&app->mutex);
	// Can't touch android_app object after this.
	return NULL;
}

static void android_app_write_cmd(struct android_app* android_app, int8_t cmd)
{
	if(write(android_app->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd))
		LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
}

static void android_app_set_input(struct android_app* android_app, AInputQueue* inputQueue) {
	pthread_mutex_lock(&android_app->mutex);
	android_app->pendingInputQueue = inputQueue;
	android_app_write_cmd(android_app, APP_CMD_INPUT_CHANGED);
	while (android_app->inputQueue != android_app->pendingInputQueue) {
		pthread_cond_wait(&android_app->cond, &android_app->mutex);
	}
	pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_set_window(struct android_app* android_app, ANativeWindow* window) {
	pthread_mutex_lock(&android_app->mutex);
	if (android_app->pendingWindow != NULL) {
		android_app_write_cmd(android_app, APP_CMD_TERM_WINDOW);
	}
	android_app->pendingWindow = window;
	if (window != NULL) {
		android_app_write_cmd(android_app, APP_CMD_INIT_WINDOW);
	}
	while (android_app->window != android_app->pendingWindow) {
		pthread_cond_wait(&android_app->cond, &android_app->mutex);
	}
	pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_set_activity_state(struct android_app* android_app, int8_t cmd) {
	pthread_mutex_lock(&android_app->mutex);
	android_app_write_cmd(android_app, cmd);
	while (android_app->activityState != cmd)
		pthread_cond_wait(&android_app->cond, &android_app->mutex);
	pthread_mutex_unlock(&android_app->mutex);
}

static void android_app_free(struct android_app* android_app) {
	pthread_mutex_lock(&android_app->mutex);
	android_app_write_cmd(android_app, APP_CMD_DESTROY);
	while (!android_app->destroyed) {
		pthread_cond_wait(&android_app->cond, &android_app->mutex);
	}
	pthread_mutex_unlock(&android_app->mutex);

	close(android_app->msgread);
	close(android_app->msgwrite);
	pthread_cond_destroy(&android_app->cond);
	pthread_mutex_destroy(&android_app->mutex);
	delete android_app;
}

static void onDestroy(ANativeActivity* activity) {
	LOGV("Destroy: %p\n", activity);
	android_app_free((struct android_app*)activity->instance);
}

static void onStart(ANativeActivity* activity) {
	LOGV("Start: %p\n", activity);
	android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_START);
}

static void onResume(ANativeActivity* activity) {
	LOGV("Resume: %p\n", activity);
	android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_RESUME);
}

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen)
{
	struct android_app* android_app = (struct android_app*)activity->instance;
	void* savedState = NULL;

	LOGV("SaveInstanceState: %p\n", activity);
	pthread_mutex_lock(&android_app->mutex);
	android_app->stateSaved = 0;
	android_app_write_cmd(android_app, APP_CMD_SAVE_STATE);
	while(!android_app->stateSaved)
		pthread_cond_wait(&android_app->cond, &android_app->mutex);

	if (android_app->savedState != NULL)
	{
		savedState = android_app->savedState;
		*outLen = android_app->savedStateSize;
		android_app->savedState = NULL;
		android_app->savedStateSize = 0;
	}

	pthread_mutex_unlock(&android_app->mutex);

	return savedState;
}

static void onPause(ANativeActivity* activity) {
	LOGV("Pause: %p\n", activity);
	android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_PAUSE);
}

static void onStop(ANativeActivity* activity) {
	LOGV("Stop: %p\n", activity);
	android_app_set_activity_state((struct android_app*)activity->instance, APP_CMD_STOP);
}

static void onConfigurationChanged(ANativeActivity* activity) {
	struct android_app* android_app = (struct android_app*)activity->instance;
	LOGV("ConfigurationChanged: %p\n", activity);
	android_app_write_cmd(android_app, APP_CMD_CONFIG_CHANGED);
}

static void onLowMemory(ANativeActivity* activity) {
	struct android_app* android_app = (struct android_app*)activity->instance;
	LOGV("LowMemory: %p\n", activity);
	android_app_write_cmd(android_app, APP_CMD_LOW_MEMORY);
}

static void onWindowFocusChanged(ANativeActivity* activity, int focused) {
	LOGV("WindowFocusChanged: %p -- %d\n", activity, focused);
	android_app_write_cmd((struct android_app*)activity->instance, focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window) {
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

int8_t android_app_read_cmd(struct android_app* android_app)
{
	int8_t cmd;
	if(read(android_app->msgread, &cmd, sizeof(cmd)) == sizeof(cmd))
	{
		switch(cmd)
		{
			case APP_CMD_SAVE_STATE: free_saved_state(android_app); break;
		}
		return cmd;
	}
	else
	{
		LOGE("No data on command pipe!");
	}
	return -1;
}

void android_app_pre_exec_cmd(struct android_app* android_app, int8_t cmd)
{
	switch (cmd)
	{
		case APP_CMD_INPUT_CHANGED:
			LOGV("APP_CMD_INPUT_CHANGED\n");
			pthread_mutex_lock(&android_app->mutex);
			if(android_app->inputQueue != NULL)
				AInputQueue_detachLooper(android_app->inputQueue);
			android_app->inputQueue = android_app->pendingInputQueue;
			if(android_app->inputQueue != NULL)
				LOGV("Attaching input queue to looper");
			AInputQueue_attachLooper(android_app->inputQueue,
									 android_app->looper, LOOPER_ID_INPUT, NULL,
									 &android_app->inputPollSource);
			pthread_cond_broadcast(&android_app->cond);
			pthread_mutex_unlock(&android_app->mutex);
			break;

		case APP_CMD_INIT_WINDOW:
			LOGV("APP_CMD_INIT_WINDOW\n");
			pthread_mutex_lock(&android_app->mutex);
			android_app->window = android_app->pendingWindow;
			pthread_cond_broadcast(&android_app->cond);
			pthread_mutex_unlock(&android_app->mutex);
			break;

		case APP_CMD_TERM_WINDOW:
			LOGV("APP_CMD_TERM_WINDOW\n");
			pthread_cond_broadcast(&android_app->cond);
			break;

		case APP_CMD_RESUME: case APP_CMD_START: case APP_CMD_PAUSE: case APP_CMD_STOP:
			LOGV("activityState=%d\n", cmd);
			pthread_mutex_lock(&android_app->mutex);
			android_app->activityState = cmd;
			pthread_cond_broadcast(&android_app->cond);
			pthread_mutex_unlock(&android_app->mutex);
			break;

		case APP_CMD_CONFIG_CHANGED:
			LOGV("APP_CMD_CONFIG_CHANGED\n");
			AConfiguration_fromAssetManager(android_app->config, android_app->activity->assetManager);
			break;

		case APP_CMD_DESTROY:
			LOGV("APP_CMD_DESTROY\n");
			android_app->destroyRequested = 1;
			break;
	}
}

void android_app_post_exec_cmd(struct android_app* android_app, int8_t cmd) {
	switch (cmd) {
		case APP_CMD_TERM_WINDOW:
			LOGV("APP_CMD_TERM_WINDOW\n");
			pthread_mutex_lock(&android_app->mutex);
			android_app->window = NULL;
			pthread_cond_broadcast(&android_app->cond);
			pthread_mutex_unlock(&android_app->mutex);
			break;

		case APP_CMD_SAVE_STATE:
			LOGV("APP_CMD_SAVE_STATE\n");
			pthread_mutex_lock(&android_app->mutex);
			android_app->stateSaved = 1;
			pthread_cond_broadcast(&android_app->cond);
			pthread_mutex_unlock(&android_app->mutex);
			break;

		case APP_CMD_RESUME:
			free_saved_state(android_app);
			break;
	}
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

	android_app* app = new android_app;
	memset(app, 0, sizeof(android_app));
	app->activity = activity;

	pthread_mutex_init(&app->mutex, NULL);
	pthread_cond_init(&app->cond, NULL);

	if(savedState != NULL)
	{
		app->savedState = malloc(savedStateSize);
		app->savedStateSize = savedStateSize;
		memcpy(app->savedState, savedState, savedStateSize);
	}

	int msgpipe[2];
	if(pipe(msgpipe))
	{
		LOGE("could not create pipe: %s", strerror(errno));
		return NULL;
	}
	app->msgread = msgpipe[0];
	app->msgwrite = msgpipe[1];

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&app->thread, &attr, android_app_entry, app);

	// Wait for thread to start.
	pthread_mutex_lock(&app->mutex);
	while(!app->running) pthread_cond_wait(&app->cond, &app->mutex);
	pthread_mutex_unlock(&app->mutex);

	activity->instance = app;
}
