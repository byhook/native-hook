#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <inttypes.h>
#include <sys/mman.h>
#include "hook_simple.h"
#include "logger.h"

#define PAGE_START(addr) ((addr) & PAGE_MASK)
#define PAGE_END(addr)   (PAGE_START(addr) + PAGE_SIZE)

/**
 * hook导出表函数
 * @param buf
 * @param size
 * @param count
 * @param fp
 * @return
 */
size_t hook_fwrite(const void *buf, size_t size, size_t count, FILE *fp) {
    LOG_D("hook fwrite success");
    //这里插入一段文本
    const char *text = "hello ";
    fwrite(text, strlen(text), 1, fp);
    return fwrite(buf, size, count, fp);
}

/**
 * hook导出表函数
 * @param path
 * @param text
 */
void hook_writeFile(const char *path, const char *text) {
    LOG_D("hook writeFile success path=%s text=%s", path, text);
}

/**
 * hook演示操作
 * @param env
 * @param obj
 * @param jSoName
 */
void Java_com_feature_hook_NativeHook_hookSimple(JNIEnv *env, jobject obj, jstring jSoName) {
    const char *soName = (*env)->GetStringUTFChars(env, jSoName, 0);
    LOG_D("soName=%s", soName);
    char line[1024] = "\n";
    FILE *fp = NULL;
    uintptr_t base_addr = 0;
    uintptr_t addr = 0;
    //查找自身对应的基址
    if (NULL == (fp = fopen("/proc/self/maps", "r"))) return;
    while (fgets(line, sizeof(line), fp)) {
        if (NULL != strstr(line, soName) &&
            sscanf(line, "%"PRIxPTR"-%*lx %*4s 00000000", &base_addr) == 1)
            break;
    }
    fclose(fp);
    LOG_D("base address=0x%08X", base_addr);
    if (0 == base_addr) return;
    //基址+偏移=真实的地址
    addr = base_addr + 0x2FE0;
    LOG_D("value=0x%08X address=0x%08X fwrite=0x%08X", addr, *(uintptr_t *) addr, fwrite);
    //调整写权限
    mprotect((void *) PAGE_START(addr), PAGE_SIZE, PROT_READ | PROT_WRITE);
    //替换目标地址
    *(void **) addr = hook_fwrite;
    //清除指令缓存
    __builtin___clear_cache((void *) PAGE_START(addr), (void *) PAGE_END(addr));
}