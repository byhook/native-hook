#include <jni.h>

#ifndef NATIVE_HOOK_HOOK_SIMPLE_H
#define NATIVE_HOOK_HOOK_SIMPLE_H
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_com_feature_hook_NativeHook_hookSimple(JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif
#endif //NATIVE_HOOK_HOOK_SIMPLE_H
