#include "header/user/cp.h"
#include "header/user/rm.h"
#include "header/user/user-shell.h"
#include "header/user/cd.h"
#include "header/stdlib/string.h"

void mv(char argv[4][100], int argc) {
    int temp_Path = currentDirectory;
    if (argc < 3) {
        print("cat: missing operand\n", 0b0110);
    } else if(argc > 3){
        print("cat: too much operand\n", 0b0110);
    } else{
        char pathList[16][10];
        int pathCount;
        parsePath(argv[1], pathList, &pathCount);

        char output[16][10];
        int count;
        parseExt(pathList[pathCount-1],output,&count);

        //cari filenya
        int entry =  findDirEntryIndex(output[0]);
        if(entry == -1){ //tidak ketemu
            print("Error: no such File: ", 0b0110);
            print(argv[1], 0b0110);
            print ("\n", 0xF);
            return;
        } else{
            if (dirTable.table[entry].attribute != ATTR_SUBDIRECTORY){
                cp(argv,  argc);
                rm(argv, argc-1);
            } else{
                print("Error: not a file\n", 0xF);
            }
        }
    }
    currentDirectory = temp_Path;
    return;
}