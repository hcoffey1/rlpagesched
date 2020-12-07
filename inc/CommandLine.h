#ifndef __COMMANDLINE__
#define __COMANDLINE__
#include <string.h>

char* getCmdOption(char ** begin, int end, const char * option);
int cmdOptionExists(char** begin, int end, const char * option);

#endif