#ifndef FUNCT_PARSE_H_
#define FUNCT_PARSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMDLINE_MAX 512
#define ARG_MAX 16
#define TOK_LEN_MAX 32

char arg_array[ARG_MAX][TOK_LEN_MAX];

char funct_parse(char[] cmd)
{
  char[] tokens = " ";
  int arg_num = 0;
  
  char* cmd_arg = strtok(cmd, tokens);
  
  redir = strchr(cmd_arg, '<');
  if(redir){
    *redir = '\0';
    arg_array[arg_num] = cmd_arg;
    arg_num++;
    arg_array[arg_num] = <;
    arg_num++;
  } else {
    arg_array[arg_num] = cmd_arg
    arg_num++;
  }
  
  while(cmd_arg != NULL)
  {
    arg_array[arg_num] = cmd_arg
    cmd_arg = strtok(NULL, tokens);
  }
     
}

#endif //FUNCT_PARSE_H_
