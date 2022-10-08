struct usrProcess {
    
	// Separate Name and Args for execv
	char processName[TOK_LEN_MAX + 1];
	char* processArgs[ARG_MAX];
	
	char* fileName[TOK_LEN_MAX + 1];

	// To allow / deny usage of fdIn and fdOut variables
	// Possible that we only need one set, to remove if needed later
	bool pipeIn = false;
	bool pipeOut = false;
	bool redirIn = false;
	bool redirOut = false;

	// To be initialized after 
	int fdIn;
	int fdOut;
}

// Make sure to declare array of struct outside of this scope and pass by reference
usrProcess* organizeProcesses (usrProcess usrProcessArr[], char** inputCmds, int numCmds) { // numCmds includes | and <> symbols

	int p = 0; // iterator for process in usrProcessArr
	bool firstArg = true; // toggle within for loop
	int argCount = 0; // iterator for struct argument array parameter

	for (int i = 0; i < numCmds; i++) {
		if (inputCmds[i][0] == '|') {
			usrProcessArr[p]->pipeOut = true;
			usrProcessArr[++p]->pipeIn = true;
			firstArg = true;
			continue;
		} 
		if (inputCmds[i][0] == '<') { //input redir, program < file
			usrProcessArr[p]->redirIn = true;
			strcpy(usrProcessArr[p]->filename, inputCmds[i + 1]);
			i++;
			firstArg = false;
			continue;
		}
		if (inputCmds[i][0] == '>') {
			usrProcessArr[p]->redirOut = true;
			strcpy(usrProcessArr[p]->filename, inputCmds[i + 1]);
			i++;
			firstArg = false;
			continue;
		}
		if (firstArg) {
			strcpy(usrProcessArr[p]->processName, inputCmds[i]);
			firstArg = false;
			argCount = 0;
		} else {
			usrProcessArr[p]->processArgs[argCount] = inputCmds[i];
			argCount++;
		}
	}

	return usrProcessArr;
}

