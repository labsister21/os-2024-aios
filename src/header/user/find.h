#ifndef _USER_FIND
#define _USER_FIND

#include <stdint.h>

void find(char argv[4][100], int argc);
void printPathFromList(char pathList[15][10], int lenPath);
void findHelper(char* item, uint32_t curDirCluster, bool visited[], char pathList[15][10], int lenPath, bool *found);
#endif