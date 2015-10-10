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


//Maggie Orr and Katey Loughran - we worked on the whole thing together

struct Node {
        char dir[1024];
        struct Node *next;
        } ;
        
        
struct job{
	pid_t pidID;
	char command[1024];
	char status[10];
	struct job *next;
	};

struct job *list_delete(pid_t name, struct job *list) {
	struct job *head=list;
	if (head == NULL){
		return head;
	}
	printf("%s is done! ",list->command);
	struct job *next_node=list->next;
	if (list->pidID==name){
		list= head->next;
		free(head);
		return list;
	}
	while(next_node!=NULL){
		if (list->pidID==name){
			list->next=next_node->next;
			free(next_node);
			return head;
		}
		list=list->next;
		next_node=next_node->next;
	}
	return head;
}

struct job *list_append(pid_t id, char *command,  struct job *list) {
	struct job *new_node=(struct job *)malloc(sizeof(struct job));
	strncpy(new_node->command,command,1024);
	strncpy(new_node->status,"running",10);
	new_node->pidID = id;
	new_node->next=NULL;
	struct job *current = list;
	if (list==NULL){
		current=new_node;
		return current;
	}

	while(list->next!=NULL){
		list=list->next;
	}
	list->next=new_node;

	return current;


}


void free_commands(char **commands) {
    int i = 0;
    while (commands[i] != NULL) {
        free(commands[i]); 
        i++;
    }
    free(commands); 
}

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

