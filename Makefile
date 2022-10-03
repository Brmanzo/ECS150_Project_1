sshell:  sshell.c  our_system.h
	gcc	-Wall	-Werror	sshell.c  -o	sshell

clean:
	rm	sshell  our_system  $(objects)