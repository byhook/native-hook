#include <jni.h>

#ifndef NATIVE_HOOK_NATIVE_HOOK_H
#define NATIVE_HOOK_NATIVE_HOOK_H
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_feature_hook_NativeHook_hookWrite(JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif
#endif //NATIVE_HOOK_NATIVE_HOOK_H