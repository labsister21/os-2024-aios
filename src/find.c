#include <stdint.h>
#include "header/stdlib/string.h"
#include "header/user/user-shell.h"
#include "header/user/find.h"

void find(char argv[4][100], int argc){
    char* item = argv[1];
    bool visited[1000];
    char pathList[15][10];
    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 10; j++) {
            pathList[i][j] = 0;
        }
    }
    int lenPath = 1;
    bool found = false;
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

void findHelper(char* item, uint32_t curDirCluster, bool visited[], char pathList[15][10], int lenPath, bool *found){

    uint32_t originalCurDirCluster = curDirCluster;
    updateDirectoryTable(curDirCluster);

    for(int i = 2; i < 64; i++){
        if(dirTable.table[i].user_attribute == UATTR_NOT_EMPTY){
            if(dirTable.table[i].attribute == ATTR_SUBDIRECTORY){
                memcpy(
                    pathList[lenPath], dirTable.table[i].name, 8);
                lenPath++;
                if(memcmp(dirTable.table[i].name, item, 8) == 0){
                    *found = true;
                    printPathFromList(pathList, lenPath);
                }
                curDirCluster = (dirTable.table[i].cluster_high << 16) + dirTable.table[i].cluster_low;
                findHelper(item, curDirCluster, visited, pathList, lenPath, found);
                updateDirectoryTable(originalCurDirCluster);
                lenPath--;
                for (int k = 0; k < 10; k++) {
                    pathList[lenPath][k] = 0;
                }
            } else{
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

void printPathFromList(char pathList[15][10], int lenPath){
    for(int i = 0; i < lenPath; i++){
        print(pathList[i], 0xF);
        print("/", 0xF);
    }
    print("\n", 0xF);
}