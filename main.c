
//major2
#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#define command 50     //50 characters max received per command 


int main()
{
	char input[50];           //create string to store command 
	char* line;    //pointer to constant 

	char* args[command];   //a max of 50 arguments
	int count = 0, pid, i;
	
	printf("major2> ");     //initial print line to ask user for a command
	fgets(input, 100, stdin);      //read a line from a string

 
    
	while (strcmp(input, "quit\n"))  //string comparison to continue with loop
        {                                //while user does not input the command "quit"

		printf("minor3> ");      //ask for command

		count = 0;

		input[strlen(input) - 1] = '\0';    //string length

		for (i = 0; i < 10; i++)
		{
			args[i] = (char*)malloc(20 * sizeof(char)); //memory allocation
		}

		

		line = strtok(input, " ");   //tokenize string adn separate it when space is found

		strcpy(args[count++], line);   //copy every susbstring

 
		while (line != "quit")   //do until string is empty
		{
			line = strtok(NULL, " ");
			if (line == NULL)
				break;
			
			strcpy(args[count++], line);
		}

		//after building list of commands , execute command using execvp , for that create child process

		args[count] = NULL;
		pid = fork();   //return value of fork()


	
		if(pid == 0) // child process
            {
                execvp(args[0], args);      //output if wrong command
                printf("%s: Command not found\n", args[0]);
                break;
            }
                else if (pid > 0) { // parent process
                int status;
                waitpid(-1, &status, 0); 
            } else {
                printf("Error: fork() failed!\n");
            }
		
		
		
		fgets(input, 100, stdin); //store string until a new command is given
	}

}
