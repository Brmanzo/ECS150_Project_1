#ifndef SSHELL_C_
#define SSHELL_C_

#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMDLINE_MAX 512
#define USR_ARG_MAX 16
#define TOK_LEN_MAX 32

typedef struct org_cmd {
  // Separate Name and Args for execv
  char processName[TOK_LEN_MAX + 1];
  char* processArgs[USR_ARG_MAX];
  char filename[TOK_LEN_MAX + 1];

  // To allow / deny usage of fdIn and fdOut variables
  // Possible that we only need one set, to remove if needed later
  bool pipeIn;
  bool pipeOut;
  bool redirIn;
  bool redirOut;

  // To be initialized after
  int fdIn;
  int fdOut;
}org_cmd;

// Declare array of struct outside of this scope and pass by reference
org_cmd* organizeProcesses(org_cmd usrProcessArr[],
  char* inputCmds[], int numCmds)
{ // numCmds includes | and <> symbols
  int p = 0; // iterator for process in usrProcessArr
  bool firstArg = true; // toggle within for loop
  int argCount = 0; // iterator for struct argument array parameter

  //initialize struct
  for (int j = 0; j < 5; j++)
  {
    usrProcessArr[j].pipeIn = false;
    usrProcessArr[j].pipeOut = false;
    usrProcessArr[j].redirIn = false;
    usrProcessArr[j].redirOut = false;
  }
  for (int i = 0; i < numCmds; i++) {
    if (!strncmp(inputCmds[i], "|", 1))
    {
      usrProcessArr[p].pipeOut = true;
      usrProcessArr[++p].pipeIn = true;
      firstArg = true;

      printf("Pipe\n");
      fflush(stdout);
      continue;
    }
    if (!strncmp(inputCmds[i], "<", 1)) { //input redir, program < file
      usrProcessArr[p].redirIn = true;
      strcpy(usrProcessArr[p].filename, inputCmds[i + 1]);
      i++;
      firstArg = false;
      argCount = 0;

      printf("Input: %s\n", usrProcessArr[p].filename);
      fflush(stdout);
      continue;
    }
    if (!strncmp(inputCmds[i], ">", 1)) {
      usrProcessArr[p].redirOut = true;
      strcpy(usrProcessArr[p].filename, inputCmds[i + 1]);
      i++;
      firstArg = false;

      printf("Output: %s\n", usrProcessArr[p].filename);
      fflush(stdout);
      continue;
    }
    if (firstArg)
    {
      strcpy(usrProcessArr[p].processName, inputCmds[i]);
      firstArg = false;
      argCount = 0;

      printf("Name: %s\n", usrProcessArr[p].processName);
      fflush(stdout);
    } else {
      usrProcessArr[p].processArgs[argCount] = malloc(sizeof(char)
        * strlen(inputCmds[i]));
      strcpy(usrProcessArr[p].processArgs[argCount], inputCmds[i]);
      argCount++;

      printf("Arg %d: %s\n", argCount - 1, 
        usrProcessArr[p].processArgs[argCount - 1]);
      fflush(stdout);
      }
  }
  return usrProcessArr;
}
/* Enums to pass when handling errors. */
enum {
  TOO_MANY_ARGS,
  CANT_CD_DIR,
  NO_SUCH_DIR,
  DIR_STACK_EMPTY,
  CMD_NOT_FOUND
};
/* Function to handle errors, recieves an enum to determine error code. */
void error_handler(int error_code)
{
  switch (error_code)
  {
  case TOO_MANY_ARGS:
    fprintf(stderr, "Error: too many process arguments\n");
    break;
  case CANT_CD_DIR:
    fprintf(stderr, "Error: cannot cd into directory\n");
    break;
  case NO_SUCH_DIR:
    fprintf(stderr, "Error: no such directory\n");
    break;
  case DIR_STACK_EMPTY:
    fprintf(stderr, "Error: directory stack empty\n");
    break;
  case CMD_NOT_FOUND:
    fprintf(stderr, "Error: command not found\n");
    break;
  }
}
/* function to clear the array of strings buffer */
void buf_clear(int arg_num, char** arg_array)
{
  for (int i = 0; i < arg_num - 1; i++)
    memset(arg_array[i], 0, strlen(arg_array[i]));
}
/* This function inserts spaces surrounding the redirect characters */
/* in order to account for the edge case where there is no space    */
char* redir_space(char* cmd, int* pipe_num)
{
  char buf_cmd[CMDLINE_MAX + 1];
  memset(buf_cmd, 0, sizeof(buf_cmd));
  int j = 0;

  /* Looks for the characters < and > and inserts spaces surrounding them */
  for (unsigned int i = 0; i < strlen(cmd); i++)
  {
    if (cmd[i] == '<')
    {
      buf_cmd[j] = ' ';
      j++;
      buf_cmd[j] = '<';
      j++;
      buf_cmd[j] = ' ';
      j++;
    }
    else if (cmd[i] == '>')
    {
      buf_cmd[j] = ' ';
      j++;
      buf_cmd[j] = '>';
      j++;
      buf_cmd[j] = ' ';
      j++;
    }
    /* If the character is a pipe, it increments pipe_num for our piping. */
    else if (cmd[i] == '|')
    {
      (*pipe_num)++;
      buf_cmd[j] = cmd[i];
      j++;
    /* Otherwise it simply buffers the character into the new array */
    } else {
      buf_cmd[j] = cmd[i];
      j++;
    }
  }
  return strdup(buf_cmd);
}
/* This function splits the input cmd string into an */
/* array of strings by treating spaces as tokens.    */
int funct_parse(char* cmd, char** arg_array, int* pipe_num)
{
  /* Inserts spaces surrounding redirection characters */
  char* buf_arr = redir_space(cmd, pipe_num);
  int arg_num = 0;

  char* cmd_arg = strtok(buf_arr, " \n");

  /* Splits cmd string into an array of strings on every space. */
  /* The redundant space inserts by redir_space are ignored.    */
  while (cmd_arg != NULL)
  {
    arg_array[arg_num] = cmd_arg;
    arg_num++;

    cmd_arg = strtok(NULL, " ");
  }
  if (arg_num > USR_ARG_MAX)
    error_handler(TOO_MANY_ARGS);
  arg_array[arg_num] = NULL;
  /* Returns number of strings in new array */
  return arg_num;
}
int built_in_funct(char** arg_array, char** dir_stack, char* pwd_buf,
  char* stack_buf, int* dir_num)
{
  /* If user types in pwd, the current filepath is printed to the terminal. */
  if (!strcmp(arg_array[0], "pwd"))
  {
    /* Retrieves current working directory as a string literal*/
    memset(pwd_buf, 0, CMDLINE_MAX);
    char* cwd = getcwd(pwd_buf, CMDLINE_MAX);

    printf("%s\n", cwd);
    return(0);
  }
  /* If user types in cd the current working directory is changed to */
  /* the directory specified in the second argument (arg_array[1]).  */
  else if (!strcmp(arg_array[0], "cd"))
  {
    int DirEntry = chdir(arg_array[1]);
    if (DirEntry == 0) {
      return(0);
    } else {
      error_handler(CANT_CD_DIR);
      return(1);
    }
  }
  /* If user types in pushd, the current working directory is changed to  */
  /* the directory specified in the second argument and the new directory */
  /* is added to the top of the directory stack.                          */
  else if (!strcmp(arg_array[0], "pushd"))
  {
    int DirEntry = chdir(arg_array[1]);

    if (DirEntry == 0)
    {
      int i = *dir_num;
      dir_stack[i] = malloc(sizeof(char) * CMDLINE_MAX);
      getcwd(stack_buf, CMDLINE_MAX);
      strcpy(dir_stack[i], stack_buf);
      (*dir_num)++;
      return(0);
    } else {
      error_handler(NO_SUCH_DIR);
      return(1);
    }
  }
  /* If the user types in popd, the most recent directory is popped from the */
  /* directory stack aka decrementing the array index to ignore the top.     */
  else if (!strcmp(arg_array[0], "popd"))
  {
    if (*dir_num > 0)
    {
      /* Places index on new top of stack. */
      (*dir_num) -= 2;
      int i = *dir_num;
      /* Changes directory to new top. */
      int DirEntry = chdir(dir_stack[i]);
      /* Places increment at new available top.*/
      (*dir_num)++;
      return(DirEntry);
    } else {
      error_handler(DIR_STACK_EMPTY);
      return(1);
    }
  }
  /* Otherwise, the fifth condition will be that the user typed in dirs and  */
  /* prints the current state of the directory stack array in reverse order. */
  else
  {
    int dir_num_cpy = *dir_num - 1;
    if (dir_num_cpy >= 0)
    {
      int j = dir_num_cpy;

      for (int i = j; i > -1; i--)
        fprintf(stdout, "%s\n", dir_stack[i]);
      return(0);
    } else {
      error_handler(DIR_STACK_EMPTY);
      return(1);
    }
  }
}
/* Custom system function to execute the shell command. */
int our_system(char** arg_array)
{
  int status = 0;
  pid_t pid;

  pid = fork();
  /* Child Path */
  if (pid == 0)
  {
    /* using execv for array of arguments, -p to access PATH variables. */
    execvp(arg_array[0], arg_array);
    _exit(1);
  }
  /* The fork failed. Report failure. */
  else if (pid < 0)
  {
    status = -1;
  } else {
    /*This is the parent process. Wait for the child to complete. */
    if (waitpid(pid, &status, 0) != pid)
      status = -1;
  }
  return status;
}
int main(void)
{
  char* arg_array[USR_ARG_MAX];

  /* string to hold the working directory (stack) */
  char pwd_buf[CMDLINE_MAX];
  char stack_buf[CMDLINE_MAX];
  char* dir_stack[USR_ARG_MAX];
  
  int pipe_num = 0;
  int dir_num = 0;
  dir_stack[dir_num] = malloc(sizeof(char) * CMDLINE_MAX);
 
  getcwd(stack_buf, CMDLINE_MAX);
  strcpy(dir_stack[0], stack_buf);
  dir_num++;

  char cmd[CMDLINE_MAX];
  int arg_num = 0;

  while (1)
  {
    char* nl;
    int retval = 0;

    /* Print prompt */
    printf("sshell$ ");
    fflush(stdout);

    /* Get command line */
    fgets(cmd, CMDLINE_MAX, stdin);

    /* Print command line if stdin is not provided by terminal */
    if (!isatty(STDIN_FILENO))
    {
      printf("%s", cmd);
      fflush(stdout);
    }
    /* Remove trailing newline from command line */
    nl = strchr(cmd, '\n');
    if (nl)
      *nl = '\0';

    /* clears memory of arg_array buffer */
    buf_clear(arg_num, arg_array);

    /* Parses arguments from string cmd into array of solitary */
    /* strings, each its own argument or token (<, >, |)       */
    arg_num = funct_parse(cmd, arg_array, &pipe_num);

    /* Exit must be within main in order to break from program */
    /* instead of breaking from a fork                         */
    if (!strcmp(arg_array[0], "exit"))
    {
      fprintf(stderr, "Bye...\n");
      fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
      break;
    }
    /* If the command string is any of the built-in commands below */
    /* the built-in function will handle them.                     */
    if ((!strcmp(arg_array[0], "pwd")) || (!strcmp(arg_array[0], "cd")) 
      || (!strcmp(arg_array[0], "pushd")) || (!strcmp(arg_array[0], "popd")) 
        || (!strcmp(arg_array[0], "dirs")))
    {
      retval = built_in_funct(arg_array, dir_stack, pwd_buf,
          stack_buf, &dir_num);
      fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
    } else {
      /* Calls System to execute non-exit command, actual command*/
      /* identification takes place in our_system function       */
      retval = our_system(arg_array);

      /* Returns value from non-exit command */
      if (retval != 0)
      {
        error_handler(CMD_NOT_FOUND);
        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
      } else {
        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
      }
    }
  }
  return EXIT_SUCCESS;
}
#endif //SSHELL_C_
