#include <stdint.h>
#include "header/stdlib/string.h"
#include "header/user/user-shell.h"
#include "header/user/cd.h"
int cd(char argv[4][100], int argc, bool isdir){
    if(argc > 2){
        print("Error: too many arguments\n", 0xF);
        return 1;
    }else{
        uint32_t startDir = currentDirectory;
        char* pathTemp = argv[1];
        char path[16][10];
        int wordCount;
        parsePath(pathTemp,path,&wordCount);
        int lenPath;
        if(isdir){
            lenPath = wordCount;
        }else{
            lenPath = wordCount-1;
        }
        int index = 0;
        // kalo ga absolute ke current
        if(isAbsolutePath(argv[1])){
            currentDirectory = ROOT_CLUSTER_NUMBER;
        }
        updateDirectoryTable(currentDirectory);
        while(index < lenPath){
            int entry = findDirEntryIndex(path[index]);
            // tidak ketemu
            if(entry == -1){
                currentDirectory = startDir;
                return 2;
            }
            // entry di directory table bukan directory
            if(dirTable.table[entry].attribute != ATTR_SUBDIRECTORY){
                currentDirectory = startDir;
                return 3;
            }
            currentDirectory = (int)(dirTable.table[entry].cluster_high << 16 | dirTable.table[entry].cluster_low);
            // get directory table
            updateDirectoryTable(currentDirectory);
            index++;
        }
    }
    return 0;
}