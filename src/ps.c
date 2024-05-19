#include "header/user/ps.h"
#include "header/user/user-shell.h"

void ps(int argc){
    if (argc > 1){
        print("ps: too much arguments\n", 0xF);
        return;
    }
    int process_list[16];
    for (int i = 0; i < 16; i++) {
        process_list[i] = -1;
    }
    syscall(12, (uint32_t) &process_list, 0, 0);
    print("Active Processes: \n", 0xF);
    char str[10];
    for (int i = 0; i < 16; i++) {
        if (process_list[i] != -1) {
            int_to_string(str, process_list[i]);
            print("pid: ", 0xF);
            print(str, 0xF);
            print("\n", 0xF);
        }
    }
}

void int_to_string(char str[10], int num){
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