char **separate_commands(const char *buffer, const char *separateby){
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


/*int print_jobs(

bool pauseOrResume(char *args[]){
        bool found = false;
        if(strcmp(args[0], "pause") == 0){
                kill(0, SIGSTOP);
                found = true;
        }
        if(strcmp(args[0], "resume") == 0){
                kill(0,SIGCONT);
                found = true;
        }
        return found;
}*/


int runsequential(char *commands[], bool *sequential, bool *ex, struct Node *head){
	int i =0;
	while (commands[i] != NULL){
		struct Node *cpy= head;
		char **args= separate_commands(commands[i], " \n\t");
		if (strcmp(args[0], "mode")==0){
		        if (args[1] == NULL) {			  
			        printf("Current mode is: SEQUENTIAL\n"); 		       
			}
			else if (strcmp(args[1], "p")==0 ||  strcmp(args[1], "parallel")==0){
				*sequential = false;
			}
			else if (strcmp(args[1], "s")==0 || strcmp(args[1], "sequential")==0){
			        *sequential= true;
			}
			
		}
		else if (strcmp(args[0], "exit")==0){
			*ex = true;
		}

		else{
		        struct stat statresult;
                        int rv = stat(args[0], &statresult);
                        char newarg[1024];
                        strncpy(newarg,args[0],1024);
                        while (rv < 0 && cpy!=NULL) {
                                strcpy(newarg,cpy->dir);
                                strcat(newarg,args[0]);
                                rv = stat(newarg, &statresult);
                                cpy=cpy->next;
                        }
                        free(cpy);
                        if (rv<0){
                        	fprintf(stderr, "execv failed: %s\n", strerror(errno));
                        }
                        
                        else{
			pid_t pid = fork();
			if (pid==0){
				if (execv(newarg, args) < 0) {
					fprintf(stderr, "execv failed: %s\n", strerror(errno));
				}
			}
			if (pid >0){
				int status=0;
			   	wait(&status);
			}
			}
		}

		free_commands(args);
		i++;
	}
	

	return 0;
	
}

void printjobs(struct job *jobs){
	printf("Current Jobs:\n");
	if (jobs==NULL){printf("---no current jobs---\n");}
	int i = 1;
	while(jobs != NULL){
		printf("Job %d: process id is %d, command is %s, and process state is %s\n", i, jobs->pidID, jobs->command, jobs->status);
		jobs= jobs->next;
		i++;
	}
}

int runparallel(char *commands[], bool *sequential, bool *ex, struct Node *head, struct job **jobs){
       	int i =0;
       	int num_children=0;
	while (commands[i] != NULL){
		struct Node *cpy = head;
		char **args= separate_commands(commands[i], " \n\t");
		if (strcmp(args[0], "mode")==0){
		        if (args[1] == NULL){
			        printf("Current mode is: PARALLEL\n");
			       
			}
			else if (strcmp(args[1], "p")==0 ||  strcmp(args[1], "parallel")==0){
				*sequential = false;
			}
			else if (strcmp(args[1], "s")==0 || strcmp(args[1], "sequential")==0){
			        *sequential= true;
			}
			
		}
		else if (strcmp(args[0], "exit")==0){
			*ex = true;
		}
		else if (strcmp(args[0], "jobs")==0){
			printjobs(*jobs);
		}
		else{
		       struct stat statresult;
                        int rv = stat(args[0], &statresult);
                        char newarg[1024];
                        strncpy(newarg,args[0],1024);
                        while (rv < 0 && cpy!=NULL) {
                                strncpy(newarg,cpy->dir,1024);
                                strcat(newarg,args[0]);
                                rv = stat(newarg, &statresult);
                                cpy=cpy->next;
                        }
                      
                        if (rv<0){
                        	fprintf(stderr, "execv failed: %s\n", strerror(errno));
                        }
                        
                        else{
		        num_children++;
			pid_t pid = fork();
			*jobs = list_append(pid, commands[i], *jobs);

			if (pid==0){
				if (execv(newarg, args) < 0) {
					fprintf(stderr, "execv failed: %s\n", strerror(errno));
				}
			}
			}

		}
		free_commands(args);
		i++;
		
	}
	//this code was commented out for the purposes of background processing
	/*
	for(int i = 0; i< num_children; i++){
	        int status = 0;
	        wait(&status);
	}*/
	

	return 0;

}

void readfile(FILE *datafile,struct Node **head){
        struct Node *newnode;
        char line[1024];
        while(fgets(line, 1024, datafile)!=NULL){
                line[strlen(line)-1] = '/';
                line[strlen(line)] = '\0';
                newnode = (struct Node *) malloc(sizeof(struct Node));
                strncpy(newnode->dir,line,1024);
                newnode->next=*head;
                *head=newnode;
       }

}

void free_dir(struct Node *dirlist) {

	while (dirlist != NULL){
		struct Node *tmp = dirlist;
		dirlist = dirlist -> next;
		free(tmp);
	}
 
}


void free_jobs(struct job *joblist) {

	while (joblist != NULL){
		struct job *tmp = joblist;
		joblist = joblist -> next;
		free(tmp);
	}
 
}


int main(int argc, char **argv) {
    bool sequential = true;
    bool ex = false;
    char buffer[1024];
    printf("prompt:: ");
    fflush(stdout);
    struct Node *head1 = NULL;
    struct job *jobs= NULL;
    FILE *datafile = fopen("shell-config", "r");
    readfile(datafile,&head1);
    fclose(datafile);
    while (fgets(buffer, 1024, stdin) != NULL) {
        int bufflen = strlen(buffer);
        buffer[bufflen-1] = '\0';
	int index = commentindex(buffer);
	buffer[index] = '\0';
	char**commands;
	if (sequential){
		commands = separate_commands(buffer, ";");
		runsequential(commands, &sequential, &ex, head1);
		free_commands(commands);
	}
	else{
		while(1){
				  // declare an array of a single struct pollfd
		    struct pollfd pfd[1];
		    pfd->fd = 0; // stdin is file descriptor 0
		    pfd->events = POLLIN;
		    pfd->revents = 0;
		 
		    // wait for input on stdin, up to 1000 milliseconds
		    int rv = poll(&pfd[0], 1, 1000);
		 
		    // the return value tells us whether there was a 
		    // timeout (0), something happened (>0) or an
		    // error (<0).

		    if (rv == 0) {
			   
		    } else if (rv > 0) {
		    	char buffer[1024];
			if (fgets(buffer, 1024, stdin)==NULL){
				ex=true;
				}
			int bufflen = strlen(buffer);
			buffer[bufflen-1] = '\0';
			int index = commentindex(buffer);
			buffer[index] = '\0';
			commands = separate_commands(buffer, ";");
			runparallel(commands, &sequential, &ex, head1,&jobs);
			free_commands(commands);
			
		    } else {
			printf("there was some kind of error: %s\n", strerror(errno));
		    }
	
	   	int status;
		pid_t tostop;
		tostop = waitpid(-1, &status, WNOHANG);
		
		if (tostop > 0){
			jobs = list_delete(tostop, jobs);
			printf("Process %d completed!\n",tostop);	
		}
		
		if (ex && jobs==NULL){
				break;
			}
		
		
		}	
	}
	
	if (ex){
	        free_dir(head1);
	        printf("bye now\n");
	        exit(1);
	}
	
	printf("prompt:: ");
	fflush(stdout);
	
    }
    
    printf("bye now!!\n");
    free_dir(head1);
    return 0;
}

