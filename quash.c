/**
 * @file quash.c
 *
 * Quash's main file
 */

/**************************************************************************
 * Included Files
 **************************************************************************/ 
#include "quash.h" // Putting this above the other includes allows us to ensure
                   // this file's headder's #include statements are self
                   // contained.

#include <string.h>


/**************************************************************************
 * Private Variables
 **************************************************************************/
/**
 * Keep track of whether Quash should request another command or not.
 */
// NOTE: "static" causes the "running" variable to only be declared in this
// compilation unit (this file and all files that include it). This is similar
// to private in other languages.
static bool running;

/**************************************************************************
 * Private Functions 
 **************************************************************************/
/**
 * Start the main loop by setting the running flag to true
 */
static void start() {
  running = true;
}

/**************************************************************************
 * Public Functions 
 **************************************************************************/


/*******************************************************
 * returns if the program is running or not
 *******************************************************/
bool is_running() {
  return running;
}
/*******************************************************
 * stopps the program
 *******************************************************/
void terminate() {
  running = false;
}
/*******************************************************
 * get and parse command from command line
 *******************************************************/
bool get_command(command_t* cmd, FILE* in) {
  fputs("(quash) ", stdout);
  if (fgets(cmd->cmdstr, MAX_COMMAND_LENGTH, in) != NULL) {
    size_t len = strlen(cmd->cmdstr);
    char last_char = cmd->cmdstr[len - 1];

    if (last_char == '\n' || last_char == '\r') {
      // Remove trailing new line character.
      cmd->cmdstr[len - 1] = '\0';
      cmd->cmdlen = len - 1;
    }
    else
      cmd->cmdlen = len;
    
    char tmpCmd[1024];
    memset(tmpCmd, 0, 1024);
    strcpy(tmpCmd, cmd->cmdstr);
    
	int i = 0;
	cmd->cmds[i] = strtok(tmpCmd,"|");
	while (cmd->cmds[i] != NULL) {
		if(cmd->cmds[i][0] == ' ') { // Removes leading space
			cmd->cmds[i]++;
		}
		i++;
		cmd->cmds[i] = strtok(NULL, "|");
	}
	cmd->cmdNum = i;

    //check for special io cases: <,>,&, or |
    cmd->background = (cmd->cmdstr[cmd->cmdlen-1] == '&') ? true : false;
    
    return true;
  }
  else
    return false;
}
/*******************************************************
 * run executable
 *******************************************************/
int run_executable(char* args[], int numArgs, int infile, int outfile) {

    int foundExecutable = 0;
    char* paths[100];
    char dirBuffer[1024];
    int i = 0;
    char pPath[1024];
    strcpy(pPath, getenv("PATH"));
    paths[i] = strtok(pPath," :");

    int status;
    pid_t pid_in, pid_out;
    pid_in = fork();
    if(pid_in == 0)
    {
        /*if(cmd->takesIn != 0)*/
        /*{*/
            /*//redirect standard input to the file using dup2*/
            /*if(dup2(infile,0) < 0)    */
                /*return;*/
        /*}*/
        /*if(cmd->sendsOut != 0)*/
        /*{*/
            /*//redirect standard output to the file using dup2*/
            /*if(dup2(outfile,1) < 0) */
                /*return;*/
        /*}*/
        
        /*if(access(args[0], F_OK) == 0)*/
        /*{*/
                foundExecutable = 1;
                if ( (execvp(args[0], args))) {
                    fprintf(stderr, "\n1Error execing find. ERROR#%d\n", errno);
                    return;
                }
        /*}*/
        /*else*/
        /*{*/
            /*while (paths[i] != NULL)*/
            /*{*/
                /*sprintf(dirBuffer, (paths[i][strlen(paths[i]) - 1] != '/') ? "%s/%s" : "%s%s", paths[i], args[0]);*/
                /*if(access(dirBuffer, F_OK))*/
                /*{*/
                    /*foundExecutable = 1;*/
                    /*if ( (execl(dirBuffer, dirBuffer, (char *) 0)) < 0) {*/
                        /*fprintf(stderr, "\n2Error execing find. ERROR#%d\n", errno);*/
                        /*return;*/
                    /*}*/
                /*}*/
                /*i++;*/
                /*paths[i] = strtok (NULL, " :");*/
            /*}*/
        /*}*/
        if(foundExecutable == 0)
        {
            puts("executable not found");
        }
        exit(0);
    }
    
    if ((waitpid(pid_in, &status, 0)) == -1) {
        fprintf(stderr, "Process 1 encountered an error. ERROR%d", errno);
    return;
  } 
}


/*******************************************************
 * Sets a given environment variable to a given value, both stored in cmd struct.
 *******************************************************/
void set_var(char * args[], int numArgs) 
{
	int ret = 0;
	if(numArgs != 2) {
		printf("Cannot Set Environment Variable\n");
	}
	else {
		char * str;
		char * str1;
		char * str2;

		// Removes equal sign
		str = strtok(args[1], "=");
		str1 = str;
		str = strtok (NULL, "=");
		str2 = str;

		ret = setenv(str1, str2, 1);
		if(ret == -1) {
			printf("Cannot Set Environment Variable\n");
		}
	}
}

/*******************************************************
 * This function prints the current jobs.
 *******************************************************/
