#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


#define MAX 512 //user's input is less than 512 bytes
int hist_count = 0; //global variable for MyHistory function
int alias_count = 0;  //global variable for MyAlias function
pid_t ppid; //gloabal parent id
pid_t cpid; //global child id

void InteractiveMode();
void BatchMode(char* file);

int ParseCommands(char* userInput); //e.g., "ls -a -l; who; date;" is converted to "ls -al" "who" "date"
int ParseArgs(char* full_line, char* args[]); //e.g., "ls -a -l" is converted to "ls" "-a" "-l"
void ExecuteCommands(char* command, char* full_line);

void MyCD(char* dir_input, int arg_count);
void MyExit();
void MyPath(char* args[], int arg_count);
void MyHistory(char* args[], int arg_count);

void CommandRedirect(char* args[], char* first_command, int arg_count, char* full_line);
void PipeCommands(char* args[], char* first_command, int arg_count);
void signalHandle(int sig);
void MyAlias(char* args[], int arg_count);
void io_redirect(char* filename);
char CURRENT_DIRECTORY[MAX]; //current directory
char* COMMANDS[MAX]; //commands to be executed
char* MYHISTORY[MAX]; //shell command history
char* MYPATH; //my PATH variable
char* MYALIAS[MAX]; //alias variable
const char* ORIG_PATH_VAR; //The original PATH contents
char* prompt;

int EXIT_CALLED = 0;//Functions seem to treat this as a global variable -DM


int main(int argc, char* argv[]) {
	//error checking on user's input
	if (!(argc < 3)) {
		fprintf(stderr, "Error: Too many parameters\n");
		fprintf(stderr, "Usage: './output [filepath]'\n");
		exit(0);//No memory needs to be cleared
	}

	//initialize your shell's enviroment
	MYPATH = (char*)malloc(1024);
	memset(MYPATH, '\0', sizeof(MYPATH));
	ORIG_PATH_VAR = getenv("PATH"); // needs to include <stdlib.h>

	//save the original PATH, which is recovered on exit
	strcpy(MYPATH, ORIG_PATH_VAR);

	//make my own PATH, namely MYPATH
	setenv("MYPATH", MYPATH, 1);

	if (argc == 1) InteractiveMode();
	else if (argc == 2) BatchMode(argv[1]);

	//gets the parent id and sets it to ppid
	ppid = getpid();

	//handles the signal (Ctrl + C)
	signal(SIGINT, signalHandle);

	//handles the signal (Ctrl + Z)
	signal(SIGTSTP, signalHandle);

	//free all variables initialized by malloc()
	free(MYPATH);

	return 0;
}

void BatchMode(char* file) {

	FILE* fptr = fopen(file, "r");
	//error checking for fopen function
	if (fptr == NULL) {
		fprintf(stderr, "Error: Batch file not found or cannot be opened\n");
		MyExit();
	}

	char* batch_command_line = (char*)malloc(MAX);
	memset(batch_command_line, '\0', sizeof(batch_command_line));

	//reads a line from fptr, stores it into batch_command_line
	while (fgets(batch_command_line, MAX, fptr)) {
		//remove trailing newline
		batch_command_line[strcspn(batch_command_line, "\n")] = 0;
		printf("Processing batchfile input: %s\n", batch_command_line);

		//parse batch_command_line to set the array COMMANDS[]
		//for example: COMMANDS[0]="ls -a -l", COMMANDS[1]="who", COMMANDS[2]="date"
		int cmd_count = ParseCommands(batch_command_line);

		//execute commands one by one
		for (int i = 0; i < cmd_count; i++) {
			char* temp = strdup(COMMANDS[i]); //for example: ls -a -l
			temp = strtok(temp, " "); //get the command
			ExecuteCommands(temp, COMMANDS[i]);
			//free temp
			free(temp);
		}
	}
	//free batch_command_line, and close fptr
	free(batch_command_line);
	fclose(fptr);
}

