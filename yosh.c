
/* -----------------------------------------------------------------------------
   FILE: shell.c

   NAME Het Patel

   DESCRIPTION: A SHELL SKELETON
   -------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "parse.h"   // local file include declarations for parse-related structs
#include <wordexp.h>
#include <errno.h>

int tildeFind(char* VarList[],int);
char* function(parseInfo* info, int com_index);

enum BUILTIN_COMMANDS { NO_SUCH_BUILTIN=0, EXIT, JOBS,CD,TIME,HISTORY,KILL,HELP};
int pidArray[50];
char* charPidArray[50];
int numPid;
char wd[50];
char* expansion;
/* -----------------------------------------------------------------------------
   FUNCTION: buildPrompt()
   DESCRIPTION: creates a prompt of working directory
   -------------------------------------------------------------------------------*/
char * buildPrompt()
{
	
    char* cwd =getcwd(wd,sizeof(wd));
	strcat(cwd,"$ ");	
    return cwd ;
}
// get the signal by the children
void signal_sigchild(int s)
{
    pid_t t;
    while ((t = waitpid(0,NULL,WNOHANG)) > 0)
    {
		int j;
		int idxRemove;
		for(int i = 0; i < numPid; i++)
		{
			if (pidArray[i] = t) {
			idxRemove = i;
			}
		}
		numPid--;
		for(j = idxRemove; j < numPid; j++)
		{
			pidArray[j] = pidArray[j + 1];
			charPidArray[j] = charPidArray[j + 1];
		}
	} // while
}	
// Help functions
void help()  
{      
	
	printf("built-in commands and their syntax: \n");      
	printf("[jobs] provides a numbered list of jobs currently executing in the background\n");      
	printf("[cd] changes the working directory\n");     
	printf("[history] prints out a history of old executed commands\n");     
	printf("[exit] terminates the shell process\n");      
	printf("[kill] [num] terminates the process numbered num in the list of background process returned by jobs\n");      
	printf("[help] you are here right now\n"); 
}
// Using recusrive pipes

// combines the arguements and command
char* function(parseInfo* info, int index) {
	struct commandType *com;
	com = &(info->CommArray[index]);
	char* strArray[11];
	int strLength = 0;
	char* string;
	for(int i = 0; i < com->VarNum; i++) 
	{
		strArray[i] = strdup(com->VarList[i]);
		strLength += strlen(strArray[i]);

	}
	string = malloc(strLength + com->VarNum * sizeof(char));
	for(int i = 0; i < com->VarNum; i++) {
		int strLength = strlen(strArray[i]);
		char* string_revised = malloc(strLength + 1 * sizeof(char));
		strcpy(string_revised," ");
		strcat(string_revised,strArray[i]);
		strcat(string,string_revised);
	}
	return string;
}


 void pipeExec( parseInfo* info, int index, int fdIn) {
    struct commandType *com;
    com = &(info->CommArray[index + 1]);

    if (com == NULL || com->command == NULL) {
        com = &(info->CommArray[index]);
        dup2(fdIn, STDIN_FILENO);
        int err;

        wordexp_t p;
        char* cmdLine = function(info,index);
        wordexp(cmdLine, &p, 0);
        err = execvp(com->command, com->VarList);
        if (err < 0) {
            fprintf( stderr, "command not found!\n" ) ;
            exit(1);
        } //if
    } else {
        com = &(info->CommArray[index]);
        int pipes[2];

        if (index < info->pipeNum) {
            pipe(pipes);
        }
        pid_t cPid;

        if((cPid = fork()) == 0) {
            dup2(pipes[1], STDOUT_FILENO);
            //dup2(pipes[1], STDIN_FILENO);
             close( pipes[0]);
             close (pipes[1]);
            int err;

            wordexp_t p;
            char* cmdLine = function(info, index);
            wordexp(cmdLine, &p, 0);
            err = execvp(p.we_wordv[0],p.we_wordv);
            if (err < 0) {
                exit(1);
            } //if

        } else {
            dup2( pipes[0] , STDIN_FILENO );
            close( pipes[0] );
            close(pipes[1]);
            pipeExec(info, index + 1, pipes[0]);
        }
	}
}
 



/* -----------------------------------------------------------------------------
   FUNCTION: isBuild()
   DESCRIPTION: The Built In commands
   -------------------------------------------------------------------------------*/
int isBuiltInCommand( char * cmd )
{
    if( strncmp(cmd, "exit", strlen( "exit" ) ) == 0 )
    {
	return EXIT;
    }
    else if (strncmp(cmd,"cd",strlen("cd")) == 0)
    {
	return CD;
    }
    else if (strncmp(cmd,"history",strlen("history")) == 0)
    {
	return HISTORY;
    }
    else  if (strncmp(cmd,"jobs",strlen("jobs")) == 0)
    {
	return JOBS;
    }
    else if( strncmp(cmd, "kill", strlen( "kill" ) ) == 0 )
    {
	return KILL;
    }
    else if (strncmp(cmd,"help",strlen("help")) == 0)
    {
	return HELP;
    }
    return NO_SUCH_BUILTIN;
}

// checks the Pid Array
int checkPidNULL()
{
    for(int i = 0; i < numPid; i++)
    {
	if (pidArray[i] != 0)
	{
	    return 1;
	}
    }
    return 0;
}

/* -----------------------------------------------------------------------------
   FUNCTION: main()
   DESCRIPTION:c
   -------------------------------------------------------------------------------*/
