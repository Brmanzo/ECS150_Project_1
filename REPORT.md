Authors: Bradley Manzo, Kent Cassidy

#Report 1
	In order to implement a simple functioning shell, our development consisted
of four distinct phases: parsing the input, developing the built-in functions
and the directory stack, devising a proper struct to store the argument array,
and implementing piping and redirection.
## Parsing
	When retrieving the command line from the user, there are a series of
processes that must take place. The first step of our solution is inserting
spaces surrounding the redirect characters using our redir_space function. This
is achieved through simple string iteration and is necessary because '<' and '>'
are not guaranteed to be separate arguments in the command line. This is
critical for our next function funct_parse to separate the string into
individual arguments stored in an array of strings. This is achieved using the
strtokfunction while treating spaces as tokens. Once every argument is its own
string,the command line is now in a form that our program can interpret. Upon
eachsuccessive loop of the shell, we call our buf_clear function to flush the
our arg_array which is allocated with 16 spaces (the maximum arguments). Before
instantiating the command line as a custom struct, the first value is read to
check if any of the built-in functions are being called.
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
## Designing the struct

##Piping and Redirection
