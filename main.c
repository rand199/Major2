
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>   
#include <errno.h>    
#include <sys/wait.h> 

	char *input = NULL;
    size_t buffer = 0; 
	int counter;
	char *token;
	char *args[];

	int main(int argc, char *argv[]){
		while(1){
            printf("minor3> ");
            getline(&input, &buffer, stdin);
            	if(strcmp(input,"\n")==0){
					printf("Please type in a command\n" );
					continue;
                }

            token = strtok(input, "\n ");
			while (token != NULL) { 
                counter = 0;
				args[counter++] = token; 
				token = strtok(NULL, "\n ");
            	}
                args[counter] = NULL;
            
			if (strcmp(args[0], "quit") == 0) {
				return 0;
			}

            int pid = fork(); 
            if(pid == 0) // child process
            {
                execvp(args[0], args);
                printf("%s: Command not found\n", args[0]);
                break;
            }
                else if (pid > 0) { // parent process
                int status;
                waitpid(-1, &status, 0); 
            } else {
                printf("Error: fork() failed!\n");
            }
        }   
}