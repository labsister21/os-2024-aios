#ifndef _USER_CP
#define _USER_CP

void cp(char argv[4][100], int argc);
void append(char argv[4][100], char source[10]);
void mkfile(char argv[4][100], int argc, char ext[3]);
void cpHelper(char argvS[4][100], char argvD[4][100], uint32_t startDir);
#endif