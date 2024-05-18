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
        mkdir(argvD,2);
        cpHelper(argvS,argvD, startDir);
        currentDirectory = startDir;
        



    }
}
void cpHelper(char argvS[4][100], char argvD[4][100], uint32_t startDir){
    char pathList[16][10];
    int pathCount;
    char lastpath[10];
    for (int i = 0; i < 10; i++) {
        lastpath[i] = 0;
    }
    parsePath(argvS[1], pathList, &pathCount);
    memcpy(lastpath, pathList[pathCount-1], 8);
    int retval = cd(argvS,2,true);
    if(retval == 3){
        retval = cd(argvS, 2, false);
        for(int i = 2; i < 64; i++){
            if (dirTable.table[i].user_attribute == UATTR_NOT_EMPTY){
                if(dirTable.table[i].attribute != ATTR_SUBDIRECTORY && strlen(dirTable.table[i].ext) == 0 && (memcmp(dirTable.table[i].name, lastpath, 8) == 0)){
                    char pathList[16][10];
                    int pathCount;
                    parsePath(argvS[1], pathList, &pathCount);
                    char output[16][10];
                    int count;
                    parseExt(pathList[pathCount-1],output,&count);
                    struct ClusterBuffer clusterBuff[12] = {0};
                    struct FAT32DriverRequest request = {
                        .buf = &clusterBuff,
                        .name = "\0\0\0\0\0\0\0",
                        .ext = "\0\0\0",
                        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
                        .buffer_size = 12 * CLUSTER_SIZE,
                    };
                    memcpy(request.name, output[0], 8);
                    memcpy(request.ext, output[1], 3);
                    int32_t retcode;
                    syscall(0,(uint32_t) &request, (uint32_t) &retcode, 0x0);
                    mkfile(argvD,2,request);
                }
            }
        }
    }else if(retval == 0){
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

                    uint32_t curWorkingDir = currentDirectory;

                    
                    // pindah ke original dir
                    currentDirectory = startDir;
                    updateDirectoryTable(currentDirectory);

                    mkdir(argvDnew,2);
                    cpHelper(argvSnew,argvDnew,startDir);

                    currentDirectory =  curWorkingDir;
                    updateDirectoryTable(currentDirectory);
                }
                if(dirTable.table[i].attribute != ATTR_SUBDIRECTORY && strlen(dirTable.table[i].ext) == 0 ){
                    struct ClusterBuffer clusterBuff[12] = {0};
                    struct FAT32DriverRequest request = {
                        .buf = &clusterBuff,
                        .name = "\0\0\0\0\0\0\0",
                        .ext = "\0\0\0",
                        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
                        .buffer_size = 12 * CLUSTER_SIZE,
                    };
                    memcpy(request.name, dirTable.table[i].name, 8);
                    memcpy(request.ext, dirTable.table[i].ext, 3);
                    int32_t retcode;
                    syscall(0,(uint32_t) &request, (uint32_t) &retcode, 0x0);

                    uint32_t curWorkingDir = currentDirectory;

                    currentDirectory = startDir;
                    updateDirectoryTable(currentDirectory);

                    mkfile(argvD,2,request);

                    currentDirectory =  curWorkingDir;
                    updateDirectoryTable(currentDirectory);
                }
                
            }
        }
    }
}



void mkfile(char argv[4][100], int argc, struct FAT32DriverRequest request){
    if (argc > 2) {
        print("Error: too many arguments\n", 0xF);
        return;
    } else {
        int startDir = currentDirectory;
        int retval = cd((char (*)[100]) argv, argc,true);
        if (retval != 0) {
            return;
        }
        request.parent_cluster_number = currentDirectory;
        retval = write(request);
        if (retval == 1) {
            print("Error: File exists\n", 0xF);
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
