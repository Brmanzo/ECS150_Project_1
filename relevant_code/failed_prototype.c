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

#define CMDLINE_MAX 512
#define USR_ARG_MAX 16
#define TOK_LEN_MAX 32

typedef struct org_cmd {
    // Separate Name and Args for execv
    char processName[TOK_LEN_MAX + 1];
    char* processArgs[USR_ARG_MAX + 1];
    char filename[TOK_LEN_MAX + 1];

    // To allow / deny usage of fdIn and fdOut variables, error handling
    bool pipeIn;
    bool pipeOut;
    bool redirIn;
    bool redirOut;
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
        if (!strncmp(inputCmds[i], "|", 1)) {
	    usrProcessArr[p].processArgs[i] = malloc(2 * sizeof(char*));
	    usrProcessArr[p].processArgs[i] = NULL;
            usrProcessArr[p].pipeOut = true;
            usrProcessArr[++p].pipeIn = true;
            firstArg = true;
            continue;
        }
        if (!strncmp(inputCmds[i], "<", 1)) { //input redir, program < file	
	    usrProcessArr[p].processArgs[i] = malloc(2 * sizeof(char*));
            usrProcessArr[p].processArgs[i] = NULL;
	    usrProcessArr[p].redirIn = true;
	    strcpy(usrProcessArr[p].filename, inputCmds[i + 1]);
	    i++;
            firstArg = false;
            argCount = 0;
            continue;
        }
        if (!strncmp(inputCmds[i], ">", 1)) {	    
	    usrProcessArr[p].processArgs[i] = malloc(2 * sizeof(char*));
	    usrProcessArr[p].processArgs[i] = NULL;
            usrProcessArr[p].redirOut = true;
	    strcpy(usrProcessArr[p].filename, inputCmds[i + 1]);
	    i++;
            firstArg = false;
	    continue;
        }
        if (firstArg) {
	    fprintf(stderr, "PROCESS NAME\n");
            strcpy(usrProcessArr[p].processName, inputCmds[i]);
            firstArg = false;
            argCount = 0;
        } else {
            usrProcessArr[p].processArgs[argCount] = malloc(sizeof(char) * (strlen(inputCmds[i]) + 1));
	    strcpy(usrProcessArr[p].processArgs[argCount], inputCmds[i]);
            argCount++;
        }
    }
    return usrProcessArr;
}

void forkSetRun(int pipe_count, org_cmd* org_cmd_array) {

    int processCount = pipe_count + 1;
    int pipefds[processCount - 1][2];
    int myProcessIndex = 0; // Manually counted ProcessID to be copied / passed, not to be confused with actual PID
    int PIDs[processCount];

    for (int i = 0; i < processCount; i++) {
        if(pipe(pipefds[i]) == -1) {
	fprintf(stderr, "Pipe %d Failed\n", i);
	_exit(1);
	}
    }

    for (int j = 0; j < processCount; j++) {
	// sleep(1 + myProcessIndex); // Guarantees process n runs before n+1. Useful for testing
        PIDs[j] = fork();

	if (!PIDs[j]) { //child process
	  if (myProcessIndex == 0) {
	    close(pipefds[myProcessIndex][0]);
            dup2(pipefds[myProcessIndex][1], STDOUT_FILENO);
	    close(pipefds[myProcessIndex][1]);// close redundant input
            if (org_cmd_array[myProcessIndex].redirIn) {
              int redirfd = open(org_cmd_array[myProcessIndex].filename, O_RDONLY);
	      if(redirfd == -1) {
	      	fprintf(stderr, "FOPEN FAILED\n");
	      }
              dup2(redirfd, STDIN_FILENO);
            }
          }
          else if (myProcessIndex == processCount - 1) {
            close(pipefds[myProcessIndex][1]);
	    dup2(pipefds[myProcessIndex - 1][0], STDIN_FILENO);
            close(pipefds[myProcessIndex - 1][0]);// close redundant output
	    if (org_cmd_array[myProcessIndex].redirOut) {
              int redirfd = open(org_cmd_array[myProcessIndex].filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	      if(redirfd == -1) {
		fprintf(stderr, "FOPEN FAILED\n");
	      }
              dup2(redirfd, STDOUT_FILENO);
            }
          }
          else {
            dup2(pipefds[myProcessIndex - 1][0], STDIN_FILENO);
            dup2(pipefds[myProcessIndex][1], STDOUT_FILENO);
          }

          for (int k = 0; k < processCount - 1; k++) {
            close(pipefds[k][0]);
            close(pipefds[k][1]);
          }

          execvp(org_cmd_array[myProcessIndex].processName, org_cmd_array[myProcessIndex].processArgs);
          _exit(1);
        } else {
            myProcessIndex++;
        }
    }
    // Everything after above for loop is supposed to be parent only process
    int retvals[processCount];
    for (int l = 0; l < processCount; l++) {
    	retvals[l] = waitpid(PIDs[l], NULL, 0);
    }
    for (int l = 0; l < processCount; l++) {
    	fprintf(stderr, "Return Value is: %d for PID: %d\n", retvals[l], PIDs[l]);
    }
    return;
}

#endif //SSHELL_C_