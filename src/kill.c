#include "header/user/kill.h"
#include "header/user/user-shell.h"
#include "header/stdlib/string.h"

void kill(char argv[4][100], int argc){
    if (argc < 2) {
        print("exec: missing argument\n", 0xF);
        return;
    } else if (argc > 2){
        print("exec: too much arguments\n", 0xF);
        return;
    } else {
        int  i, len;
        int pid = 1;
        len = strlen(argv[1]);
        int currentNumber = 0;
        // parse number
        for(i=0; i < len; i++){
            currentNumber = (argv[1][i] - '0');
            if (currentNumber < 0 || currentNumber > 9){
                print("exec: not a number: ", 0xF);
                print(argv[1], 0xF);
                print("\n", 0xF);
                return;
            }
            pid = pid * currentNumber;
        }
        bool success;
        syscall(10, pid, (uint32_t) &success, 0);
        if (success) {
            print("kill: process killed", 0xF);
        } else {
            print("kill: process termination failed", 0xF);
        }
    }
}
