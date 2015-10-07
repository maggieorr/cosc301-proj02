#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

static bool sequential = true;

int commentindex(const char *buffer){
	int i =0;
	while (buffer[i] != '\0'){
		if (buffer[i]== '#'){
			return i;			
		}
		i++;
	}
	return strlen(buffer);
}

void removewhitespace(char s[]) {
	int j=0;
    	for(int i =0; i<strlen(s); i++){
		if(!isspace(s[i])){
			s[j]=s[i];
			j+=1;
}
}
	s[j]='\0';
}

char **separate_commands(const char *buffer[], const char *separateby){
	char *copy = strdup(buffer);
	char *token;
	char *tokencpy;
	int num_commands=0;
	for (token = strtok(copy,separateby); token != NULL ; token= strtok(NULL,separateby)){
		num_commands ++;	
	}
	free(copy);
	copy = strdup(buffer);
	char **j = malloc((num_commands+1)*sizeof(char *));
	int k = 0;
	for(int i=0;i<num_commands;i++){
		if (i==0){
			token = strtok(copy, separateby);
			
		}
		else{
			token = strtok(NULL,separateby);
		}
		tokencpy= strdup(token);
		removewhitespace(tokencpy);
		if (strcmp(tokencpy,"")!=0){
			j[k]=strdup(token);
			k++;
		}
		free(tokencpy);
			
	}
	j[k]=NULL;
	free(copy);
	return j;

}
void print_tokens(char *tokens[]) {
    int i = 0;
    while (tokens[i] != NULL) {
        printf("Token %d: %s\n", i+1, tokens[i]);
        i++;
    }
}
int runsequential(char *commands[]){
//is this already parallel??
	int i =0;
	while (commands[i] != NULL){
		char **args= separate_commands(commands[i], " \n\t");
		if (strcmp(args[0], "mode")==0){
			if (strcmp(args[1], "p")==0){
				printf("got here\n");
				sequential = false;
				return 0;
			}
			else if (strcmp(args[1], "s")==0){
				printf("got here\n");
			}
			else {
				fprintf(stderr, "execv failed: %s\n", strerror(errno));
			}
			//need a case to wrongly execv all other modes? e.g. mode x
			//currently prints <execv failed: Success> so prob need to do something dif
		}
		else{
			pid_t pid = fork();
			if (pid==0){
				if (execv(args[0], args) < 0) {
					fprintf(stderr, "execv failed: %s\n", strerror(errno));
				}
			}
			if (pid >0){
				int status=0;
			   	wait(&status);
			}
		}
		i++;
	}
	return 0;
	
}

void runparallel(char *commands[]){
	

}


int main(int argc, char **argv) {
    sequential = true;
    char buffer[1024];
    printf("prompt:: ");
    fflush(stdout);
    while (fgets(buffer, 1024, stdin) != NULL) {
        int bufflen = strlen(buffer);
        buffer[bufflen-1] = '\0';
	int index = commentindex(buffer);
	buffer[index] = '\0';
	char **commands = separate_commands(buffer, ";");
	if (sequential){
		runsequential(commands);
	}
	else{
		runparallel(commands);
	}
	printf("prompt:: ");
	fflush(stdout);
    }
    printf("\nbye now\n");

    return 0;
}

