#ifndef SSHELL_C_
#define SSHELL_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512
#define ARG_MAX 16
#define TOK_LEN_MAX 32

/* This function inserts spaces surrounding the redirect characters */
/* in order to account for the edge case where there is no space    */
char* redir_space(char* cmd)
{
    char buf_cmd[CMDLINE_MAX];
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
            /* Otherwise it simply buffers the character into the new array */
        }
        else {
            buf_cmd[j] = cmd[i];
            j++;
        }
    }
    return strdup(buf_cmd);
}

/* This function splits the input cmd string into an array of strings */
/* by treating spaces as tokens                                       */
int funct_parse(char* cmd, char** arg_array)
{
    /* Inserts spaces surrounding redirection characters */
    char* buf_arr = redir_space(cmd);

    int arg_num = 0;

    char* cmd_arg = strtok(buf_arr, " ");

    /* Splits cmd string into an array of strings according */
    /* to the position of interleaven spaces                */
    while (cmd_arg != NULL)
    {
        arg_array[arg_num] = cmd_arg;

        arg_num++;
        cmd_arg = strtok(NULL, " ");
    }
    /* Returns number of strings in new array */
    return arg_num;
}

/* Custom system function to execute the shell command */
int our_system(char** arg_array)
{
    int status;
    pid_t pid;

    pid = fork();
    /* Child Path */
    if (pid == 0)
    {
        /* using execv for convenience, -p to access PATH variables */
        execvp(arg_array[0], arg_array);
        _exit(EXIT_FAILURE);
    }
    /* The fork failed. Report failure */
    else if (pid < 0)
    {
        status = -1;
    }
    else {
        /*This is the parent process. Wait for the child to complete */
        if (waitpid(pid, &status, 0) != pid)
        {
            status = -1;
        }
    }
    return status;
}

/* function to clear the array of strings buffer */
void buf_clear(int arg_num,  char** arg_array)
{
    for (int i = 0; i < arg_num; i++)
    {
        memset(arg_array[i], ' ', strlen(arg_array[i]));
    }
}

int main(void)
{
    char* arg_array[ARG_MAX];
    char cmd[CMDLINE_MAX];
    int arg_num = 0;

    while (1) {
    char* nl;
    int retval;

    /* Print prompt */
    printf("sshell$ ");
    fflush(stdout);

    /* clears memory of arg_array buffer */
    buf_clear(arg_num, arg_array);

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

    /* Parses arguments from string cmd into array of solitary */
    /* strings, each its own argument or token (<, >, |)       */
    arg_num = funct_parse(cmd, arg_array);

    /* Exit must be within main in order to break from program */
    /* instead of breaking from a fork                         */

    for (int i = 0; i < arg_num; i++)
    {
        printf("%s\n", arg_array[i]);
        fflush(stdout);
    }
    if (!strcmp(arg_array[0], "exit")) {
        fprintf(stderr, "Bye...\n");
        break;
    }
    /* Calls System to execute non-exit command, actual command*/
    /* identification takes place in our_system function       */
    retval = our_system(arg_array);

    /* Returns value from non-exit command */
    fprintf(stdout, "Return status value for '%s': %d\n",
        cmd, retval);
    }
    return EXIT_SUCCESS;
}
#endif //SSHELL_C_
