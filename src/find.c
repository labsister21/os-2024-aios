#include <stdint.h>
#include "header/stdlib/string.h"
#include "header/user/user-shell.h"
#include "header/user/find.h"

void find(char* argv[], int argc){
    char* item = argv[1];
    bool visited[1000];
    char* pathList[1000];
    int lenPath = 1;
    pathList[0] = "/";
    bool found = false;
    uint32_t curDir = currentDirectory;
    uint32_t curDirCluster = ROOT_CLUSTER_NUMBER;
    
    if(argc > 2){
        print("Find: wrong argument count", 0xF);
        return;
    }
    findHelper(item, curDirCluster, visited, pathList, lenPath, &found);
    if(!found){
        print("Find: gak ketemu cok!!!", 0xF);
    }
}

void findHelper(char* item, uint32_t curDirCluster, bool visited[], char* pathList[], int lenPath, bool *found){
    updateDirectoryTable(curDirCluster);
    uint32_t originalCurDirCluster = curDirCluster;

    for(int i = 1; i <= 63; i++){
        if(dirTable.table[i].user_attribute == UATTR_NOT_EMPTY){
            if(dirTable.table[i].attribute == ATTR_SUBDIRECTORY){
                pathList[lenPath] = dirTable.table[i].name;
                lenPath++;
                if(memcmp(dirTable.table[i].name, item, 8) == 0){
                    *found = true;
                    printPathFromList(pathList, lenPath);
                    if(dirTable.table[i].name[7] != '\0'){
                        printlen(dirTable.table[i].name, 8, 0xF);
                    }else{
                        print(dirTable.table[i].name, 0xF);
                    }
                curDirCluster = (dirTable.table[i].cluster_high << 16) + dirTable.table[i].cluster_low;
                findHelper(item, curDirCluster, visited, pathList, lenPath, found);
                updateDirectoryTable(originalCurDirCluster);
                lenPath--;
                pathList[lenPath] = '\0';
                }else{
                    if(memcmp(dirTable.table[i].name, item, 8) == 0){
                        *found = true;
                        printPathFromList(pathList, lenPath);
                        if(dirTable.table[i].name[7] != '\0'){
                            printlen(dirTable.table[i].name, 8, 0xF);
                        }else{
                            print(dirTable.table[i].name, 0xF);
                        }
                    }
                }
            }
        }
    }
}

void printPathFromList(char* pathList[], int lenPath){
    for(int i = 0; i < lenPath; i++){
        print(pathList[i], 0xF);
        print("/", 0xF);
    }
}