int ParseCommands(char* str) {

	int i = 0;

	char* token = strtok(str, ";"); //breaks str into a series of tokens using ;

	while (token != NULL) {
		//error checking for possible bad user inputs
		//Removes Spaces at beginning
		while (token[0] == ' ') {
			int size = strlen(token);
			for (int j = 0; j < size; j++) {
				token[j] = token[j + 1];
			}
		}

		//If after, removing all whitespaces we're left with a NULL char,
		//then the command is empty and will be ignored
		if (token[0] == '\0') {
			token = strtok(NULL, ";");
			continue;
		}

		//Removes all but one whitespace in between args
		for (int j = 0; j < strlen(token); j++) {
			//fprintf(stderr,"Token Edit: %s\n", token);
			if (token[j] == ' ' && token[j + 1] == ' ') {
				int size = strlen(token);
				for (int k = j; k < size; k++)
					token[k] = token[k + 1];
				j--;
			}
		}

		//save the current token into COMMANDS[]
		COMMANDS[i] = token;
		i++;
		//move to the next token
		token = strtok(NULL, ";");
	}

	return i;
}

void ExecuteCommands(char* command, char* full_line) {

	char* args[MAX]; //hold arguments

	MYHISTORY[hist_count % 20] = strdup(full_line); //array of commands
	hist_count++;

	//save backup full_line
	char* backup_line = strdup(full_line);

	if (strcmp(command, "alias") == 0 && strchr(full_line, '=') != NULL) {
		//break full_line into a series of tokens by the delimiter space (or " ")
		char* token = strchr(full_line, ' ');
		while (token[0] == ' ') {
			int size = strlen(token);
			for (int j = 0; j < size; j++) {
				token[j] = token[j + 1];
			}
		}
		MYALIAS[alias_count] = strdup(token);
		alias_count++;
	}
	else {
		//parse full_line to get arguments and save them to args[] array
		int arg_count = ParseArgs(full_line, args);

		//restores full_line
		strcpy(full_line, backup_line);
		free(backup_line);

		//check if built-in function is called
		if (strcmp(command, "cd") == 0)
			MyCD(args[0], arg_count);
		else if (strcmp(command, "exit") == 0)
			EXIT_CALLED = 1;
		else if (strcmp(command, "path") == 0)
			MyPath(args, arg_count);
		else if (strcmp(command, "myhistory") == 0)
			MyHistory(args, arg_count);
		else if (strcmp(command, "alias") == 0)
			MyAlias(args, arg_count);
		else
			CommandRedirect(args, command, arg_count, full_line);
		//free memory used in ParsedArgs() function
		for (int i = 0; i < arg_count - 1; i++) {
			if (args[i] != NULL) {
				free(args[i]);
				args[i] = NULL;
			}
		}
	}
}

int ParseArgs(char* full_line, char* args[]) {
	int count = 0;

	//break full_line into a series of tokens by the delimiter space (or " ")
	char* token = strtok(full_line, " ");
	//skip over to the first argument
	token = strtok(NULL, " ");

	while (token != NULL) {
		//copy the current argument to args[] array
		args[count] = strdup(token);
		count++;
		//move to the next token (or argument)
		token = strtok(NULL, " ");
	}

	return count + 1;
}

void CommandRedirect(char* args[], char* first_command, int arg_count, char* full_line) {
	pid_t pid;
	int status;

	//if full_line contains pipelining and redirection, error displayed
	if (strchr(full_line, '|') != NULL && (strchr(full_line, '<') != NULL || strchr(full_line, '>') != NULL)) {
		fprintf(stderr, "Command cannot contain both pipelining and redirection\n");
	}
	//if full_line contains "<" or ">", then io_redirect() is called
	else if (strchr(full_line, '<') != NULL || strchr(full_line, '>') != NULL) {
	void io_redirect(char* filename);
	}
	//if full_line contains "|", then PipeCommands() is called
	else if (strchr(full_line, '|') != NULL) {
	PipeCommands(args, first_command, arg_count);
	}
	else {//else excute the current command
	//set the new cmd[] array so that cmd[0] hold the actual command
	//cmd[1] - cmd[arg_count] hold the actual arguments
	//cmd[arg_count+1] hold the "NULL"
	char* cmd[arg_count + 1];
	cmd[0] = first_command;
	for (int i = 1; i < arg_count; i++)
		cmd[i] = args[i - 1];
	cmd[arg_count] = '\0';

	pid = fork();
	if (pid == 0) {
		execvp(*cmd, cmd);
		fprintf(stderr, "%s: command not found\n", *cmd);
		MyExit();//Ensures child exits after executing command
	}
	else wait(&status);
	}
}

