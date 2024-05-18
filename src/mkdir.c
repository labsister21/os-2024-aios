#include "header/user/mkdir.h"
#include "header/user/user-shell.h"
#include "header/user/cd.h"
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"

int mkdir(char argv[4][100], int argc){
    if (argc > 2) {
        print("Error: too many arguments\n", 0xF);
        // -2 ga ngapa ngapain
        return -2;
    } else if (argc < 2) {
        print("Error: too few arguments\n", 0xF);
        return -2;
    } else {
        int startDir = currentDirectory;
        int retval = cd(argv, argc, false);
        if (retval != 0) {
            return -2;
        }
        char pathList[16][10];
        int pathCount;
        parsePath(argv[1], pathList, &pathCount);
        struct FAT32DriverRequest request;
        memcpy(request.name, pathList[pathCount-1], 8);
        request.buffer_size = 0;
        request.parent_cluster_number = currentDirectory;
        retval = write(request);
        currentDirectory = startDir;
        return retval;
    }

}