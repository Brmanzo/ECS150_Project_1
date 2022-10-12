processCount = pipe_count + 1;


declare pfd12[2]
declare pfd23[2]
declare pfd34[2]
 or
int pfd[3][2];

for each pfd[x]
	pipe(pfd[x]);

for each struct within structArray

	for processCount {
		if child
			break

		if parent {
			fork();

	}

	if child
		change file descriptors:
			pipe in: pipe(fd[2]) -> dup2(fd[0], STDIN), dup2(fd[1], STDOUT)
			 or
			file in:

			pipe out:
			 or
			file out:

		execvp with program name,
		

	else if parent (shell)

		wait for all child pids

		return


int pfd[processCount - 1][2];

for (int r = 0; r < processCount; r++) {
	pipe(pfd[i]);
}

/* say processCount == 3:
	sharedFileTable[0] == STDIN_FILENO
	sharedFileTable[3] == pfd[0][in == 0]         pid1[stdout] -> pfd[0][in]   // if(not last element) : dup2(pfd[pid-1][0], STDOUT_FILENO);
	sharedFileTable[4] == pfd[0][out == 1]	      pid2[stdin]  -> pfd[0][out]  // if(not first element) : dup2(pfd[pid-2][1], STDIN_FILENO);
	sharedFileTable[5] == pfd[1][in]	      pid2[stdout] -> pfd[1][in]   
	sharedFileTable[6] == pfd[1][out]     	      pid3[stdin]  -> pfd[1][out]
	                                	      pfd3[stdout] -> terminal out 

	


*/
