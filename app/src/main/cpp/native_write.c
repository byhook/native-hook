
#include <stdio.h>
#include <string.h>
#include "native_write.h"
#include "logger.h"

/**
 * 完成写入文件的操作
 * @param path
 * @param text
 */
void writeText(const char *path, const char *text) {
    FILE *fp = NULL;
    if ((fp = fopen(path, "w")) == NULL) {
        LOG_E("file cannot open");
        return;
    }
    //写入数据
    fwrite(text, strlen(text), 1, fp);
    if (fclose(fp) != 0) {
        LOG_E("file cannot be closed");
        return;
    }
}

JNIEXPORT void JNICALL
Java_com_feature_hook_NativeHook_writeText(JNIEnv *env, jobject obj, jstring filePath,
                                           jstring jText) {
    const char *path = (*env)->GetStringUTFChars(env, filePath, 0);
    const char *text = (*env)->GetStringUTFChars(env, jText, 0);
    LOG_D("fwrite address=0x%08X", fopen);
    LOG_D("writeText address=0x%08X", writeText);
    writeText(path, text);
}