void InteractiveMode() {

	int status = 0;

	//get custom prompt
	prompt = (char*)malloc(MAX);
	printf("Enter custom prompt: ");
	fgets(prompt, MAX, stdin);

	//remove newline from prompt
	if (prompt[strlen(prompt) - 1] == '\n') {
		prompt[strlen(prompt) - 1] = '\0';
	}

	while (1) {
		char* str = (char*)malloc(MAX);

		printf("%s> ", prompt);
		fgets(str, MAX, stdin);

		//error checking for empty commandline
		if (strlen(str) == 1) {
			continue;
		}

		//remove newline from str
		if (str[strlen(str) - 1] == '\n') {
			str[strlen(str) - 1] = '\0';
		}

		//parse commands
		int cmd_num = ParseCommands(str);//this function can be better designed

		//execute commands that are saved in COMMANDS[] array
		for (int i = 0; i < cmd_num; i++) {
			char* temp = strdup(COMMANDS[i]);
			temp = strtok(temp, " ");
			ExecuteCommands(temp, COMMANDS[i]);
			//free temp
			free(temp);
		}

		//ctrl-d kill

		free(str);

		// if exit was selected
		if (EXIT_CALLED) {
			free(prompt);
			MyExit();
		}
	}
}

void MyCD(char* dir_input, int arg_count) {

	
    //if no arguments are passed. Go to home directory
	if (arg_count == 1) 
	{
		if (chdir("/nfs/home") != 0) {
			fprintf(stderr, "error with chdir() to %s failed", dir_input);
			perror("");
		}
    //target directory is passed
	}
	else if (arg_count == 2)
	{   //
		if (chdir(dir_input) != 0)  //error message  if directory is not found
		{
			fprintf(stderr, "error with chdir() to %s failed, ", dir_input);
			perror("");
		}
	}
	else if (arg_count > 2) //error message if more than two arguments are passed
	{
		printf("Too many arguments\n");
	}
	else    //error message to avoid program crashing
		printf("Something went wrong\n");


}

void MyExit() { // printf free malloc (IGNORE: For highlighting puposes) 
	exit(EXIT_SUCCESS);
}

void MyPath(char* args[], int arg_count) {
	/*
	//initialize your shell's enviroment
	MYPATH = (char*)malloc(1024);
	memset(MYPATH, '\0', sizeof(MYPATH));
	ORIG_PATH_VAR = getenv("PATH"); // needs to include <stdlib.h>
	//save the original PATH, which is recovered on exit
	strcpy(MYPATH, ORIG_PATH_VAR);
	//make my own PATH, namely MYPATH
	setenv("MYPATH", MYPATH, 1);
	*/
	int argCount = arg_count--; //path + bin is 3-1=2. 
	int index = argCount--; //args[max] = index which is 2-1=1. 
	if (argCount == 0)
	{
		printf("%s\n",MYPATH);
		//printf("%s\n",ORIG_PATH_VAR);
	}
	else if ((strcmp(args[0], "+") == 0) && (argCount == 2))
	{
		strcat(MYPATH, ":");
		strcat(MYPATH, args[1]);
		//setenv("PATH", MYPATH, 1);
		printf("original = %s\n", ORIG_PATH_VAR);
		printf("mypath = %s\n", MYPATH);
	}
	else if ((strcmp(args[0], "-") == 0) && (argCount == 2))
	{
		char* str = MYPATH;
		char* sub = args[1];
		char* p, * q, * r;
		if (*sub && (q = r = strstr(str, sub)) != NULL)
		{
			size_t len = strlen(sub);
			while ((r = strstr(p = r + len, sub)) != NULL)
			{
				memmove(q, p, r - p);
				q += r - p;
			}
			memmove(q, p, strlen(p) + 1);
		}

		//setenv("PATH", str, 1);
		printf("original = %s\n", ORIG_PATH_VAR);
		printf("mypath = %s\n", MYPATH);
	}
	else if (argCount >= 3)
		printf("Error! too many arguments, this command has this format below.\npath(without arguments) displays the pathnames currently set\npath + ./bin appends the pathname to the path variable. Only one pathname may be added at a time\npath -./ bin removes the pathname to the path variable. Only one pathname may be removed at a time\n");
	else
		printf("Error! Wrong input, this command has this format below.\npath(without arguments) displays the pathnames currently set\npath + ./bin appends the pathname to the path variable. Only one pathname may be added at a time\npath -./ bin removes the pathname to the path variable. Only one pathname may be removed at a time\n");

}

