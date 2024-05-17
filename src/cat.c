#include "../header/user/user-shell.h"
#include "cd.c"

void cat (char* argv, int argc) {
    int temp_Path = currentDirectory;
    if (argc < 2) {
        print("cat: missing operand\n", 0b0110);
    } else if(argc > 2){
        print("cat: too much operand\n", 0b0110);
    } else{
        int resultCD = cd(argv, argc, false);
        if (resultCD == 0){ //kalo berhasil cd nya
            bool file_Found = false;
            char pathList[10][16];
            int pathCount;
            parsePath(argv[1], pathList, &pathCount);
            char lastPath = pathList[pathCount-1]; //cari nama file

            //cari filenya
            int entry =  findDirEntryIndex(lastPath);
            if(entry == -1){ //tidak ketemu
                print("Error: no such directory", 0b0110);
                print(argv[1], 0b0110);
            } else{
                if (dirTable.table[entry].attribute != ATTR_SUBDIRECTORY){ //file
                    char output[10][16];
                    int count;
                    parseExt(lastPath,output,&count);
                    struct ClusterBuffer clusterBuff = {0};
                    struct FAT32DriverRequest request = {
                        .buf = &clusterBuff,
                        .name = "\0\0\0\0\0\0\0",
                        .ext = "\0\0\0",
                        .parent_cluster_number = currentDirectory,
                        .buffer_size = 4 * CLUSTER_SIZE,
                    };
                    memcpy(&(request.name), output[0], 8);
                    memcpy(&(request.ext), output[1], 3);

                    int32_t retcode;
                    syscall(0,(uint32_t) &request, (uint32_t) &retcode, 0x0);
                    if (retcode != 0) {
                        print("cat: '", 0b0110);
                        switch (retcode) {
                            case 1:
                                print("Is a directory\n", 0b0110);
                                break;
                            case 2:
                                print("Buffer size is not enough\n", 0b0110);
                                break;
                            case 3:
                                print("No such file or directory\n", 0b0110);
                                break;
                        }
                    } else {
                        print((char *) &clusterBuff, 0xF);
                    }
                } else{
                    print("Error: not a file", 0b0110);
                }
            }
        }
    }
    currentDirectory = temp_Path;

}