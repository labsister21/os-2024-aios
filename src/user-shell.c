#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "stdlib/string.c"
#include "framebuffer.c"

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void print(char* string, uint32_t color) {
    puts(string, strlen(string), color);
}

int main(void) {
    struct ClusterBuffer      cl[2]   = {0};
    struct FAT32DriverRequest request = {
        .buf                   = &cl,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = CLUSTER_SIZE,
    };
    int32_t retcode;
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    if (retcode == 0)
        syscall(6, (uint32_t) "owo\n", 4, 0xF);

    char line[100];
    for (int i = 0; i < 100; i++) {
        line[i] = 0;
    }
    int linelen;
    char buf;
    syscall(7, 0, 0, 0);
    while (true) {
        buf = 0;
        linelen = 0;
        while (buf != '\n') {
            linelen++;
            line[linelen] = buf;
            syscall(4, (uint32_t) &buf, 0, 0);
            syscall(5, (uint32_t) &buf, 0xF, 0);
        }
        buf = '\0';
    }

    return 0;
}



// int main(void) {
//     __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(0xDEADBEEF));
//     return 0;
// }

