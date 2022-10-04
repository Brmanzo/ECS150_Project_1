#ifndef PARSE_TEST_C_
#define PARSE_TEST_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMDLINE_MAX 512
#define ARG_MAX 16
#define TOK_LEN_MAX 32

// Takes in the command line and returns the new array with spaces
char* redir_space(char* cmd)
{
  char buf_cmd[CMDLINE_MAX];
  int j = 0;
  
  // Looks for the redirect characters < and > inserting spaces surrounding them
  for(unsigned int i = 0; i < strlen(cmd); i++)
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
    // Otherwise it simply buffers the character into the new array
    } else {
      buf_cmd[j] = cmd[i];
      j++;
    }
  }
  return strdup(buf_cmd);
}
  
int funct_parse(char* cmd, char** arg_array)
{
  // Inserts spaces surrounding redirection characters
  char* buf_arr = redir_space(cmd);

  int arg_num = 0;
  
  char* cmd_arg = strtok(buf_arr, " ");

  while(cmd_arg != NULL)
  {
    arg_array[arg_num] = cmd_arg;

    cmd_arg = strtok(NULL, " ");
    arg_num++;
  }
  return arg_num;
     
}

  
  char* arg_array[ARG_MAX];
  char cmd[CMDLINE_MAX];
  
  int main()
  {
      printf("Enter a command : ");
      fgets(cmd, CMDLINE_MAX, stdin);
  
      int arg_num = funct_parse(cmd, arg_array);
  
      for(int i = 0; i < arg_num; i++)
      {
          printf("%s\n", arg_array[i]);
      }
  }

#endif //PARSE_TEST_C_