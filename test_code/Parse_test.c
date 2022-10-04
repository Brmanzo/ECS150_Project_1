#ifndef PARSE_TEST_C_
#define PARSE_TEST_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "funct_parse.h"

#define CMDLINE_MAX 512
#define ARG_MAX 16
#define TOK_LEN_MAX 32

  char* arg_array[ARG_MAX][TOK_LEN_MAX];
  char cmd[CMDLINE_MAX];
  
  int main()
  {
      printf("Enter a command : ");
      fgets(cmd, CMDLINE_MAX, stdin);
  
      funct_parse(cmd, *arg_array);
  
      for(int i = 0; i < ARG_MAX; i++)
      {
          printf("%s\n", *arg_array[i]);
      }
  }

#endif //PARSE_TEST_C_