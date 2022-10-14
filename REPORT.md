Authors: Bradley Manzo, Kent Cassidy

#Report 1
	In order to implement a simple functioning shell, our development consisted 
of three distinct phases: parsing, built-in functions, and piping/redirecting. 
## Parsing
	When retrieving the command line from the user, there are a series of 
processes that must take place. The first step of our solution is inserting
spaces surrounding the redirect characters achieved through simple iteration. 
This step is necessary because they are not guaranteed to be separate arguments 
and it critical for our next function separates the string into an array
of strings using spaces as tokens with strtok(). Once each argument is its own
string, the command line is now in a form that our program can interpret. Before
instantiating the command line as a custom struct, the first value is read to
check if any of the built-in functions are being called. 
## Built-In Functions
	The first of the built-in functions is exceedingly simple, breaking from the
main process when the user calls exit.
## Piping
	Currently, there are two instances of piping in the code. The working one
takes a single piped input, while the yet broken one is designed to take in as
many as needed. Should the parsed input be more than a simple command (<>|), the
arguments will be passed into function forkSetRun, taking in the number of pipes
(separate commands) as well as the arrayed input. Initially, an empty 2D array
of dynamically appropriate size is declared, to be called into the pipe() function.
The 2D array is now filled with the appropriate file descriptors for each pipe's
instream and outstream. This process is done before the fork, as every single
forked process requires access to these descriptors to connect to each other.
(Pipes declared within a forked process are inaccessible from outside).
	I created a loop to run as many times as there are commands. Each run through
the loop creates a fork, whose resulting child's ID is stored into an array.
fork() returns a PID which can be any number > 0. To keep track of each process,
I used an iterator p to tally and keep track of a custom PID, which increments
from 0. The parent was set to wait for all processes as the end of the function.
Though there is usually a race condition for each thread to finish, a properly
implemented pipe should block processes until they receive the necessary input.
	Now that processes are ordered by traceable, incrementing integers, they can
interact with neighboring pipes using their own ID as an index. Each process with
ID i will be hooked up to the out(1) of pipe i-1, and to the in(0) of pipe i.
Pi0  Pi1
|  \ |  \
Pr0 Pr1 Pr2

Finally, once each process properly hooks up their own in and out to each respective
pipe, the children call execv using data from the predefined array of process structs.
## Redirecting
	The first command (in a series of piped commands) may take an input redirection
from a file. The final command may also redirect its output into a file. This is a
similar process to piping, in which the file descriptors 0 and 1 are swapped with
the returned fd from open, rather than the return of pipe().
## Process Structure Type
	To keep track of each process and its properties, we defined a struct (yet
unused due to broken code) for each command and its arguments. To construct the
structure, an input of an array of strings is taken. The array of strings is the
previously parsed strtok returns. This would operate while iterating over each string
to figure out its role. The first element, as well as every element immediately after
a pipe character would create a new struct instance (to be pushed onto the array of struct
instances - representing an array of processes), and define itself as the processName command.
All subsequent arguments, if not <, >, or |, will be placed into the "arguments" portion -
yet another array of strings to be passed as the second argument in execv. Each structure
instance only needs to represent a respective process thread, as each file in/out or pipe
must be tied to an existing process. If there is a pipe between Process 1 and Process 2,
Process 1 needs only to acknowledge "pipeOut = true", while Process 2 needs only to acknowledge
"pipeIn = true". If a redirect is involved, each process denotes the type of redirect (in/out)
as well as the filename. From parsing onwards, each function is to refer to the data
within the structs for easy organization and reading.
Which is what I would say if my portion of dynamic piping and redirecting works.
My wonderful and ingenious partner Bradley Manzo was able to implement his own threads of
single piping and single redirects in the time it's taking me to squash bugs in my code.
