#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdbool.h>

#define CMDLINE_MAX 512
#define TOK_LEN_MAX 32




typedef struct usrProcess {
    
	// Separate Name and Args for execv
	char processName[TOK_LEN_MAX + 1];
	char *processArgs[ARG_MAX];
	
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
}usrProcess;

// Make sure to declare array of struct outside of this scope and pass by reference
usrProcess* organizeProcesses (usrProcess usrProcessArr[], char* inputCmds[], int numCmds) { // numCmds includes | and <> symbols

	int p = 0; // iterator for process in usrProcessArr
	bool firstArg = true; // toggle within for loop
	int argCount = 0; // iterator for struct argument array parameter
	
	//initialize struct
	for (int j = 0; j < 5; j++) {
		usrProcessArr[j].pipeIn = false;
		usrProcessArr[j].pipeOut = false;
		usrProcessArr[j].redirIn = false;
		usrProcessArr[j].redirOut = false;	
	}

	for (int i = 0; i < numCmds; i++) {
		if (!strncmp(inputCmds[i], "|", 1)) {
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
			argCount = 0; //??

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
		if (firstArg) {
			strcpy(usrProcessArr[p].processName, inputCmds[i]);
			firstArg = false;
			argCount = 0;

			printf("Name: %s\n", usrProcessArr[p].processName);
			fflush(stdout);
		} else {
			usrProcessArr[p].processArgs[argCount] = malloc(sizeof(char) * strlen(inputCmds[i]));
			strcpy(usrProcessArr[p].processArgs[argCount], inputCmds[i]);
			argCount++;

			printf("Arg %d: %s\n", argCount - 1, usrProcessArr[p].processArgs[argCount - 1]);
			fflush(stdout);
		}
	}

	return usrProcessArr;
}

int main () {

	char *list[ARG_MAX] = {"echo", "Hello", "world", ">", "file"};
	usrProcess usrProcessArr[5]; //index size "num Pipes"

	organizeProcesses(usrProcessArr, list, 5);

	return 0;
}
