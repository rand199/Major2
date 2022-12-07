//Libaries:
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

//functions:
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

//variables/constants:
#define MAX 512 //user's input is less than 512 bytes
int hist_count = 0; //global variable for MyHistory function
int alias_count = 0;  //global variable for MyAlias function
pid_t ppid; //gloabal parent id
pid_t cpid; //global child id

char CURRENT_DIRECTORY[MAX]; //current directory
char* COMMANDS[MAX]; //commands to be executed
char* MYHISTORY[MAX]; //shell command history
char* MYPATH; //my PATH variable
char* MYALIAS[MAX]; //alias variable
const char* ORIG_PATH_VAR; //The original PATH contents
char* prompt;
int EXIT_CALLED = 0;//Functions seem to treat this as a global variable -DM
