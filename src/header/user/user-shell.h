#ifndef _USER_SHELL
#define _USER_SHELL

#include <stdint.h>
#include "../filesystem/fat32.h"

extern int currentDirectory ;
extern struct FAT32DirectoryTable dirTable;

void syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

void print(char* string, uint32_t color);

bool isAbsolutePath(char* path);

void printDir();

int findDirEntryClusterNumber(char* entryName);

void lineParser(char* line, char argv[4][100], int* argc);

#endif