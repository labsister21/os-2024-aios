#include <stdint.h>
#include "header/stdlib/string.h"
#include "header/user/user-shell.h"
#include "header/user/cd.h"
#include "header/user/ls.h"


void ls(char* argv[], int argc){
    int curDir = currentDirectory;
    int retval = cd(argv,argc, true);
    if(retval == 0){
        for(int i = 1; i <= 63; i++){
            if(dirTable.table[i].name[7] != '\0'){
                printlen(dirTable.table[i].name, 8, 0xF);
            }else{
                print(dirTable.table[i].name, 0xF);
            }
            if(dirTable.table[i].attribute != ATTR_SUBDIRECTORY && strlen(dirTable.table[i].ext) != 0 ){
                print(".",0xF);
                if(dirTable.table[i].ext[2] != '\0'){
                    printlen(dirTable.table[i].name, 3, 0xF);
                }
            }
        }
    }
    currentDirectory = curDir;
}