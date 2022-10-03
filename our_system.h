#ifndef OUR_SYSTEM_H_
#define OUR_SYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define CMDLINE_MAX 512



int our_system(const char *cmd) {
	int status;
	pid_t pid;

	pid = fork();
	if(pid == 0){ // Child
		/* using execl for convenience, -p to access PATH variables */
		execlp(cmd, cmd, NULL);
		_exit(EXIT_FAILURE);
	} else if(pid < 0) {
		/* The fork failed. Report failure */
		status = -1;
	} else {
		/*This is the parent process. Wait for the child to complete */	
		  if( waitpid( pid, &status,0)!= pid){
		  	status = -1;
		  }
  }
return status;
}

#endif //OUR_SYSTEM_H_