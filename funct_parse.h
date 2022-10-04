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
void redir_space(char* cmd[], const char redir)
{
  char* redir_pos = 0;
  size_t cmd_len = strlen(cmd);
  
  // Looks for the redirect character and inserts spaces surrounding it
  redir_pos =  strchr(cmd, redir)
  memmove(redir_pos +1 , redir_pos, cmd_len - redir_pos);
  *redir_pos = ' ';
  memmove(redir_pos +2 , redir_pos + 1, cmd_len - redir_pos - 1);
  *redir_pos + 2 = ' ';
}
  
char funct_parse(char[] cmd)
{
  // Inserts spaces surrounding redirection characters
  redir_space(cmd, '<');
  redir_space(cmd, '>');

  char[] tokens = " ";
  int arg_num = 0;
  
  char* cmd_arg = strtok(cmd, tokens);
  
  strcpy(arg_array[arg_num], cmd_arg);
  arg_num++;

  while(cmd_arg != NULL)
  {
    strcpy(arg_array[arg_num], cmd_arg);

    cmd_arg = strtok(NULL, tokens);
    arg_num++;
  }
     
}

#endif //FUNCT_PARSE_H_
