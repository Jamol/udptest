#include <stdio.h>
#include <stdint.h>
#include <jni.h>
#include <android/log.h>
#include <stdbool.h>

#include <android/log.h>
#define MY_TAG  "udptest"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, MY_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, MY_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, MY_TAG, __VA_ARGS__)

extern uint64_t getTickCount_t();
extern void startTest(const char* bind_addr, unsigned short bind_port, const char* peer_addr, unsigned short peer_port,
                      unsigned int bw, unsigned int slice_ms, unsigned int burst_bytes);
extern void stopTest();
extern void waitTest();

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
    return JNI_VERSION_1_4;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved)
{
}

JNIEXPORT jlong JNICALL Java_com_jamol_udpclient_app_MainActivity_getTickCount1(JNIEnv *env, jclass clazz)
{
    return  0;//getTickCount_1();
}

JNIEXPORT jlong JNICALL Java_com_jamol_udpclient_app_MainActivity_getTickCount2(JNIEnv *env, jclass clazz)
{
    return  0;//getTickCount_2();
}

#include <sys/times.h>
JNIEXPORT void JNICALL Java_com_jamol_udpclient_app_MainActivity_startTest(JNIEnv *env, jclass clazz,
                                                                       jstring bindAddr, jint bindPort,
                                                                       jstring peerAddr, jint peerPort,
                                                                       jint bandwidth, jint slice_ms,
                                                                       jint burstBytes)
{
    int64_t now = times(NULL);
    LOGI("now: %lld", now);
    const char *bind_addr = (*env)->GetStringUTFChars(env, bindAddr, 0);
    const char *peer_addr = (*env)->GetStringUTFChars(env, peerAddr, 0);
    startTest(bind_addr, bindPort, peer_addr, peerPort, bandwidth, slice_ms, burstBytes);
    (*env)->ReleaseStringUTFChars(env, bindAddr, bind_addr);
    (*env)->ReleaseStringUTFChars(env, peerAddr, peer_addr);
}

JNIEXPORT void JNICALL Java_com_jamol_udpclient_app_MainActivity_stopTest(JNIEnv *env, jclass clazz)
{
    stopTest();
    waitTest();
    LOGI("test stopped");
}
