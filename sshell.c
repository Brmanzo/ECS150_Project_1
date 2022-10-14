#ifndef SSHELL_C_
#define SSHELL_C_

#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Cmdline macros according to assignment specifications. */
#define CMDLINE_MAX 512
#define USR_ARG_MAX 16
#define TOK_LEN_MAX 32

/* Enums to pass when handling errors. */
enum {
  TOO_MANY_ARGS,
  CANT_CD_DIR,
  NO_SUCH_DIR,
  DIR_STACK_EMPTY,
  CMD_NOT_FOUND,
  MISSING_CMD,
  NO_OUTPUT_F,
  CANT_OPEN_OUT_F,
  CANT_OPEN_IN_F,
  MISLOC_OUT_DIR,
  NO_INPUT_F,
  MISLOC_IN_DIR
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
  case MISSING_CMD:
    fprintf(stderr, "Error: missing command\n");
    break;
  case NO_OUTPUT_F:
    fprintf(stderr, "Error: no output file\n");
    break;
  case CANT_OPEN_OUT_F:
    fprintf(stderr, "Error: cannot open output file\n");
    break;
  case CANT_OPEN_IN_F:
    fprintf(stderr, "Error: cannot open input file\n");
    break;
  case MISLOC_OUT_DIR:
    fprintf(stderr, "Error: mislocated output redirection\n");
    break;
  case NO_INPUT_F:
    fprintf(stderr, "Error: no input file\n");
    break;
  }
}
/* function to clear the array of strings buffer */
void buf_clear(int arg_num, char** arg_array)
{
  /* Sets all occupied elements to NULL. */
  for (int i = 0; i < arg_num; i++)
    memset(arg_array[i], 0, strlen(arg_array[i]));
  /* Ensures final element is NULL. */
  arg_array[arg_num - 1] = NULL;
}
/* This function inserts spaces surrounding the redirect and pipe   */
/* characters  to account for the edge case where there's no space. */
char* insert_spaces(char* cmd, int* pipe_num,
  int* in_redir_num, int* out_redir_num)
{
  /* Buffer to hold resulting command line with spaces inserted. */
  char cmd_line_buf[CMDLINE_MAX + 1];
  memset(cmd_line_buf, 0, sizeof(cmd_line_buf));
  int j = 0;

  /* Looks for the characters <, >, and | and inserts spaces surrounding them*/
  for (unsigned int i = 0; i < strlen(cmd); i++)
  {
    if (cmd[i] == '<')
    {
      if (*pipe_num != 0)
        error_handler(MISLOC_IN_DIR);
      (*in_redir_num)++;
      cmd_line_buf[j] = ' ';
      j++;
      cmd_line_buf[j] = '<';
      j++;
      cmd_line_buf[j] = ' ';
      j++;
    }
    else if (cmd[i] == '>')
    {
      (*out_redir_num)++;
      cmd_line_buf[j] = ' ';
      j++;
      cmd_line_buf[j] = '>';
      j++;
      cmd_line_buf[j] = ' ';
      j++;
    }
    else if (cmd[i] == '|')
    {
      if (*in_redir_num != 0)
        error_handler(MISLOC_OUT_DIR);
      if (*out_redir_num != 0)
        error_handler(MISLOC_IN_DIR);
      (*pipe_num)++;
      cmd_line_buf[j] = ' ';
      j++;
      cmd_line_buf[j] = '|';
      j++;
      cmd_line_buf[j] = ' ';
      j++;
      /* Otherwise it simply buffers the character into the new array */
    }
    else {
      cmd_line_buf[j] = cmd[i];
      j++;
    }
  }
  return strdup(cmd_line_buf);
}
/* This function splits the input cmd string into an array of strings */
/* by treating spaces as tokens. Returns total amount of argyments.   */
int funct_parse(char* cmd, char** arg_array, int* pipe_num,
  int* in_redir_num, int* out_redir_num)
{
  /* Inserts spaces surrounding redirection characters */
  char* buf_arr = insert_spaces(cmd, pipe_num, in_redir_num, out_redir_num);
  int arg_num = 0;

  /* string of arguments separated by spaces. */
  char* spaced_cmd_args = strtok(buf_arr, " \n");

  /* Splits cmd string into an array of strings on every space. */
  /* The redundant space inserts by space_insert are ignored.   */
  while (spaced_cmd_args != NULL)
  {
    arg_array[arg_num] = spaced_cmd_args;
    arg_num++;

    spaced_cmd_args = strtok(NULL, " ");
  }
  if (arg_num > USR_ARG_MAX)
  {
    error_handler(TOO_MANY_ARGS);
    return(-1);
  }
  arg_array[arg_num] = NULL;

  if ((!strcmp(arg_array[0], ">")) || (!strcmp(arg_array[0], "|"))
    || (!strcmp(arg_array[arg_num - 1], "|"))) {
    error_handler(MISSING_CMD);
    return(-1);
  }

  if ((!strcmp(arg_array[arg_num - 1], ">")))
  {
    error_handler(NO_OUTPUT_F);
    return(-1);
  }
  if ((!strcmp(arg_array[arg_num - 1], "<")))
  {
    error_handler(NO_INPUT_F);
    return(-1);
  }
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
    }
    else {
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
      /* Creates new room for cwd independent from the buffer. */
      dir_stack[i] = malloc(sizeof(char) * CMDLINE_MAX);
      getcwd(stack_buf, CMDLINE_MAX);
      strcpy(dir_stack[i], stack_buf);
      (*dir_num)++;
      return(0);
    }
    else {
      error_handler(NO_SUCH_DIR);
      return(1);
    }
  }
  /* If the user types in popd, the most recent directory is popped from the */
  /* directory stack aka decrementing the array index to ignore the top.     */
  else if (!strcmp(arg_array[0], "popd"))
  {
    if (*dir_num > 1)
    {
      /* Places index on new top of stack. */
      (*dir_num) -= 2;
      int i = *dir_num;
      /* Changes directory to new top. */
      int DirEntry = chdir(dir_stack[i]);
      /* Places increment at new available top.*/
      (*dir_num)++;
      return(DirEntry);
    }
    else {
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
    }
    else {
      error_handler(DIR_STACK_EMPTY);
      return(1);
    }
  }
}
int Pipe(char** arg_array, int arg_num) {
  /* File descriptor array to pass to pipe. */
  int fd[2];
  int i = 0;
  int pipe_pos = 0;
  int status = 0;

  /* Instantating separate command string arrays to execvp(). */
  char* command_1[USR_ARG_MAX];
  char* command_2[USR_ARG_MAX];

  /* Up until the pipe, put the command and arguments in command_1[]. */
  while (strcmp(arg_array[i], "|"))
  {
    command_1[i] = arg_array[i];
    i++;
    pipe_pos++;
  }
  command_1[i] = NULL;

  /* From the pipe to the end of arg_array, put */
  /* the command and arguments into command_2[]. */
  for (int j = pipe_pos + 1; j < arg_num; j++)
  {
    command_2[j - pipe_pos - 1] = arg_array[j];
  }
  command_2[arg_num - pipe_pos] = NULL;

  status = pipe(fd);
  /* Pipe Parent Process */
  if (fork() != 0) {
    close(fd[0]);
    /* connect output of command_1 to input of pipe. */
    dup2(fd[1], STDOUT_FILENO);
    /* Close redundant output. */
    close(fd[1]);

    /* Execute command_1 as the parent */
    execvp(command_1[0], command_1);
  }
  /* Pipe Child Process */
  else {
    /* No need for write access */
    close(fd[1]);
    /* Connect output of pipe to input of command_2 */
    dup2(fd[0], STDIN_FILENO);
    /* Close redundant input */
    close(fd[0]);

    /* Execute command_2 as the child */
    execvp(command_2[0], command_2);
  }
  /* Source: Professor's Syscall Lecture Code */
  return(status);
}
int Redirect(int in_redir_num, int out_redir_num, char** arg_array,
  int arg_num) 
{
  int fd = 0;
  int i = 0;
  char* redir_char;
  int channel;

  /* Sets channel to stdout and character to the output redirect. */
  if ((in_redir_num == 0) && (out_redir_num > 0))
  {
    fd = open(arg_array[arg_num - 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    redir_char = ">";
    /* STDOUT_FILE_NO */
    channel = 1;
  }
  /* Otherwise sets channel to stdin and character to the input redirect. */
  else
  {
    fd = open(arg_array[arg_num - 1], O_RDONLY);
    redir_char = "<";
    /* STDIN_FILE_NO */
    channel = 0;
  }
  /* Array to store command arguments before the redirect character. */
  char* parameter_array[USR_ARG_MAX];

  while (strcmp(arg_array[i], redir_char))
  {
    parameter_array[i] = arg_array[i];
    i++;
  }
  /* Ensures final string in array is NULL. */
  parameter_array[i] = NULL;

  /* If file opened successfully, is duped over input/output and executed. */
  if (fd >= 0)
  {
    dup2(fd, channel);
    close(fd);
    execvp(arg_array[0], parameter_array);
    _exit(1);
  }
  /* Source: Professor's Syscall Lecture Code. */
  /* Otherwise throws appropriate file opening error. */
  else
  {
    if (channel)
    {
      error_handler(CANT_OPEN_OUT_F);
      return(-1);
    }
    else
    {
      error_handler(CANT_OPEN_IN_F);
      return(-1);
    }
  }
}
/* Custom system function to execute the shell     */
/* commands while enabling piping and redirection. */
int our_system(char** arg_array, int* pipe_num, int* in_redir_num,
  int* out_redir_num, int arg_num)
{
  int status = 0;
  pid_t pid;

  pid = fork();
  /* Child Path */
  if (pid == 0)
  {
    if (((*in_redir_num == 1) || (*out_redir_num == 1)) && (*pipe_num == 0))
    {
      status = Redirect(*in_redir_num, *out_redir_num, arg_array, arg_num);
      _exit(1);
    }
    else if ((*in_redir_num == 0) && (*out_redir_num == 0) && (*pipe_num > 0))
    {
      status = Pipe(arg_array, arg_num);
    }
    else if ((*in_redir_num == 1) && (*pipe_num > 0)) {
      error_handler(MISLOC_IN_DIR);
    }
    else {
      /* using execv for array of arguments, -p to access PATH variables. */
      execvp(arg_array[0], arg_array);
      _exit(1);
    }
  }
  /* The fork failed. Report failure. */
  else if (pid < 0)
  {
    status = -1;
    /*This is the parent process. Wait for the child to complete. */
  }
  else {
    if (waitpid(pid, &status, 0) != pid)
      status = -1;
  }
  return status;
}
int main(void)
{
  /* Directory Stack buffers and arrays.  */
  char pwd_buf[CMDLINE_MAX];
  char stack_buf[CMDLINE_MAX];
  char* dir_stack[USR_ARG_MAX];
  int dir_num = 0;
  dir_stack[dir_num] = malloc(sizeof(char) * CMDLINE_MAX);

  /* Initializes Directory Stack with CWD. */
  getcwd(stack_buf, CMDLINE_MAX);
  strcpy(dir_stack[0], stack_buf);
  dir_num++;

  /* Function parsing structures and indices. */
  char* arg_array[USR_ARG_MAX];
  char cmd[CMDLINE_MAX];
  int arg_num = 0;

  /* Character Counters. */
  int pipe_num = 0;

  int in_redir_num = 0;
  int out_redir_num = 0;

  while (1)
  {
    char* nl;
    int retval = 0;

    in_redir_num = 0;
    out_redir_num = 0;

    /* Print prompt */
    printf("sshell@ucd$ ");
    fflush(stdout);

    /* Get command line */
    fgets(cmd, CMDLINE_MAX, stdin);

    if (strcmp(cmd, "\n"))
    {
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
      arg_num = funct_parse(cmd, arg_array, &pipe_num,
        &in_redir_num, &out_redir_num);

      if (arg_num > 0) {
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
        }
        else {
          /* Calls System to execute non-built-in commands, actual     */
          /* command identification takes place in our_system function */

          retval = our_system(arg_array, &pipe_num, &in_redir_num,
            &out_redir_num, arg_num);

          /* Returns value from non-exit command */
          if ((retval != 0))
          {
            //if ((retval != 0))
            //  continue;
            error_handler(CMD_NOT_FOUND);
            fprintf(stderr, "+ completed '%s' [1]\n", cmd);
          }
          else {
            fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
          }
        }
      }
    }
    else
      error_handler(MISSING_CMD);
  }
  return EXIT_SUCCESS;
}
#endif //SSHELL_C_
