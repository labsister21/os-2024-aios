#include <stdint.h>
#include "header/stdlib/string.h"
#include "header/user/user-shell.h"
#include "header/user/cp.h"
#include "header/user/mkdir.h"
#include "header/user/cd.h"

void cp(char argv[4][100], int argc){
    if(argc > 3){
        print("CP: too many arguments\n", 0xF);
        return;
    }else{
        char argvS[4][100]; 
        char argvD[4][100];
        memcpy(argvS[0], argv[0], 100);
        memcpy(argvS[1], argv[1], 100);
        memcpy(argvD[0], argv[0], 100);
        memcpy(argvD[1], argv[2], 100);
        uint32_t startDir = currentDirectory;

        // char* pathSourceTemp = argv[1];
        // char* pathDestTemp = argv[2];
        // char pathSource[16][10];
        // char pathDest[16][10];
        // int wordCountSource;
        // int wordCountDest;
        // parsePath(pathSourceTemp,pathSource,&wordCountSource);
        // parsePath(pathDestTemp,pathDest,&wordCountDest);
        mkdir(argvD,2);
        cpHelper(argvS,argvD, startDir);
        currentDirectory = startDir;
        



    }
}
void cpHelper(char argvS[4][100], char argvD[4][100], uint32_t startDir){
    int retval = cd(argvS,2,true);
    // ada di source
    if(retval == 0){
        for(int i = 2; i < 64; i++){
            if (dirTable.table[i].user_attribute == UATTR_NOT_EMPTY){
                if(dirTable.table[i].attribute == ATTR_SUBDIRECTORY){
                    
                    // tambahin yang lagi diproses ke argv
                    char argvDnew[4][100];
                    char argvSnew[4][100];
                    memcpy(argvSnew,argvS,200);
                    memcpy(argvDnew,argvD,200);

                    char tempName[10];
                    for (int j = 0; j<10; j++) {
                        tempName[j] = 0;
                    }
                    memcpy(tempName, dirTable.table[i].name, 8);
                    // tambahin dir yang lagi dibaca ke argv
                    append(argvDnew, tempName);
                    append(argvSnew, tempName);

                    // pindah ke original dir
                    currentDirectory = startDir;
                    updateDirectoryTable(currentDirectory);

                    mkdir(argvDnew,2);
                    cpHelper(argvSnew,argvDnew,startDir);
                }
                if(dirTable.table[i].attribute != ATTR_SUBDIRECTORY && strlen(dirTable.table[i].ext) != 0 ){
                    mkfile(argvD,2,dirTable.table[i].ext);
                }
                
            }
        }
    }
}

void mkfile(char argv[4][100], int argc, char ext[3]){
    if (argc > 2) {
        print("Error: too many arguments\n", 0xF);
        return;
    } else {
        int startDir = currentDirectory;
        int retval = cd((char (*)[100]) argv, argc, false);
        if (retval != 0) {
            return;
        }
        char pathList[16][10];
        int pathCount;
        parsePath(argv[1], pathList, &pathCount);
        struct FAT32DriverRequest request;
        memcpy(request.name, pathList[pathCount-1], 8);
        int filesize = 2;
        for(int i = 0; i<31;i++){
            filesize*=2;
        }
        request.buffer_size = filesize-1;
        request.parent_cluster_number = currentDirectory;
        memcpy(request.ext,ext,3); 
        retval = write(request);
        if (retval == 1) {
            print("Error: Directory exists\n", 0xF);
        } else if (retval == 2) {
            print("Error: Parent directory invalid\n", 0xF);
        } else if (retval == -1) {
            print("Error: Unknown error", 0xF);
        }
        currentDirectory = startDir;
        return;
    }

}
void append(char argv[4][100], char source[10]){
    int len = strlen(argv[1]);
    argv[1][len++] = '/';
    memcpy(&(argv[1][len]), source, strlen(source));
}
