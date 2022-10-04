#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CMDLINE_MAX 512
#define ARG_MAX 16
#define TOK_LEN_MAX 32


int main()
{
  char cmd[CMDLINE_MAX];
  char buf_cmd[CMDLINE_MAX];
  
  printf("Enter a command : ");
  fgets(cmd, 100, stdin);
  int j = 0;
  // Looks for the redirect character and inserts spaces surrounding it
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
    } else {
        buf_cmd[j] = cmd[i];
        j++;
    }

  }
  printf("%s", buf_cmd);
}