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