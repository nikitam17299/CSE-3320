/*Student Name: Nikita Ashok Menon 
University ID: 1001548454
NET ID: nam8454
CSE 3320 - OPERATING SYSTEMS 
PROFFESOR - TREVOR BAKKER
*/

#define _GNU_SOURCE

//Required header files 

#include<stdbool.h>
#include <stdio.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

//declaration and initialization of global variables

int PID[15];
int p_count = 0, count_c = 0;
 int count_l;

char *command_history[255];
 
bool return_command(int i, char *command) 
{ 	
		
	if(i >= count_c||i < 0) 
	{ 
		return false;
	} 
	
	count_l++;
 
	strcpy(command, strcat(command_history[i],"\0"));

	if(count_l>50) 
{ 
	printf("MORE THAN 50 NOT INVALID.\n");
	command= '\0';
	return false;
} 

return true; 

} 
 
int main()

{

	char *cmd_str = (char*) malloc(255);
	bool h_command = false;
	
	while(1)
{
	if(!h_command)
	{

		count_l=0;
	
	//Requirement 1: Prints msh whenever console is ready to accept input
	printf("msh> ");
//Gets command entered by user	
	while(!fgets(cmd_str,255, stdin));
	
	}


	char* cmd_in = strdup(cmd_str);
	cmd_in[strcspn(cmd_in,"\r\n")]=0;

	char *token[10];
	char *command = token[0];
int token_count =0 ;
char *arg_ptr;
char *working_str = strdup(cmd_str);
char *working_root = working_str;
//Requirment 5 - quits running the shell if the user says quit and exit
		if(strcmp(command,"quit")==0||strcmp(command,"exit") == 0 ) 
	{
		//removes all memory
		free(working_root);
		break;
	} 
//taking command line arguments 
//	int token_count = 0;
	
//	char *arg_ptr;

//	char *working_str = strdup(cmd_str);

//	char* working_root = working_str;

	while(((arg_ptr = strsep(&working_str," \t\n"))!=NULL)&&(token_count<10))
	{
		token[token_count] = strndup(arg_ptr,255);
		
		if (strlen(token[token_count])==0)
		{
			token[token_count]=NULL;
		} 

		token_count++;
	}

	if(token[0]==NULL)
	{	
		continue;

	}
//Requirement 8 : suspending a process with bg
if(strcmp(command,"bg")==0)
{
	
	char *cwd = NULL;
	cwd = getcwd(NULL,0);

	if(token[1]!=NULL)
	{ 
	//Allocating memory to new command 
	char  *newCwd = (char*) malloc(strlen(cwd) + strlen(token[1]) +2);

	strcpy(newCwd, cwd);
	newCwd[strlen(newCwd)] = '/';
	newCwd[strlen(newCwd)] = '\0';
	
	strcat(newCwd,token[1]);

	
	errno =0;
	
//freeing memory assigned 
free(newCwd);
} 

else 
{ 
	printf("%s\n",cwd);
} 

	continue;
}

//Requirement 13 For shell to support thr cd command 
if(strcmp(command,"cd")==0)
{
	chdir(token[1]);
	continue;
}

if(strcmp(command,"listpids")==0)
{
	
	int i;
	for(i=0;i< p_count; i++)
	{ 	
		printf("%d:%d\n", i, PID[i]);
	} 

	continue;
}

//Requirement 15 displaying history 
if(!h_command) 
{
	//add_command(cmd_in);
	{
	count_c++;
	
	if(count_c == 51) 
	{ 	
		int i;
		for(i=0;i<50;i++)
		{
			command_history[i] = command_history[i+1];
		}

	count_c--;
}


		command_history[count_c -1] = cmd_in;
}

}

h_command = false;

	if(strcmp(command,"history")==0)
	{
		{
int i;

	for( i=0;i< count_c;i++)
	{
		printf("%d : %s \n", i, command_history[i]);
	} 

} 
		continue;
	} 
	

//To repeat command based on the number entered
if(cmd_in[0] == '!')
	{
		bool check = false;
		check = return_command(atoi(cmd_in+1), cmd_str);

		if(!check)
		{
			printf("COMMAND NOT IN HISTORY.\n");

			h_command= false;
			continue;
		}

		else
		{
			
			if(!h_command)
			{
				{
	count_c++;
	
	if(count_c == 51) 
	{ 	
		int i;
		for(i=0;i<50;i++)
		{
			command_history[i] = command_history[i+1];
		}

	count_c--;
}


		command_history[count_c -1] = cmd_in;
}

			}
		
		h_command = true;
		continue;
		}
	}	

pid_t pid = fork();
	
if(pid == -1) 
{
	printf("ERROR\n");
}	
	else if(pid==0)
	{
		
		char* buffer = NULL;
		buffer = getcwd(NULL,0);


		
	char* cwd_command = (char*)malloc(strlen (buffer) + strlen(command) +2 ) ;

	strcat(cwd_command,buffer);
	char * forward_slash = "/";
	strcat(cwd_command,forward_slash);
	strcat(cwd_command,command);

	free(buffer);

	errno = 0;

	execv(cwd_command,token);

	

 	if(errno ==2)
	{
	 char *usr_local_bin_s = "/usr/local/bin/";
	 char *usr_local_command = (char*) malloc(strlen(usr_local_bin_s) + strlen(command) + 1) ;
	 strcat(usr_local_command,usr_local_bin_s);
	 strcat(usr_local_command, command);

	

	errno = 0;
 	execv(usr_local_command, token);
	
	
	if(errno ==2) 
	{ 
		char *usr_bin_s = "/usr/bin/";
		char *usr_bin_command = (char*) malloc(strlen(usr_bin_s) + strlen(command) +1) ;
		strcat(usr_bin_command,usr_bin_s);
		strcat(usr_bin_command,command);
	 

	errno =0; 

	execv(usr_bin_command,token);


if(errno ==2) 
{ 
		char *bin_s = "/bin/";
		char *bin_command = (char*) malloc(strlen(bin_s) + strlen(command)+1);
		strcat(bin_command,bin_s);
		strcat(bin_command,command);
 
	errno =0;
	execv(bin_command, token);


	if(errno==2)
	{

		printf("%s : COMMAND NOT FOUND\n", command);
	}
	
     }

   }
}

	exit(EXIT_SUCCESS);
} 

else
{
	int c_status;
	


	{ 


		{

	p_count++;
	
	if(p_count == 16)

	{
		int i;
		for(i=0;i< 15;i++)
		{
		PID[i] = PID[i+1];
		} 


		p_count--;
	} 

	
	PID[p_count - 1] = pid;

} 

		} 

	(void)waitpid(pid,&c_status, 0|WUNTRACED) ;

} 
	free(working_root);

}

return 0;

} 































