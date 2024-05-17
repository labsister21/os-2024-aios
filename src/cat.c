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
        int resultCD = cd((char (*)[100]) argv, argc, false);
        if (resultCD == 0){ //kalo berhasil cd nya
            char pathList[16][10];
            int pathCount;
            parsePath(argv[1], pathList, &pathCount);

            char output[16][10];
            int count;
            parseExt(pathList[pathCount-1],output,&count);

            //cari filenya
            int entry =  findFileEntryIndex(output[0]);
            if(entry == -1){ //tidak ketemu
                print("Error: no such File", 0b0110);
                print(argv[1], 0b0110);
            } else{
                    struct ClusterBuffer clusterBuff = {0};
                    struct FAT32DriverRequest request = {
                        .buf = &clusterBuff,
                        .name = "\0\0\0\0\0\0\0",
                        .ext = "\0\0\0",
                        .parent_cluster_number = currentDirectory,
                        .buffer_size = 4 * CLUSTER_SIZE,
                    };
                    memcpy(request.name, output[0], 8);
                    memcpy(request.ext, output[1], 3);

                    int32_t retcode;
                    syscall(0,(uint32_t) &request, (uint32_t) &retcode, 0x0);
                    if (retcode != 0) {
                        print(output[0],0XF);
                        print(output[1],0XF);
                        print("cat: ini?", 0b0110);
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
            }
        }
    }
    currentDirectory = temp_Path;
    return;
}