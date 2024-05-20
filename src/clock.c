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

void int_to_string(char str[3], int num){
    int i, rem, len = 0, n;
 
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}

int main(){
    while(1){
        
        unsigned char time[3];
        syscall(13, (uint32_t)&time[2], (uint32_t)&time[1], (uint32_t)&time[0]);
        char p[3];
        time[2] = (time[2] + 7) % 24;
        for (int i = 0; i < 3; i++) {
            if (time[i] != 0) {
                int_to_string(p, time[i]);
                if (p[1] == 0) {
                    p[1] = '0';
                    syscall(14, 24, 78-i*3, (uint32_t) &p[1]);
                    syscall(14, 24, 79-i*3, (uint32_t) &p[0]);
                } else {
                    syscall(14, 24, 78-i*3, (uint32_t) &p[0]);
                    syscall(14, 24, 79-i*3, (uint32_t) &p[1]);
                }
            } else {
                p[0] = '0';
                p[1] = '0';
                syscall(14, 24, 79, (uint32_t) &p[0]);
                syscall(14, 24, 78, (uint32_t) &p[1]);
            }
        }
        p[0] = ':';
        syscall(14, 24, 77, (uint32_t) &p[0]);
        syscall(14, 24, 74, (uint32_t) &p[0]);
    }
    return 0;
}