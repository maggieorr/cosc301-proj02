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

int commentindex(const char *buffer){
	int i =0;
	while (buffer[i] != '\0'){
		if (buffer[i]== '#'){
			return i;			
		}
		i++;
	}
	return -1;
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

char **separate_commands(const char *buffer[]){
	char *copy = strdup(buffer);
	char *semicolon= ";";
	char *token;
	char *tokencpy;
	int num_commands=0;
	for (token = strtok(copy,semicolon); token != NULL ; token= strtok(NULL,semicolon)){
		num_commands ++;	
	}
	free(copy);
	copy = strdup(buffer);
	char **j = malloc((num_commands+1)*sizeof(char *));
	int k = 0;
	for(int i=0;i<num_commands;i++){
		if (i==0){
			token = strtok(copy, semicolon);
			
		}
		else{
			token = strtok(NULL,semicolon);
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

void runsequential(char *commands[]){
	printf("got here\n");
	if (execv(commands[0],commands)<0){
		fprintf(stderr, "exec failed:%s\n", strerror(errno));
	}
	
}

void runparallel(char *commands[]){

}

int main(int argc, char **argv) {

    char buffer[1024];
    printf("prompt:: ");
    fflush(stdout);
    bool sequential = true;
    while (fgets(buffer, 1024, stdin) != NULL) {
        int bufflen = strlen(buffer);
        buffer[bufflen-1] = '\0';

	int index = commentindex(buffer);
	buffer[index] = '\0';
	char **commands = separate_commands(buffer);
	if (sequential==true){
		printf("here\n");
		runsequential(commands);
	}
	else{
		runparallel(commands);
	}
	printf("prompt:: ");
	fflush(stdout);
    }
    printf("bye now\n");

    return 0;
}

