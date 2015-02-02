#pragma once
#ifdef ANDROID
#include <android/log.h>
#define logdbg(...) __android_log_print(ANDROID_LOG_DEBUG, "wheel", __VA_ARGS__)
#define loginf(...) __android_log_print(ANDROID_LOG_INFO, "wheel", __VA_ARGS__)
#define logerr(...) __android_log_print(ANDROID_LOG_ERROR, "wheel", __VA_ARGS__)
#else
#include <stdio.h>
#define logdbg(...) do { printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while(0)
#define loginf(...) do { printf(__VA_ARGS__); printf("\n"); fflush(stdout); } while(0)
#define logerr(...) do { fprintf(stderr, __VA_ARGS__); printf("\n"); fflush(stderr); } while(0)
#endif
