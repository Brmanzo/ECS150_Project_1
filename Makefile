sshell:  sshell.c
	gcc	-Wall	-Wextra	-Werror	-g	sshell.c	-o	sshell

clean:
	rm	sshell	$(objects)