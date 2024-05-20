#include <stdint.h>

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void printlen(char* string, int length, uint32_t color) {
    syscall(6, (uint32_t) string, length, color);
}

int main(){
    while(1){
        unsigned char hour;
        unsigned char minute;
        unsigned char second;
        syscall(13, (uint32_t)&hour, (uint32_t)&minute, (uint32_t)&second);
        printlen((char*) &second, 1, 0xF);
    }
    return 0;
}