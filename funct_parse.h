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

// Takes in the command line and the redirect character of choice
char* redir_space(char* cmd)
{
  char buf_cmd[CMDLINE_MAX];
  int j = 0;
  
  // Looks for the redirect characters < and > inserting spaces surrounding them
  for(int i = 0; i < strlen(cmd); i++)
  {
    if(cmd[i] == '<')
    {
        buf_cmd[j] = ' ';
        j++;
        buf_cmd[j] = '<';
        j++;
        buf_cmd[j] = ' ';
        j++;
    }
    else if(cmd[i] == '>')
    {
        buf_cmd[j] = ' ';
        j++;
        buf_cmd[j] = '>';
        j++;
        buf_cmd[j] = ' ';
        j++;
    // Otherwise it simply buffers the character
    } else {
        buf_cmd[j] = cmd[i];
        j++;
    }

  }
}
  
char funct_parse(char* cmd)
{
  // Inserts spaces surrounding redirection characters
  char* buf_arr = redir_space(cmd);

  int arg_num = 0;
  
  char* cmd_arg = strtok(buf_arr, " ");
  
  strcpy(arg_array[arg_num], cmd_arg);
  arg_num++;

  while(cmd_arg != NULL)
  {
    strcpy(arg_array[arg_num], cmd_arg);

    cmd_arg = strtok(NULL, " ");
    arg_num++;
  }
     
}

#endif //FUNCT_PARSE_H_