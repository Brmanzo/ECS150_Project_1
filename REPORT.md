Authors: Bradley Manzo, Kent Cassidy
Date: October 13, 2022

#Report 1
	In order to implement a simple functioning shell, our development consisted
of several phases: parsing the input, developing the built-in functions
and the directory stack, devising a proper struct to store the argument array,
and implementing piping and redirection.

## Parsing
	When retrieving the command line from the user, there are a series of
processes that must take place. The first step of our solution is inserting
spaces surrounding the redirect and pipe characters using our insert_spaces 
function. This is achieved through simple string iteration and is necessary
because '<', '>' and '|' are not guaranteed to be separate arguments in the
command line. This is critical for our next function funct_parse to separate the
string into individual arguments stored in an array of strings. This is achieved
using the strtok function while treating spaces as tokens. Once every argument
is its own string, the command line is now in a form that our program can
interpret. Upon each successive loop of the shell, we call our buf_clear
function to flush our arg_array which is allocated with 16 spaces
(the maximum arguments). The first value is read to check if any of the built-in
functions are being called.

## Built-In Functions
	The first of the built-in functions is exceedingly simple, breaking from the
main process when the user calls exit. The rest of our built-in functions are
compartmentalized in a separate function. If arg_array[0] is "pwd" the current
directory is printed to the terminal, and if "cd", the current directory is
changed to the filepath in arg_array[1]. The following commands are part of the
family responsible for the directory stack. "pushd" changes the directory like
"cd" but also uses "pwd" to add the new working directory to the stack which is
formed from an array. The index is incremented with each successive "pushd" and
acts as a pointer for where the new directory should be placed. "popd" works
similarily where the index is decremented twice to "cd" into the previous
directory, and then incremented once more to point to where the new directory
should "pushd" to. Since this array stores the most recent directory at the
greatest index, the stack is printed in reverse. 

## Piping
	Currently, there are two instances of piping in the code. The working
version takes a single piped input, while the deprecated one is designed to take
in as many as needed. Should the parsed input be more than a simple command
(<>|), the arguments would be passed into function forkSetRun, taking in the 
number of pipes (separate commands) as well as the arrayed input. Initially, an 
empty 2D array of dynamically appropriate size is declared, to be called into
the pipe() function. The 2D array is now filled with the appropriate file
descriptors for each pipe's instream and outstream. This process is done before
the fork, as every single forked process requires access to these descriptors to
connect to each other. (Pipes declared within a forked process are inaccessible
from outside).
	We created a loop to run as many times as there are commands. Each run
through the loop creates a fork, whose resulting child's ID is stored into an
array. fork() returns a PID which can be any number > 0. To keep track of each
process, we used an iterator p to tally and keep track of a custom PID, which
increments from 0. The parent was set to wait for all processes as the end of
the function. Though there is usually a race condition for each thread to
finish, a properly implemented pipe should block processes until they receive
the necessary input.
	Now that the processes are ordered by traceable, incrementing integers, they
can interact with neighboring pipes using their own ID as an index. Each process
with ID i will be hooked up to the out(1) of pipe i-1, and to the in(0) of pipe
i.

Pi0  Pi1
|  \ |  \
Pr0 Pr1 Pr2

Finally, once each process properly hooks up their own in and out to each
respective pipe, the children call execv using data from the predefined array of
process structs.

## Redirecting
	In order to implement redirection, we first had to locate the position of
the redirect character in our array of arguments. The same function is applied
to each character with minor differences including the channel (stdin or stdout)
and the according character to search arguments for. Once the position of the
desired character is determined, we separate the preceding arguments into a
separate array in order to pass to execvp as arguments. The specified file is
opened onto the desired channel using dup2(), the original descriptor is closed,
and the array of parameters is executed. If the file is unable to be opened, or
if the redirect character is in an inappropriate positon, the proper error is
thrown.
## Process Structure Type
	To keep track of each process and its properties, we defined a struct
(deprecated due to broken code) for each command and its arguments. To construct
the structure, an input of an array of strings is taken. The array of strings is
the previously parsed strtok returns. This would operate while iterating over
each string to figure out its role. The first element, as well as every element
immediately after a pipe character would create a new struct instance (to be
pushed onto the array of struct instances - representing an array of processes),
and define itself as the processName command. All subsequent arguments, if not
<, >, or |, will be placed into the "arguments" portion - yet another array of
strings to be passed as the second argument in execv. Each structure instance
only needs to represent a respective process thread, as each file in/out or pipe
must be tied to an existing process. If there is a pipe between Process 1 and
Process 2, Process 1 needs only to acknowledge "pipeOut = true", while Process 2
needs only to acknowledge "pipeIn = true". If a redirect is involved, each
process denotes the type of redirect (in/out) as well as the filename. From
parsing onwards, each function is to refer to the data within the structs for
easy organization and reading. Which is what we would say if my portion of
dynamic piping and redirecting works.

## Closing Remarks
	Although the scope of our submitted code is lacking in comparison with our
ambitious plans, we feel that we have learned an enormous amount about how the
unix environment works and stores files, how redirection works by duping over
i/o channels, and how piping can be used to perform inter-process communication.
While we weren't able to implement multi-piping, or properly integrate our
pipe and redirect functions, we are very proud of the work we are submitting and
are open to criticism regarding style and implementation, as learning how to
write readable code is just as important as writing functional code. Included in
our submission is a version is the struct frequently referenced in our report
that we are eager to discuss with you during office hours. We feel we were very
close to implementing a dynamic piping function and we would like to know if we
were on the right track. Thank you for your time.

