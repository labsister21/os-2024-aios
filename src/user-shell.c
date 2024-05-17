#include <stdint.h>
#include "header/filesystem/fat32.h"
#include "header/stdlib/string.h"
#include "header/user/user-shell.h"

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    __asm__ volatile("mov %0, %%ebx" : /* <Empty> */ : "r"(ebx));
    __asm__ volatile("mov %0, %%ecx" : /* <Empty> */ : "r"(ecx));
    __asm__ volatile("mov %0, %%edx" : /* <Empty> */ : "r"(edx));
    __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(eax));
    // Note : gcc usually use %eax as intermediate register,
    //        so it need to be the last one to mov
    __asm__ volatile("int $0x30");
}

void print(char* string, uint32_t color) {
    syscall(6, (uint32_t) string, strlen(string), color);
}

bool isAbsolutePath(char* path) {
    if (path[0] == '/') {
        return true;
    } else {
        return false;
    }
}

void updateDirectoryTable(int dirCluster) {
    syscall(8, (uint32_t) (&dirTable), dirCluster, 0);
}

void printDir() {
    print("/", 0xB);
    char tempDir[32][10];
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 10; j++) {
            tempDir[i][j] = 0;
        }
    }

    uint32_t dir = currentDirectory;
    // traverse directory
    int i = 0;
    while (dir != ROOT_CLUSTER_NUMBER) {
        updateDirectoryTable(dir);
        memcpy(tempDir[i], dirTable.table->name, 8);
        dir = (uint32_t) ((dirTable.table->cluster_high << 16) | dirTable.table->cluster_low);
        i++;
    }

    for (int j = i; j >= 0; j--) {
        print(tempDir[j], 0xB);
        if (j >= 1) {
            print("/", 0xB);
        }
    }

    print("$ ", 0xF);
}

void lineParser(char* line, char argv[4][100], int* argc){
    *argc = 0;
    int index = 0;
    int wordIndex = 0;
    while (line[index] != '\n') {
        while (line[index] == ' ') {
            index++;
        }
        if (line[index] != '\n') {
            (*argc)++;
        }
        wordIndex = 0;
        while (line[index] != ' ' && line[index] != '\n') {
            argv[(*argc)-1][wordIndex++] = line[index];
            index++;
        }
        if (*argc == 4) {
            break;
        }
    }
}

// cari cluster number dari directory entry
int findDirEntryIndex(char* entryName) {
    for (int i = 0; i < 64; i++) {
        if (memcmp(dirTable.table[i].name, entryName, 8) == 0 && 
            dirTable.table[i].user_attribute == UATTR_NOT_EMPTY) {
            return i;
        }
    }
    return -1;
}

void parsePath(char* path, char output[16][10], int* wordCount) {
    int i = 0;
    int len = strlen(path);
    *wordCount = 1;
    if (path[0] == '/') {
        i++;
    }
    int wordIndex = 0;
    for (; i < len; i++) {
        if (path[i] == '/') {
            (*wordCount)++;
            wordIndex = 0;
        } else {
            output[(*wordCount)-1][wordIndex] = path[i];
            wordIndex++;
        }
    }
}

int currentDirectory = ROOT_CLUSTER_NUMBER;
struct FAT32DirectoryTable dirTable;

int main(void) {
    struct ClusterBuffer      cl[2]   = {0};
    struct FAT32DriverRequest request = {
        .buf                   = &cl,
        .name                  = "shell",
        .ext                   = "\0\0\0",
        .parent_cluster_number = ROOT_CLUSTER_NUMBER,
        .buffer_size           = 2*CLUSTER_SIZE,
    };
    int32_t retcode;
    syscall(0, (uint32_t) &request, (uint32_t) &retcode, 0);
    if (retcode == 0)
        // syscall(6, (uint32_t) "owo\n", 4, 0xF);
        print("owo\n", 0xF);

    updateDirectoryTable(currentDirectory);
    char line[128];
    for (int i = 0; i < 128; i++) {
        line[i] = 0;
    }
    int linelen;
    char buf;
    syscall(7, 0, 0, 0);
    while (true) {
        // Print directory
        print("User@aiOS:", 0x5A);
        printDir();

        // Reset line to empty
        for (int i = 0; i < 128; i++) {
            line[i] = 0;
        }
        buf = 0;
        linelen = 0;

        // Get line until '\n'
        bool isEndLine = false;
        while (!isEndLine) {
            syscall(4, (uint32_t) &buf, 0, 0);
            if (buf == '\n') {
                isEndLine = true;
            }
            if (buf == '\b') {
                if (linelen == 0) {
                    continue;
                }
                line[--linelen] = 0;
            } else if (buf != '\0') {
                line[linelen++] = buf;
            }
            syscall(5, (uint32_t) &buf, 0xF, 0);
        }

        // Parse line
        char argv[4][100];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 100; j++) {
                argv[i][j] = 0;
            }
        }
        int argc;
        lineParser(line, argv, &argc);

        // Call functions
        if (memcmp(argv[0], "cd", strlen(argv[0])) == 0) {
            // cd(argv, argc);
            int wordCount;
            char output[16][10];
            parsePath(argv[1], output, &wordCount);
            print(argv[0], 0xF);
            for (int i = 0; i < wordCount; i++) {
                print(output[i], 0xF);
            }
        } else if (memcmp(argv[0], "ls", strlen(argv[0])) == 0) {
            // ls(argv, argc);
            print(argv[0], 0xF);
        } else if (memcmp(argv[0], "mkdir", strlen(argv[0])) == 0) {
            // mkdir(argv, argc);
            print(argv[0], 0xF);
        } else if (memcmp(argv[0], "cat", strlen(argv[0])) == 0) {
            // cat(argv, argc);
            print(argv[0], 0xF);
        } else if (memcmp(argv[0], "cp", strlen(argv[0])) == 0) {
            // cp(argv, argc);
            print(argv[0], 0xF);
        } else if (memcmp(argv[0], "rm", strlen(argv[0])) == 0) {
            // rm(argv, argc);
            print(argv[0], 0xF);
        } else if (memcmp(argv[0], "mv", strlen(argv[0])) == 0) {
            // mv(argv, argc);
            print(argv[0], 0xF);
        } else if (memcmp(argv[0], "find", strlen(argv[0])) == 0) {
            // find(argv, argc);
            print(argv[0], 0xF);
        } else if (argc == 0) {
            // Do nothing
        } else {
            print("command not found: ", 0xF);
            print(argv[0], 0xF);
        }
        print("\n", 0xF);
    }

    return 0;
}



// int main(void) {
//     __asm__ volatile("mov %0, %%eax" : /* <Empty> */ : "r"(0xDEADBEEF));
//     return 0;
// }

