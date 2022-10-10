#ifndef SSHELL_C_
#define SSHELL_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define CMDLINE_MAX 512
#define ARG_MAX 16
#define TOK_LEN_MAX 32

enum {
    TOO_MANY_ARGS
};
/* File Selector to be used within Directory Selector function used in ls*/
/* implementation. Passed as a parameter to scandir().                   */
static int file_sel(const struct dirent* DirEntry)
{
    /* Filters out directory entries that represent the directory */
    /* hierarchy. Returning 1 lists the file, 0 ignores the file. */
    if (!strcmp(DirEntry->d_name, ".") || !strcmp(DirEntry->d_name, ".."))
    {
        return 0;
    }
    else {
        return 1;
    }
}

/* Directory Selector scans the contents of the provided file name */
/* and prints the contents to the terminal in alphabetical order.  */
void dir_sel(char* FileName, char** arg_array) {
    /* Initializing Directory type object and number of entries in directory. */
    struct dirent** DirEntry;
    int NumDirEntry = 0;
    NumDirEntry = scandir(FileName, &DirEntry, file_sel, alphasort);

    /* Retrieves DirEntry Array via reference and iterates through it */
    /* Printing selected files to the terminal in alphabetical order. */
    if (NumDirEntry >= 0)
    {
        for (int i = 0; i < NumDirEntry; i++)
        {
            fprintf(stdout, "%s ", DirEntry[i]->d_name);
        }
    } else {
        fprintf(stderr, "ls: cannot access '%s': No such file or directory", arg_array[1]);
    }
    fprintf(stdout, "\n");
}

/* Function to handle errors, recieves an enum to determine error code. */
void error_handler(int error_code)
{
    switch (error_code)
    {
    case TOO_MANY_ARGS:
        fprintf(stderr, "Error: too many process arguments\n");
        break;
    }
}

/* This function inserts spaces surrounding the redirect characters */
/* in order to account for the edge case where there is no space    */
char* redir_space(char* cmd)
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

    char* cmd_arg = strtok(buf_arr, " \n");

    /* Splits cmd string into an array of strings according */
    /* to the position of interleaven spaces                */
    while (cmd_arg != NULL)
    {
        arg_array[arg_num] = cmd_arg;

        arg_num++;
        cmd_arg = strtok(NULL, " ");
    }
    if (arg_num > ARG_MAX)
    {
        error_handler(0);
    }
    arg_array[arg_num] = NULL;
    /* Returns number of strings in new array */
    return arg_num;
}

/* Custom system function to execute the shell command */
int our_system(char** arg_array, int arg_num)
{
    int status = 0;
    pid_t pid;

    pid = fork();
    /* Child Path */
    if (pid == 0)
    {
        /* If user types in pwd, the current filepath is printed to the terminal. */
        if (!strcmp(arg_array[0], "pwd"))
        {
            /* Retrieves current working directory as a string literal*/
            char buffer[CMDLINE_MAX - 3];
            char* cwd = getcwd(buffer, CMDLINE_MAX - 3);

            fprintf(stdout, "%s\n", cwd);

            _exit(status);
        }
        /* If user types in ls, the contents of the filepath are printed. */
        else if (!strcmp(arg_array[0], "ls"))
        {
            /* If ls has no arguments, the contents of the cwd are printed. */
            if (arg_num == 1) {
                dir_sel("./", arg_array);
                _exit(status);
            /* Otherwise, the contents of the specified filepath are printed. */
            } else {
                dir_sel(arg_array[1], arg_array);
                _exit(status);
            }
        }
        /* If user types in cd the current working directory is changed to */
        /* the directory specified in the second argument.                 */
        else if(!strcmp(arg_array[0], "cd"))
        {
            int DirEntry = chdir(arg_array[1]);
            if (DirEntry == 0) {
                return(status);
            } else {
                fprintf(stderr, "Error: cannot cd into directory");
                status = 2;
                return(status);

            }
        }
        else {
            /* using execv for array of arguments, -p to access PATH variables */
            execvp(arg_array[0], arg_array);
            _exit(EXIT_FAILURE);
        }
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
void buf_clear(int arg_num, char** arg_array)
{
    for (int i = 0; i < arg_num - 1; i++)
    {
        memset(arg_array[i], 0, strlen(arg_array[i]));
    }
}

int main(void)
{
    char* arg_array[ARG_MAX];
    char cmd[CMDLINE_MAX];
    int arg_num = 0;

    //DIR* dp;

    while (1)
    {
        char* nl;
        int retval;

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
        arg_num = funct_parse(cmd, arg_array);

        /* Exit must be within main in order to break from program */
        /* instead of breaking from a fork                         */
        if (!strcmp(arg_array[0], "exit"))
        {
            fprintf(stderr, "Bye...\n");
            break;
        }
        /* Calls System to execute non-exit command, actual command*/
        /* identification takes place in our_system function       */
        retval = our_system(arg_array, arg_num);

        /* Returns value from non-exit command */
        fprintf(stderr, " + completed '%s' [%d]\n", cmd, retval);
    }
    return EXIT_SUCCESS;
}
#endif //SSHELL_C_
