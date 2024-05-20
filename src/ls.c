#include <stdint.h>
#include "header/stdlib/string.h"
#include "header/user/user-shell.h"
#include "header/user/cd.h"
#include "header/user/ls.h"


void ls(char argv[4][100], int argc){
    int startDir = currentDirectory;
    int retval;
    if (argc == 1) {
        retval = 0;
    } else {
        retval = cd(argv,argc, true);
    }
    if(retval == 0){
        updateDirectoryTable(currentDirectory);
        for(int i = 2; i < 64; i++){
            if (dirTable.table[i].user_attribute == UATTR_NOT_EMPTY){
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
                print("\n", 0xF);
            }
        }
    }
    currentDirectory = startDir;
}