int main( int argc, char **argv )
{
    char * cmdLine;
    parseInfo *info; 		// info stores all the information returned by parser.
    struct commandType *com; 	// com stores command name and Arg list for one command.

    fprintf( stdout, "This is the SHELL version 0.1\n" ) ;


    while(1)
    {


	cmdLine = readline( buildPrompt() ) ;
	if( cmdLine == NULL )
	{
	    fprintf(stderr, "Unable to read command\n");

	    continue;
	}

    	// insert your code about history and !x !-x here
	using_history();
	
	history_expand(cmdLine,&expansion);
	add_history(expansion);
	if (history_length == 11) {
		remove_history(0);
	}
	strncpy(cmdLine,expansion, sizeof(cmdLine) - 1);
		// calls the parser
    	info = parse( cmdLine );
	if( info == NULL )
	{
	    free(cmdLine);
	    continue;
    	}

    	// prints the info struct
    	//print_info( info );

    	//com contains the info. of the command before the first "|"
    	com = &info->CommArray[0];
    	if((com == NULL)  || (com->command == NULL))
    	{
	    	free_info(info);
	    	free(cmdLine);
	   		continue;
    	}
		// built in commands
	if ( isBuiltInCommand( com->command ) == HISTORY )
	{

	    register HIST_ENTRY **the_list;
	    register int i;
	    time_t tt;
	    char timestr[128];
	    the_list = history_list ();
	    if (the_list)
		for (i = 0; the_list[i]; i++)
		{
		    tt = history_get_time (the_list[i]);
		    if (tt)
			strftime (timestr, sizeof (timestr), "%I %M", localtime(&tt));
		    else
			strcpy (timestr, "??");
		    printf ("%d: %s: %s \n", i + 1, timestr, the_list[i]->line);
			 
		}
	    
	} else if( isBuiltInCommand( com->command ) == EXIT )
	{
		if(checkPidNULL() == 0)
		{
		    exit(0);
		} else
		{
		    fprintf(stderr,"There are process in the background\n");
			
		}
	} else if (isBuiltInCommand(com->command) == CD)
	    {
			if (com->VarList[1] == NULL){
				chdir("/home/myid/hpp65933");
			} /*else 
			}*/ else
			{
				if (strstr(cmdLine,"~") != NULL) {
					wordexp_t p;
					wordexp(cmdLine,&p,0);
					if ((access(p.we_wordv[1],F_OK)) == -1) 
					{
		    			fprintf(stderr,"Cannot access or No such directory exist\n");
		    			fflush(stdout);
					} else {
						chdir(p.we_wordv[1]);
					}
				} else 
				{
		    		chdir(com->VarList[1]);
				}
			}

	    }  else if (isBuiltInCommand(com->command) == JOBS)
	    {
		
			if (checkPidNULL() ) {
				for(int i = 0; i < numPid; i++)
				{
					if (pidArray[i] != 0) {
						printf("%d PID - %d %s \n",i + 1 , pidArray[i], charPidArray[i]);
					}

				}
			}
	    }
	    else if (isBuiltInCommand(com->command) == KILL)
	    {
			int killpid;
			if(com->VarList[1][0] == '%') {
				char* cpy = strdup(com -> VarList[1]);
				cpy++;
				int idx = atoi(cpy);
				idx--;
				killpid = pidArray[idx];
				kill(killpid,SIGKILL);
				signal(SIGCHLD,signal_sigchild);
			}else
			{
				killpid = atoi(com->VarList[1]);
				kill(killpid,SIGKILL);
				signal(SIGCHLD,signal_sigchild);
			}
			printf("Killed %s \n", com->VarList[1]);
	} else if (isBuiltInCommand(com->command) == HELP)
	{
		help();
	} else 
	{

		// input, output, pipe,and background command
		int child;
		int status;
		int pipes[2];
		pipe(pipes);
		child = fork();

		if(child == 0)
		{
		    int fdIn;
		    int fdOut;
		    close(pipes[1]);
		    close(pipes[0]);
		    if (info->boolInfile)
		    {
				fdIn= open(info->inFile,O_RDONLY, 0);
				if (fdIn == -1)
				{
					
					if (errno == ENOENT)
						fprintf(stderr,"%s\n",strerror(errno));
					
					return -1;
				}
				dup2(fdIn,0);
				
		    } else if (info->boolOutfile)
			{
				fdOut = open(info->outFile, O_EXCL | O_WRONLY | O_CREAT,0644);
				if (fdOut == -1) {
					if (errno == EEXIST) {
						fprintf(stderr,"File Already Exist\n");
					}	
					return -1;

				}
				dup2(fdOut,1);
				
			}	
		    if (info->pipeNum > 0)
			{
				pipeExec(info,0,0);
		    } 
			else
			{
				int find = tildeFind(com->VarList,com->VarNum);
				
				if (find != -1 )
				{
					int error;
					wordexp_t p;
					wordexp(cmdLine,&p,0);
					
					execvp(p.we_wordv[0],p.we_wordv);
					if(error < 0) 
					{
						fprintf(stderr,"command not found\n");
					}
				} else 
				{	
					
					int error;
					error = execvp(com->command,com->VarList);
					if(error < 0) 
					{
						fprintf(stderr,"command not found\n");
					} 
				}
			}
		}
		else {
			close(pipes[0]);
			close(pipes[1]);
			
			if(info->boolBackground)
			{
			    pidArray[numPid] = child;
			    charPidArray[numPid] = cmdLine;
			    numPid++;
			}else
			{
			    waitpid(child,&status,0);
				signal(SIGCHLD,signal_sigchild);
			}
		    
		}
	}
	//insert your code here / commands etc.
	// check for pipe
	free_info(info);

	free(cmdLine);

    }/* while(1) */

}
// finds where the tildle is in the command
int tildeFind(char** VarList,int num ) {
	for (int i = 1; i < num; i++) 
	{
		if (strstr(VarList[i], "~") != NULL)
		{
			
			return i;
		}
	}
	return -1;

}