void print_jobs()
{

}
/*******************************************************
 * echo environment variable
 *******************************************************/
void echo(char * args[], int numArgs)
{
	for (int i = 1; i < numArgs; i++) { // Loop to check through every argument
		if(strncmp(args[i], "$", 1) == 0) { // Check for environment variables
			char * var;
			char * temp;
			var = strtok(args[i], "$");
			temp = getenv(var);

			if(! temp) { // Environment variable not found
				printf(" ");
			}
			else {
				printf("%s ", temp);
			}
		}
		else { // Print string
			printf("%s ", args[i]);
		}
	}
	printf("\n");
}
/*******************************************************
 * change directory
 *******************************************************/
void cd(char * args[], int numArgs)
{
    char *newDir;
    int ret;
    
    if(numArgs == 1)
        newDir = getenv("HOME");
    else if(numArgs == 2) 
        newDir = args[1];
    ret = chdir (newDir);
    if(ret) {
    	printf("%s: No such file or directory\n",args[1]);
    }
}
/*******************************************************
 * print the current working directory
 *******************************************************/
void pwd()
{
    char* curDir = getcwd(NULL, 0);
    printf("%s\n",curDir);
}

void killBackground(command_t *cmd)
{
    
}

/*******************************************************
 * Quash entry point
 *
 * @param argc argument count from the command line
 * @param argv argument vector from the command line
 * @return program exit status
 *******************************************************/
int main(int argc, char** n) { 
  command_t cmd; //< Command holder argument
 
  start();

  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit\n");
  
  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {

    //If not receiving any command from user, skip iteration to prevent segmentation fault.
    if ((strlen(cmd.cmdstr) == 0)||(cmd.cmds[0] == NULL))
    	continue;
	else if (!strcmp(cmd.cmdstr, "exit") || !strcmp(cmd.cmdstr, "quit"))
		terminate(); // Exit Quash
	else {
		
		int status;
		pid_t pid_cmd;

		int pipefd[2];
		pipe(pipefd);

		int i;
		for(i = 0; i < cmd.cmdNum; i++) {

			// Stores command into buffer for parsing
			char tempbuff[1024];
			char * args[1024];
			int numArgs;
			memset(tempbuff, 0, 1024);
			strcpy(tempbuff, cmd.cmds[i]);

			int j = 0;
			args[j] = strtok(tempbuff," ");
			while (args[j] != NULL) {
				j++;
				args[j] = strtok(NULL, " ");
			}
			numArgs = j;

			FILE *infileptr = NULL;
			FILE *outfileptr = NULL;
			int infiledsc = -1;
			int outfiledsc = -1;
			
			int ind;
			bool takesIn = false;
			bool sendsOut = false;
			for(ind = 0; ind < numArgs; ind++) {
				if(strpbrk(args[ind],"<") != false) {
					takesIn = true;
				}
				if(strpbrk(args[ind],">") != false) {
					sendsOut = true;
				}
			}
			// File Redirecting
			/*
			if(takesIn) {
				int n;
				for(int n = 0; n < )
				char *tmp = strchr(cmd.cmdstr,'<')+1;
				if(strncmp(tmp," ",1)==0) // if space, move ahead by one.
					tmp++;
				strtok(tmp, " "); // find next space or end of string. and put \0
				infileptr = fopen(tmp,"r");
				if(infileptr != NULL)
				{
					infiledsc = fileno(infileptr);
				}
				else
				{
					perror ("The following error occurred");
					continue;
				}
				
			}
			if(sendsOut) != false) {
				[> JAKE NEEDS TO BE FIXED <]
				char *tmp = strchr(tmpCmd,'>')+1;
				if(strncmp(tmp," ",1)==0)//if space, move ahead by one.
					tmp++;
				strtok(tmp, " ");//find next space or end of string. and put \0

				outfileptr = fopen(tmp,"w+");
				outfiledsc = fileno(outfileptr);
			}
			*/

			if(!strcmp(args[0], "set"))
				set_var(args, numArgs);
			else if(!strcmp(args[0], "jobs"))
				print_jobs();
			else if(!strcmp(args[0], "echo"))
				echo(args, numArgs); //waiting for implementation
			else if(!strcmp(args[0], "cd"))
				cd(args, numArgs); //waiting for implementation
			else if(!strcmp(args[0], "pwd"))
				pwd(); //waiting for implementation
			else if(!strcmp(args[0], "kill"))
			{
				if (args[1] == NULL || args[2] == NULL){
					printf("Error. Invalid number of arguments.\n");
					continue;
				}
				else
					killBackground(&cmd);
			}
			else { // Run Command

				pid_cmd = fork();
				if(pid_cmd == 0) {
					/*
					if(i == 0) {
						close(pipefd[0]);
						dup2(pipefd[1], 1);
					}
					else {
						close(pipefd[1]);
						dup2(pipefd[0], 0);
					}
					*/
					run_executable(args, numArgs, infiledsc, outfiledsc);

					exit(0);
				}
				if((waitpid(pid_cmd, &status, 0)) == -1) {
					fprintf(stderr, "3Process encountered an error. ERROR%d", errno);
				}
			}

			if(infileptr != NULL)
				fclose(infileptr);
			if(outfileptr != NULL)
				fclose(outfileptr);
		}
	}
  }
  return EXIT_SUCCESS;
}

