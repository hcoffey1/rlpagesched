#include "CommandLine.h"

int find(char **begin, int size, const char *value) {
  int i;
  for (i = 0; i < size; i++) {
    if (strcmp(begin[i], value) == 0) {
      break;
    }
  }
  return i;
}

//====================
// From iain's answer:
// https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c

// Adapted to C by Hayden Coffey
char *getCmdOption(char **begin, int size, const char *option) {
  int i = find(begin, size, option);
  if (i != size && ++i != size) {
    return begin[i];
  }
  return 0;
}

int cmdOptionExists(char **begin, int size, const char *option) {
  return find(begin, size, option) != size;
}
//====================