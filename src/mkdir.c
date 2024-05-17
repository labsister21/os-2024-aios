#include "header/user/mkdir.h"
#include "header/user/user-shell.h"
#include "header/user/cd.h"
#include "header/stdlib/string.h"
#include "header/filesystem/fat32.h"

void mkdir(char argv[4][100], int argc){
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
        request.buffer_size = 0;
        request.parent_cluster_number = currentDirectory;
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