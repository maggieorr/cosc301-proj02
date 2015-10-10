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

//create struct for linked list of directories
struct Node {
        char dir[1024];
        struct Node *next;
        } ;
        
//create struct for linked list of jobs        
struct job{
	pid_t pidID;
	char command[1024];
	char status[10];
	struct job *next;
	};

//delete a job specified by it's pid number
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

//increase the size of the linked list of jobs by 1 (whenever a new one is typed)
struct job *list_append(pid_t id, char *command, struct job *list) {
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

//ignore comments
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

//remove all white space from a string
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

//separate a string by a specified separator
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

//check if the user wants to pause or resume one of the PIDs. If yes, return true and update. 
//If no return false
bool pauseOrResume(char *args[], struct job *jobs){
	if (args[1]==NULL){return false;}
	int pid = atoi(args[1]);
	int correctpid=0;
	struct job *head=jobs;
	while(head!=NULL){
		if (head->pidID == pid){
			correctpid = pid;
			break;
		}
		head= head->next;
	}
	if (correctpid==0){
			return false;
		}
        if(strcmp(args[0], "pause") == 0){
                kill(correctpid, SIGSTOP);
                strncpy(head->status,"paused",sizeof(char)*10);
                return true;
        }
        if(strcmp(args[0], "resume") == 0){
                kill(correctpid,SIGCONT);
                strncpy(head->status,"running",sizeof(char)*10);
                return true;
        }
        return false;
}

//check if user typed any built-in commands i.e. mode or exit
//if yes, return true -- also do the designated function of each command
//if no, return false
bool checkcases(char **args, bool *sequential, bool *ex, const char mode[]){
	if (strcmp(args[0], "mode")==0){
		        if (args[1] == NULL){
			        printf("Current mode is: %s\n", mode);
			        return true;
			}
			else if (strcmp(args[1], "p")==0 ||  strcmp(args[1], "parallel")==0){
				*sequential = false;
				return true;
			}
			else if (strcmp(args[1], "s")==0 || strcmp(args[1], "sequential")==0){
			        *sequential= true;
			        return true;
			}
			
		}
	else if (strcmp(args[0], "exit")==0){
			*ex = true;
			return true;
	}
	return false;
}


int runsequential(char *commands[], bool *sequential, bool *ex, struct Node *head){
	int i =0;
	while (commands[i] != NULL){
		struct Node *cpy= head;
		char **args= separate_commands(commands[i], " \n\t");
		if(!checkcases(args, sequential, ex, "SEQUENTIAL")){
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

//print a lit of the current jobs including pid ID, command, and status i.e. running or pausing
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
		if (strcmp(args[0], "jobs")==0 || pauseOrResume(args,*jobs)){
			printjobs(*jobs);
		}
		else if (!checkcases(args, sequential, ex, "PARALLEL")){
			//make sure stdin is a valid path
		       struct stat statresult;
                        int rv = stat(args[0], &statresult);
                        char newarg[1024];
                        strncpy(newarg,args[0],1024);
                        while (rv < 0 && cpy!=NULL) {           //check with directories
                                strncpy(newarg,cpy->dir,1024);
                                strcat(newarg,args[0]);
                                rv = stat(newarg, &statresult);
                                cpy=cpy->next;
                        }
                        char fullarg[1024]; 
                        strncpy(fullarg,newarg,1024);
                        int i=1;
                        while(args[i]!='\0'){
                       		strcat(fullarg,args[i]);
                       		i++;
                       	}
                        if (rv<0){
                        	fprintf(stderr, "execv failed: %s\n", strerror(errno));
                        }
                       
                        //if it is a valid path then fork it             
                        else{
		        num_children++;
			pid_t pid = fork();
			*jobs = list_append(pid, fullarg, *jobs);
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

//read the file and add contents to a linked list of directories
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
//free linked list of directories
void free_dir(struct Node *dirlist) {
	while (dirlist != NULL){
		struct Node *tmp = dirlist;
		dirlist = dirlist -> next;
		free(tmp);
	}
}
//free linked list of jobs
void free_jobs(struct job *joblist) {
	while (joblist != NULL){
		struct job *tmp = joblist;
		joblist = joblist -> next;
		free(tmp);
	}
}
//run the shell 
int runshell(struct Node *head1, struct job *jobs){
    bool sequential = true;
    bool ex = false;
    char buffer[1024];
    printf("prompt:: ");
    fflush(stdout);
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
	printf("prompt:: ");
	fflush(stdout);
	if(!sequential){
		//this infinite loop allows for the process to continue in the background
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
		    //check if any of the children are done
		    while((tostop=waitpid(-1, &status, WNOHANG))>0){
			jobs = list_delete(tostop, jobs);
			printf("Process %d completed!\n",tostop);	
		    }
			//make sure you're not exiting or switching modes
		    if (ex && jobs==NULL){
			break;
		    }
		    if (sequential && jobs==NULL){
			break;
		    }
		  
		}
		printf("prompt:: ");
		fflush(stdout);	
	}
	
	if (ex){
	        free_dir(head1);
	        printf("\n--bye now!!--\n");   //exits in the case of user typing "exit"
	        exit(1);
	}
    }
    printf("\n--bye now!!--\n");   //exits in the case of the user typing "ctrl+d"
    return 0;
}

int main(int argc, char **argv) {
    struct Node *head1 = NULL;
    struct job *jobs= NULL;
    FILE *datafile = fopen("shell-config", "r");
    readfile(datafile,&head1);
    fclose(datafile);
    runshell(head1, jobs);
    free_dir(head1);
    return 0;
}