void MyHistory(char* args[], int arg_count) {
    int argCount = arg_count--;
    if (arg_count == 0)
    {
        printf("%s\n", "\nShell Command History: ");
	    for (int i = 0; i < hist_count; i++) {
	        printf("%d : %s\n", i+1, MYHISTORY[i]);
	    }
    } 
    else if ((strcmp(args[0], "-c") == 0) && (arg_count == 1))
    {
	    for (int i = 0; i < hist_count; i++) {
    	    MYHISTORY[i] = "";
	        hist_count = 0;
	        printf("%s\n", "Shell Command History has been cleared");
	   }
    }
    else if ((strcmp(args[0], "-e") == 0) && (arg_count == 2))
    {
		if ((atoi(args[1]) > 0)&&((atoi(args[1]) <= hist_count)))
		{
		    printf("Commmand to run : %s\n", MYHISTORY[atoi(args[1])-1]);
		    char* temp = strdup(MYHISTORY[atoi(args[1])]); //for example: ls -a -l
		    temp = strtok(temp, " "); //get the command
		    ExecuteCommands(temp, MYHISTORY[atoi(args[1])-1]);
		    //free temp
		    free(temp);
		}
		else
		{
		    printf("Invalid history number!\n");
		}
    }
    else
    {
        printf("Invalid parameter(s) or parameter count\n");
    }
}

void PipeCommands(char* args[], char* first_command, int arg_count) {
     int i;

    for( i=1; i<arg_count-1; i++)
    {
        int pd[2];
        pipe(pd);

        if (!fork()) {
            dup2(pd[1], 1); // remap output back to parent
            execlp(args[i], args[i], NULL);
            perror("exec");
            abort();
        }

        // remap output from previous child to input
        dup2(pd[0], 0);
        close(pd[1]);
    }

    execlp(args[i], args[i], NULL);
    perror("exec");
    abort();
}

void signalHandle(int sig) {

}


void io_redirect(char* filename) {
  // Check if filename is NULL
  if (!filename) {
    printf("Usage: ./program filename < filename\n");
    return;
  }

  // Open file for reading
  FILE *fp = fopen(filename, "r");

  // Check if fopen() succeeds
  if (!fp) {
    perror("fopen");
    return;
  }

  // Redirect standard input to read from file
  int result = dup2(fileno(fp), STDIN_FILENO);

  // Check if dup2() succeeds
  if (result == -1) {
    perror("dup2");
    return;
  }

  // Close file
  fclose(fp);
}

void MyAlias(char* args[], int arg_count) {
    
    int argCount = arg_count--;
    //if alias array is empty
    if (MYALIAS[0] == NULL) 
        {
            printf("No aliases to display \n");
        }
    //prints current list of alias commands
    else if (arg_count == 0){
        printf("%s\n", "\nCurrent alias commands in place: ");
	    for (int i = 0; i < alias_count; i++) {
	        printf("%d : %s\n", i+1, MYALIAS[i]);
	    }
    } 
    //remove all cases in alias
    else if ((strcmp(args[0], "-c") == 0) && (arg_count == 1))
    {
	    for (int i = 0; i < alias_count; i++) {
    	    MYALIAS[i] = "";
	        alias_count = 0;
	        printf("%s\n", "Alias Commands have been cleared");
	   }
    }
   else if ((strcmp(args[0], "-r") == 0) && (arg_count == 2))
    {
    	    
	        //alias_count = 0;
	        printf("%s\n", "Alias Command has been deleted");
	   
    }
    

}


