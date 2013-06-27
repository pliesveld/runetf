#include <string.h>
#include <assert.h>
#include "util.h"

char *sm_strdup(const char *str)
{
  char *ptr = new char[strlen(str)+1];
  strcpy(ptr, str);
  return ptr;
}

