#include "header/user/exec.h"
#include "header/user/user-shell.h"
#include "header/user/cd.h"
#include "header/stdlib/string.h"

void exec(char argv[4][100], int argc){
    int temp_Path = currentDirectory;
    if (argc < 2) {
        print("exec: missing argument\n", 0xF);
        return;
    } else if(argc > 2){
        print("exec: too much arguments\n", 0xF);
        return;
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
                print("Error: no such File: ", 0xF);
                print(argv[1], 0xF);
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

                    int32_t retcode;
                    syscall(11,(uint32_t) &request, (uint32_t) &retcode, 0x0);
                    if (retcode != 0) {
                        switch (retcode) {
                            case 1:
                                print("exec: max process exceeded\n", 0xF);
                                break;
                            case 2:
                                print("exec: invalid entrypoint\n", 0xF);
                                break;
                            case 3:
                                print("exec: not enough memory\n", 0xF);
                                break;
                            case 4:
                                print("exec: fs read failure\n", 0xF);
                                break;
                            case 5:
                                print("exec: paging alloc failure\n", 0xF);
                                break;
                        }
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