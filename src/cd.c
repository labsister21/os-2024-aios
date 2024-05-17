#include <stdint.h>
#include "../header/stdlib/string.h"
#include "../header/user/user-shell.h"
#include "../header/user/cd.h"
int cd(char* argv[], int argc, bool isdir){
    if(argc > 2){
        put("Error: too many arguments\n", 0xF);
        return 1;
    }else{
        uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
        char* pathTemp = argv[1];
        char* path[16];
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
        if(!isAbsolutePath(argv[1])){
            search_directory_number = currentDirectory;
        }
        int cluster_number;
        updateDirectoryTable(search_directory_number);
        while(index < lenPath){
            int entry = findDirEntryIndex(path[index]);
            // tidak ketemu
            if(entry == -1){
                print("Error: no such directory", 0xF);
                print(pathTemp, 0xF);
                print("\n", 0xF);
                return 2;
            }
            // entry di directory table bukan directory
            if(dirTable.table[entry].attribute != ATTR_SUBDIRECTORY){
                print("Error: not a directory\n", 0xF);
                return 3;
            }
            cluster_number = (int)(dirTable.table[index].cluster_high << 16) + dirTable.table[index].cluster_low;
            // get directory table
            updateDirectoryTable(cluster_number);
            index++;
        }
        search_directory_number = cluster_number;

    }
    return 0;
}