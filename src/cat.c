#include "header/user/cat.h"
#include "header/user/user-shell.h"
#include "header/user/cd.h"
#include "header/stdlib/string.h"

void cat(char argv[4][100], int argc) {
    int temp_Path = currentDirectory;
    if (argc < 2) {
        print("cat: missing operand\n", 0b0110);
    } else if(argc > 2){
        print("cat: too much operand\n", 0b0110);
    } else{
        int resultCD = cd(argv, argc, false);
        if (resultCD == 0){ //kalo berhasil cd nya
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
                    struct ClusterBuffer clusterBuff[12] = {0};
                    struct FAT32DriverRequest request = {
                        .buf = &clusterBuff,
                        .name = "\0\0\0\0\0\0\0",
                        .ext = "\0\0\0",
                        .parent_cluster_number = currentDirectory,
                        .buffer_size = 12 * CLUSTER_SIZE,
                    };
                    memcpy(request.name, output[0], 8);
                    memcpy(request.ext, output[1], 3);

                    int retcode;
                    retcode = read(request);
                    if (retcode != 0) {
                        switch (retcode) {
                            case 1:
                                print("Not a File\n", 0b0110);
                                break;
                            case 2:
                                print("No file\n", 0b0110);
                                break;
                            case -1:
                                print("Error\n", 0b0110);
                                break;
                        }
                    } else {
                        print((char *) &clusterBuff, 0xF);
                    }
                } else {
                    print("Error: not a file\n", 0xF);
                }
            }
        }
    }
    currentDirectory = temp_Path;
    return;
}