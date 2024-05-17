#include <stdint.h>
#include "../header/stdlib/string.h"
#include "../header/user/user-shell.h"
void cd(char* argv[], int argc){
    if(argc > 2){
        put("cd: too many arguments\n", 0xF);
    }else{
        uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
        char* pathTemp = argv[1];
        char* path[16];
        int wordCount;
        parsePath(pathTemp,path,&wordCount);
        int lenPath = wordCount;
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
                print("cd: no such directory", 0xF);
                print(pathTemp, 0xF);
                print("\n", 0xF);
                return;
            }
            // entry di directory table bukan directory
            if(dirTable.table[entry].attribute != ATTR_SUBDIRECTORY){
                print("cd: not a directory\n", 0xF);
                return;
            }
            cluster_number = (int)(dirTable.table[index].cluster_high << 16) + dirTable.table[index].cluster_low;
            // get directory table
            updateDirectoryTable(cluster_number);
            index++;
        }
        search_directory_number = cluster_number;

    }
}