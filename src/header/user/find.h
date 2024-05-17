#ifndef _USER_FIND
#define _USER_FIND

#include <stdint.h>

void find(char* argv[], int argc);
void printPathFromList(char* pathList[], int lenPath);
void findHelper(char* item, uint32_t curDirCluster, bool visited[], char* pathList[], int lenPath, bool *found);
#endif