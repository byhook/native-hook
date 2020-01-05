#include <jni.h>

#ifndef NATIVE_HOOK_NATIVE_VALUE_H
#define NATIVE_HOOK_NATIVE_VALUE_H
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_feature_hook_NativeHook_writeText(JNIEnv *env, jobject obj, jstring filePath,
                                           jstring jText);

#ifdef __cplusplus
}
#endif
#endif //NATIVE_HOOK_NATIVE_VALUE_H
