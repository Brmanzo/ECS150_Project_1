
void forkSetRun() {

	int processCount = pipe_count + 1;
	int pipefds[processCount - 1][2];
	int myProcessIndex = 1; // Manually counted PID to be copied / passed

	for (int i = 0; i < processCount; i++) {
		pipe(pipefds[i]);
	}

	for (int j = 0; j < processCount; j++) { 
		if (fork()) { //child process
			if (myProcessIndex == 1) {
				dup2(pipefds[myProcessIndex-1][0], STDOUT_FILENO);
				if (org_cmd_array[myProcessIndex-1].redirIn) {
					int redirfd = open(org_cmd_array[myProcessIndex-1].filename);
					dup2(redirfd, STDIN_FILENO);
				}
			} else if (myPID == processCount) {
				dup2(pipefds[myProcessIndex-2][1], STDIN_FILENO);
				if (org_cmd_array[myProcessIndex-1].redirIn) {
					int redirfd = open(org_cmd_array[myProcessIndex-1].filename);
					dup2(redirfd, STDOUT_FILENO);
				}
			} else {
				dup2(pipefds[myProcessIndex-2][1], STDIN_FILENO);
				dup2(pipefds[myProcessIndex-1][0], STDOUT_FILENO);
			}
			for (int k = 0; k < processCount - 1; k++) {
				int pipeFDIn = 3 + (2 * k);
				int pipeFDOut = 4 + (2 * k);
				close(pipeFDIn);
				close(pipeFDOut);
			}
			execvp(org_cmd_array[myProcessIndex-1].processName, org_cmd_array[myProcessIndex-1].processArgs);
		} else {
			myID++;
		}
	}

	//Everything after above for loop is supposed to be parent only process
	int status = 0;
	int retstatus;
	while((retstatus = wait(&status)) > 0); //source: https://stackoverflow.com/questions/19461744/how-to-make-parent-wait-for-all-child-processes-to-finish

	return retstatus;
}

