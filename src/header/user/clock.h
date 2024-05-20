#ifndef _CLOCK_
#define _CLOCK_
#include <stdint.h>
void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

void int_to_string(char str[3], int num);

#endif