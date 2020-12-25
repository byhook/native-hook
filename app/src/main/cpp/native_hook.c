#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <jni.h>
#include "native_hook.h"

#include <android/log.h>
#include <inttypes.h>
#include <dlfcn.h>

#define LOG_TAG    "NativeHook"
#define LOG_E(format, ...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, format, ##__VA_ARGS__)
#define LOG_D(format, ...)  __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, format, ##__VA_ARGS__)

#define PAGE_START(address) ((address) & PAGE_MASK)
#define PAGE_END(address)   (PAGE_START(address) + PAGE_SIZE)

size_t (*old_fwrite)(const void *buf, size_t size, size_t count, FILE *fp);

/**
 * hook导出表函数
 * @param buf
 * @param size
 * @param count
 * @param fp
 * @return
 */
size_t new_fwrite(const void *buf, size_t size, size_t count, FILE *fp) {
    //这里插入一段文本
    const char *text = "hello ";
    LOG_D("hook fwrite success insert text: %s", text);
    old_fwrite(text, strlen(text), 1, fp);
    return old_fwrite(buf, size, count, fp);
}

void *get_module_base(pid_t pid, const char *module_name) {
    FILE *fp;
    long addr = 0;
    char filename[32] = "\n";
    char line[1024] = "\n";
    LOG_D("pid=%d ", pid);
    if (pid < 0) {
        snprintf(filename, sizeof(filename), "/proc/self/maps");
    } else {
        snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
    }
    // 获取指定pid进程加载的内存模块信息
    fp = fopen(filename, "r");
    while (fgets(line, sizeof(line), fp)) {
        if (NULL != strstr(line, module_name) &&
            sscanf(line, "%"PRIxPTR"-%*lx %*4s 00000000", &addr) == 1)
            break;
    }
    fclose(fp);
    return (void *) addr;
}

/**
 * 执行视图获取
 * @param soPath
 * @return
 */
int hook_fwrite(const char *soPath) {
    //1. 获取目标进程中模块的加载地址
    void *base_addr = get_module_base(getpid(), soPath);
    LOG_D("base address = 0x%08X", base_addr);
    //2. 保存被hook的目标函数的原始调用地址
    old_fwrite = fwrite;
    LOG_D("origin fopen = 0x%08X", old_fwrite);
    //3. 计算program header table实际地址
    Elf32_Ehdr *header = (Elf32_Ehdr *) (base_addr);
    LOG_D("e_ident = %d", header->e_ident[0]);
    if (memcmp(header->e_ident, "\177ELF", 4) != 0) {
        return 0;
    }
    //基址 + Elf32_Off e_phoff_PROGRAM_HEADER_OFFSET_IN_FILE(e_phoff)的值 = program_header_table的地址
    Elf32_Phdr *phdr_table = (Elf32_Phdr *) (base_addr + header->e_phoff);
    if (phdr_table == 0) {
        LOG_D("phdr_table address : 0");
        return 0;
    }
    size_t phdr_count = header->e_phnum;
    LOG_D("phdr_count : %d", phdr_count);
    //遍历program header table p_type等于PT_DYNAMIC即为dynameic,获取到p_offset
    unsigned long p_vaddr = 0;
    unsigned int p_memsz = 0;
    int j = 0;
    for (j = 0; j < phdr_count; j++) {
        if (phdr_table[j].p_type == PT_DYNAMIC) {
            LOG_D("phdr_table[j].p_vaddr : %x", phdr_table[j].p_vaddr);
            p_vaddr = phdr_table[j].p_vaddr + base_addr;
            p_memsz = phdr_table[j].p_memsz;
            break;
        }
    }
    LOG_D("p_vaddr : %x", p_vaddr);
    LOG_D("p_memsz : %x", p_memsz);
    Elf32_Dyn *dynamic_table = (Elf32_Dyn *) (p_vaddr);
    unsigned long jmpRelOff = 0;
    unsigned long strTabOff = 0;
    unsigned long pltRelSz = 0;
    unsigned long symTabOff = 0;
    int dynCount = p_memsz / sizeof(Elf32_Dyn);
    for (int i = 0; i < dynCount; i++) {
        int val = dynamic_table[i].d_un.d_val;
        switch (dynamic_table[i].d_tag) {
            case DT_JMPREL:
                jmpRelOff = val;
                break;
            case DT_STRTAB:
                strTabOff = val;
                break;
            case DT_PLTRELSZ:
                pltRelSz = val / sizeof(Elf32_Rel);
                break;
            case DT_SYMTAB:
                symTabOff = val;
                break;
        }
    }
    Elf32_Rel *rel_table = (Elf32_Rel *) (jmpRelOff + base_addr);
    LOG_D("jmpRelOff : %x", jmpRelOff);
    LOG_D("strTabOff : %x", strTabOff);
    LOG_D("symTabOff : %x", symTabOff);
    //遍历查找要hook的导入函数
    for (int i = 0; i < pltRelSz; i++) {
        uint16_t ndx = ELF32_R_SYM(rel_table[i].r_info);
        Elf32_Sym *symTableIndex = (Elf32_Sym *) (ndx * sizeof(Elf32_Sym) + symTabOff +
                                                  base_addr);
        char *funcName = (char *) (symTableIndex->st_name + strTabOff + base_addr);
        if (memcmp(funcName, "fwrite", strlen("fwrite")) == 0) {
            //获取当前内存分页的大小
            uint32_t page_size = getpagesize();
            //获取内存分页的起始地址(需要内存对齐)
            uint32_t mem_page_start = rel_table[i].r_offset + base_addr;
            LOG_D("old_function=0x%08X new_function=0x%08X", mem_page_start, new_fwrite);
            LOG_D("mem_page_start=0x%08X, page size=%d", mem_page_start, page_size);
            mprotect((uint32_t) PAGE_START(mem_page_start), page_size,
                     PROT_READ | PROT_WRITE | PROT_EXEC);
            //完成替换操作
            *(unsigned int *) (rel_table[i].r_offset + base_addr) = new_fwrite;
            //清除指令缓存
            __builtin___clear_cache((void *) PAGE_START(mem_page_start),
                                    (void *) PAGE_END(mem_page_start));
        }
    }
    return 0;
}

JNIEXPORT void JNICALL
Java_com_feature_hook_NativeHook_hookWrite(JNIEnv *env, jobject obj, jstring jSoPath) {
    const char *soPath = (*env)->GetStringUTFChars(env, jSoPath, 0);
    LOG_D("hook soPath=%s", soPath);
    hook_fwrite(soPath);
}