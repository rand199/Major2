# Major2
//Abdal Alkaissi was responsible for the built in command, path, and the shell support function, signal handling and being a Scrum Master.

//Daniel Padierna was responsible for the built in command, CD, and the redirection function and being a team memeber.

//Melis T. was responsible for adding the MyHistory and Alias functions, and being a team member.

//Rand Alkassi was responsible for the built-in command exit, the pipelining function and being the Product Owner


//Brief design overview: The code first determines if it is in batchmode or interactive mode. Then it scans the input given by the user, it then parses the different commands in the same line if they were seperated by a semicolon. Then it parses the arguments for each command. Then it transfers the arrays of the commands and arguments to the execuction function where the function scans for any built in commands, if any are found then it sends it to the specific built in command function, if none are found then it executes the commands, prints the output and frees the allocated memory and then waits for the user to enter something else.

//Known Bugs: signal handling does not work


//Github: https://github.com/rand199/Major2
