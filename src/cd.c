#include <stdint.h>
#include "../header/user/user-shell.h"
void cd(char* argv[], int argc){
    if(argc > 2){
        put("cd: too many arguments\n", 0xF);
    }else{
        uint32_t search_directory_number = ROOT_CLUSTER_NUMBER;
        // kalo ga absolute ke current
        if(!isAbsolutePath(argv)){
            search_directory_number = currentDirectory;
        }
        dirTable.

    }
}