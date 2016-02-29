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
command_t backprocess[MAX_BACKGROUND_TASKS];

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
  fputs("(quash)", stdout);
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
    
    int i = 0;
    cmd->args[i] = strtok(cmd->cmdstr," =");
    while (cmd->args[i] != NULL)
    {
        i++;
        cmd->args[i] = strtok (NULL, " =");
    }
    cmd->argNum = i;
    
    //check for special io cases: <,>,&, or |
    cmd.takesIn = (strpbrk(cmd.cmdstr,"<")!=NULL);
    cmd.sendsOut = (strpbrk(cmd.cmdstr,">")!=NULL);
    cmd.usesPiping = (strpbrk(cmd.cmdstr,"|")!=NULL);
    cmd.background = (cmd.cmdstr[cmd.cmdlen-1] == '&') ? true : false;
    
    return true;
  }
  else
    return false;
}
/*******************************************************
 * run executable
 *******************************************************/
void run_executable(command_t cmd, int infile, int outfile) {
    
    if(cmd.takesIn)
    {
        //redirect standard input to the file using dup2
        if(dup2(infile,0) < 0)    
        {
        	return 1;
        }
    }
    if(cmd.sendsOut)
    {
        //redirect standard output to the file using dup2
        if(dup2(outfile,1) < 0) 
        {
        	return 1;
        }
    }
    
    printf("arg1: %s\n",cmd.args[0]);    //all printfs here are debug info.
    printf("arg2: %s\n",cmd.args[1]);    //shall be removed before turning in.
    printf("PATH: %s\n",getenv("PATH"));
    int foundExecutable = 0;
    char* paths[100];
    char dirBuffer[1024];
    int i = 0;
    paths[i] = strtok(getenv("PATH")," :");
    if(access(cmd.args[0], "F_OK") == 0)
    {
        printf("succeeded1");
            foundExecutable = 1;
            if (fork() == 0){
                if ( (execl(cmd.args[0], cmd.args[0], (char *) 0)) < 0) {
                    fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
                    return EXIT_FAILURE;
                }
            }
    }
    else{
    while (paths[i] != NULL)
    {
        printf("path[i]: %s\n",paths[i]);
        sprintf(dirBuffer, (paths[i][strlen(paths[i]) - 1] != '/') ? "%s/%s" : "%s%s", paths[i], cmd.args[0]);
        printf("dirBuffer: %s\n",dirBuffer);
        if(!access(dirBuffer, "F_OK"))
        {
            printf("succeeded2");
            foundExecutable = 1;
            if (fork() == 0){
                if ( (execl(dirBuffer, dirBuffer, (char *) 0)) < 0) {
                    fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
                    return EXIT_FAILURE;
                }
            }
        }
        i++;
        paths[i] = strtok (NULL, " :");
    }
    }
    if(foundExecutable == 0)
    {
        puts("executable not found");
    }
}


/*******************************************************
 * Sets a given environment variable to a given value, both stored in cmd struct.
 *******************************************************/
void set_var(command_t cmd) {
    //char pathBuffer[1024];
    //sprintf(pathBuffer, (strlen(cmd.args[2]) != 0) ? "%s:%s" : "%s%s", getenv("PATH"), cmd.args[2]);
    setenv(cmd.args[1], cmd.args[2]);
}

/*******************************************************
 * This function prints the current jobs.
 *******************************************************/
void print_jobs(){
  for (int i = 0; i < MAX_BACKGROUND_TASKS; n++){
    if(backprocess[i].pid != 0){
      printf("[%u] %u %s\n", n+1, backprocess[i].pid, backprocess[i].cmdstr);
    }
  }
  return;
}
/*******************************************************
 * echo environment variable
 *******************************************************/
void echo(cmd)
{
}
/*******************************************************
 * change directory
 *******************************************************/
void cd(cmd)
{
}
/*******************************************************
 * print the current working directory
 *******************************************************/
void pwd(cmd)
{
}
void killBack(cmd)
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

  memset(backprocess,0,sizeof(command_t)*MAX_BACKGROUND_TASKS);

  struct sigaction sa;
  sigset_t mask_set;  /* used to set a signal masking set. */
  /* setup mask_set */
  sigemptyset(&mask_set);
  sigfillset(&mask_set);//prevent other interupts from interupting intrupts
  sa.sa_mask = mask_set;
  sa.sa_handler = catch_sigchld;
  sigaction(SIGCHLD,&sa,NULL);
  
  start();
  
  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit");
  
  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {
    //Copy the command string to a temporary variable.
    char tmpCmd[1024];
    memset(tmpCmd, 0, 1024);
    strcpy(tmpCmd, cmd.cmdstr);

    
    //If not receiving any command from user, skip iteration to prevent segmentation fault.
    if ((strlen(cmd.cmdstr) == 0)||(cmd.args[0] == NULL))
      continue;
    else if (!strcmp(cmd.cmdstr, "exit") || !strcmp(cmd.cmdstr, "quit"))
        terminate(); // Exit Quash
    else if(!strcmp(cmd.args[0], "set"))
        set_var(cmd);
    else if(!strcmp(cmd.args[0], "jobs"))
        print_jobs(cmd);
    else if(!strcmp(cmd.args[0], "echo"))
        echo(cmd); //waiting for implementation
    else if(!strcmp(cmd.args[0], "cd"))
        cd(cmd); //waiting for implementation
    else if(!strcmp(cmd.args[0], "pwd"))
        pwd(cmd); //waiting for implementation
    else if(!strcmp(cmd.args[0], "kill")
    {
        if (cmd.args[1] == NULL || cmd.args[2] == NULL){
            printf("Error. Invalid number of arguments.\n");
            continue;
        }
        else
            killBack(cmd);
        }
    else 
    {
        memset(tmpCmd, 0, 1024);
        strcpy(tmpCmd, cmd.cmdstr);
        FILE *infileptr = NULL;
        FILE *outfileptr = NULL;
        int infiledsc = -1;
        int outfiledsc = -1;
        if(cmd.takesIn){
            char *tmp = strchr(tmpCmd,'<')+1;
            if(strncmp(tmp," ",1)==0)//if space, move ahead by one.
                tmp++;
            strtok(tmp, " ");//find next space or end of string. and put \0
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
        strcpy(tmpCmd, cmd.cmdstr);
        if(cmd.sendsOut)
        {
            char *tmp = strchr(tmpCmd,'>')+1;
            if(strncmp(tmp," ",1)==0)//if space, move ahead by one.
                tmp++;
            strtok(tmp, " ");//find next space or end of string. and put \0

            outfileptr = fopen(tmp,"w+");
            outfiledsc = fileno(outfileptr);
        }
        strtok(cmd.cmdstr, "<>");//add \0 to begining of <> segment to designate end of string.
        cmd.cmdlen = strlen(cmd.cmdstr);
        
        run_executable(cmd,infiledsc,outfiledsc);
        
        if(infileptr != NULL)
            fclose(infileptr);
        if(outfileptr != NULL)
            fclose(outfileptr);
    }
  }
  return EXIT_SUCCESS;
}




/**
 * @file quash.c
 *
 * Quash's main file
 *

/**************************************************************************
 * Included Files
 ************************************************************************** 
#include "quash.h" // Putting this above the other includes allows us to ensure
                   // this file's header's #include statements are self
                   // contained.

#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/**************************************************************************
 * Private Variables
 **************************************************************************
/**
 * Keep track of whether Quash should request another command or not.
 *
// NOTE: "static" causes the "running" variable to only be declared in this
// compilation unit (this file and all files that include it). This is similar
// to private in other languages.
static bool running;
char pPath[1024], hHome[1024];
command_t backprocess[MAX_BACKGROUND_TASKS];

/**************************************************************************
 * Private Functions 
 **************************************************************************
/**
 * Start the main loop by setting the running flag to true
 *
static void start() {
  memset(pPath,0,1024);
  memset(hHome,0,1024);
  strcpy(pPath, getenv("PATH"));
  strcpy(hHome, getenv("HOME"));
	
	
  running = true;
}

/**************************************************************************
 * Public Functions 
 **************************************************************************
/**
 * This function checks if the program is running or not.
 * @return boolean value of running
 *
bool is_running() {
  return running;
}

/**
 * This function terminates the program.
 * @return none
 *
void terminate() {
  running = false;
  printf("Bye!\n");
}

/**
 * This function allows the user to set the environment variable.
 * @param envVar environment string to be set
 * @return none
 *
void set(char *envVar){
  if ((envVar == NULL) || (*envVar == ' ')){
    puts("Error. Empty variable.");
    return;
  }

  char *token;

  /* get the first token *
  token = strtok(envVar, "=");

  if (strcmp(token, "PATH") == 0){
    strcpy(pPath, strtok(NULL, ""));
  }
  else if (strcmp(token, "HOME") == 0){
    strcpy(hHome, strtok(NULL, ""));  
  }
  else{
    puts("Error. No such variable.");
  }

  return;
}

/**
 * This function prints the string passed to it.
 * @param string string to be printed out.
 * @return none
 *
void echo(char *string){
  //If empty string, just print a new line.
	if(string == NULL){
    printf("\n");
  }
  else if (strcmp(string, "$HOME") == 0){
		puts(hHome);
	}	
	else if (strcmp(string, "$PATH") == 0){
		puts(pPath);
	}	
  else{
    puts(string);
  }
	
	return;
}

/**
 * This function allows the user to change directory.
 * @param dir string to cd into.
 * @return none
 *
void cd(char *dir){
	int ret = 0;
	
	if (dir == NULL || dir[0] == '~'){
		ret = chdir(hHome);	
	}
	else{
		ret = chdir(dir);
	}
	
	if (ret == -1){
		puts("Error, no such directory.");	
	}
	
	return;
}

/**
 * This function prints the current working directory.
 * @return none
 *
void pwd(){
	char *currentDir = getcwd(NULL, 0);
	puts(currentDir);
	free(currentDir);
	
	return;
}

/**
 * This function prints the current jobs.
 * @return none
 *
void jobs(){
  for (int n = 0; n < MAX_BACKGROUND_TASKS; n++){
    if(backprocess[n].pid != 0){
      printf("[%u] %u %s\n", n+1, backprocess[n].pid, backprocess[n].cmdstr);
    }
  }
  return;
}

/**
 * This function sends a signal to a process based on its Job ID
 * @param sigNum integer value of the signal to be sent
 * @param jobID integer value of the Job ID
 * @return none
 *
void killBack(int sigNum, int jobID){
  if (sigNum == 0 || jobID == 0){
    puts("Invalid command.");
    return;
  }
  else{
    pid_t pidKill = 0;
    for (int n = 0; n < MAX_BACKGROUND_TASKS; n++){
      if((jobID-1 == n) && (backprocess[n].pid != 0)){
        pidKill = backprocess[n].pid;
        kill(pidKill, sigNum);
        return;
      }
    }
    printf("No Job ID: [%u] to kill.\n", jobID);
    return;
  }
}

/**
 * This function catches the signal and notifies the parent when a child process exits.
 * @param sig_num integer value of signal to catch
 * @return none
 *
void catch_sigchld(int sig_num){
  int status;
  for (int n = 0; n < MAX_BACKGROUND_TASKS; n++){
    if(backprocess[n].pid != 0){
      pid_t ret = waitpid(backprocess[n].pid, &status, WNOHANG);
      if (ret == 0){//child is sill alive dont do anything
      }else if(ret == -1 && errno != 10){//child had an error or child does not exist any more.
        fprintf(stderr, "[%u] %u %s encountered an error. ERROR %d\n",n+1,backprocess[n].pid,backprocess[n].cmdstr,errno);
        backprocess[n].pid = 0;
        break;
      }else{//child exited
        fprintf(stderr, "[%u] %u finished %s\n",n+1,backprocess[n].pid,backprocess[n].cmdstr);
        backprocess[n].pid = 0;
        break;
      }
    }
  }
}

/**
 * This function parses the input and generates the command to be executed.
 * @param cmd command struct parsed
 * @param fdread integer value of the read file descriptor
 * @param fdwrite integer value of the write file descriptor
 * @return none
 *
void genCmdSimple(command_t* cmd,int fdread,int fdwrite){
  int status;
  pid_t pid_1;

  pid_1 = fork();

  if (pid_1 == 0) { //if child

      if(fdread != -1){
        dup2(fdread,STDIN_FILENO);
      }
      if(fdwrite != -1){
        dup2(fdwrite,STDOUT_FILENO);
      }

    int pathNum;// number in the path
    char ncmd[1024];
    memset(ncmd,0,1024);
    
    if (!strncmp(cmd->cmdstr, "/",1)){
      strcpy(ncmd,hHome);
      strcat(ncmd,strtok(cmd->cmdstr," "));
      if(execl(ncmd,ncmd,strtok(NULL," "),strtok(NULL," "),strtok(NULL," "),strtok(NULL," ")) <0){
        fprintf(stderr, "\nError. ERROR # %d\n", errno);

      }

    }else{
      strtok(cmd->cmdstr," ");
      char* arg[4] = {strtok(NULL," "),strtok(NULL," "),strtok(NULL," "),strtok(NULL," ")};
      strcpy(ncmd,strtok(pPath,":"));//copy first part of path to newcmd
      pathNum = 1;
      
      bool executed = false;
      while((strncmp(ncmd,"\0",1) != 0) && (executed == false)){ //while the path is not null, and not executed
        strcat(ncmd,"/");//add "/" between path and command.
        strcat(ncmd,cmd->cmdstr); //cat on the cmd str

        if (execl(ncmd,ncmd,arg[0],arg[1],arg[2],arg[3]) < 0){//try the next one //if failed:
          char *tmp = NULL;
          for(int i = 0; i<pathNum; i++){
            tmp = strtok(NULL,":");//go to the next part of the path.
          }
          pathNum++;

          if (tmp != NULL){//if not found a null termenator?
            strcpy(ncmd,tmp);//copy new str to ncmd
          }else{
            ncmd[0] = '\0';//else set ncmd first char to null
          }         
        } else{
          executed = true;
        }
      }
      if (executed == false){
        puts("Failed to find command.");

      }
    }
  if(fdread != -1){//close read and write, if applicable.
      close(fdread);
    }
    if(fdwrite != -1){
      close(fdwrite);
    }
  exit(0);

  }else{//if child's parent
    cmd->pid = pid_1;
    close(fdwrite);
    if ((waitpid(pid_1, &status, 0)) == -1) {
      if(errno != 10){
        fprintf(stderr, "Process %s encountered an error. ERROR %d\n", cmd->cmdstr,errno);
      }
    } 
  }
}


/**
 * This function parses the input and generates the command to be executed.
 * @param cmd command struct parsed
 * @param fdread integer value of the read file descriptor
 * @param fdwrite integer value of the write file descriptor
 * @return none
 *
void genCmd(command_t* cmd,int fdread,int fdwrite){
  int status;
  pid_t pid_1;
  int fdtopid1[2];//writing end is 1, reading end is 0 
  bool pipecmd = false;
  char* firstcmd;
  char* secondcmd;
  char tmpCmd[1024];
  memset(tmpCmd, 0, 1024);
  strcpy(tmpCmd, cmd->cmdstr);

  firstcmd = strtok(tmpCmd, "|");
  if(strcmp(firstcmd,cmd->cmdstr) != 0){//there is a second part.
    pipecmd = true;
    secondcmd = strtok(NULL, "");//get the rest.
    if(strncmp((strchr(firstcmd,'\0')-1)," ",1)){
      firstcmd[strlen(firstcmd)] = '\0';//get rid of the space
    }
    if(!strncmp(secondcmd," ",1)){
      secondcmd++;//move ahead the second command
    }
    
  }

  pid_1 = fork();

  if (pid_1 == 0) { //if child

    if(pipecmd == false){//if pipeing
      if(fdread != -1){
        dup2(fdread,STDIN_FILENO);
      }
      if(fdwrite != -1){
        dup2(fdwrite,STDOUT_FILENO);
      }

    }else{
      pipe(fdtopid1);
      command_t fstcmd;//first command first.
      strcpy(fstcmd.cmdstr,firstcmd);
      fstcmd.cmdlen = strlen(firstcmd);
      fstcmd.pid = 0;
      fstcmd.background = false;
      dup2(fdtopid1[0],STDIN_FILENO);//setup return pipe to second process,

      if(fdwrite != -1){
      dup2(fdwrite,STDOUT_FILENO);
      }
      strcpy(cmd->cmdstr,secondcmd);

      genCmdSimple(&fstcmd,fdread,fdtopid1[1]);
    }

    int pathNum;// number in the path
    char ncmd[1024];
    memset(ncmd,0,1024);
    
    if (!strncmp(cmd->cmdstr, "/",1)){
      strcpy(ncmd,hHome);
      strcat(ncmd,strtok(cmd->cmdstr," "));
      if(execl(ncmd,ncmd,strtok(NULL," "),strtok(NULL," "),strtok(NULL," "),strtok(NULL," ")) <0){
        fprintf(stderr, "\nError. ERROR # %d\n", errno);

      }

    }else{
      strtok(cmd->cmdstr," ");
      char* arg[4] = {strtok(NULL," "),strtok(NULL," "),strtok(NULL," "),strtok(NULL," ")};
      strcpy(ncmd,strtok(pPath,":"));//copy first part of path to newcmd
      pathNum = 1;
      
      bool executed = false;
      while((strncmp(ncmd,"\0",1) != 0) && (executed == false)){ //while the path is not null, and not executed
        strcat(ncmd,"/");//add "/" between path and command.
        strcat(ncmd,cmd->cmdstr); //cat on the cmd str

        if (execl(ncmd,ncmd,arg[0],arg[1],arg[2],arg[3]) < 0){//try the next one //if failed:
          char *tmp = NULL;
          for(int i = 0; i<pathNum; i++){
            tmp = strtok(NULL,":");//go to the next part of the path.
          }
          pathNum++;

          if (tmp != NULL){//if not found a null termenator?
            strcpy(ncmd,tmp);//copy new str to ncmd
          }else{
            ncmd[0] = '\0';//else set ncmd first char to null
          }         
        } else{
          executed = true;
        }
      }
      if (executed == false){
        puts("Failed to find command.");

      }
    }
  if(fdread != -1){
      close(fdread);
    }
    if(fdwrite != -1){
      close(fdwrite);
    }

  exit(0);

  }else{// else parent

    cmd->pid = pid_1;

    if(cmd->background == false){

      if ((waitpid(pid_1, &status, 0)) == -1) {
        if(errno != 10){
          fprintf(stderr, "Process %s encountered an error. ERROR %d\n", cmd->cmdstr,errno);
        }
      } 

    }else{//background task
      for (int n = 0; n < MAX_BACKGROUND_TASKS; n++){
        if(backprocess[n].pid == 0){
          memcpy(&backprocess[n],cmd,sizeof(command_t));
          printf("[%u] %u\n",n+1,cmd->pid);
          break;
        }
      }     
    }
  }
}

bool get_command(command_t* cmd, FILE* in) { //checks for an input from in, and returns the command.
  memset(cmd->cmdstr,0,1024);
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
    
    return true;
  }
  else{
    if((errno != 0) && (errno != EINTR)){
    perror ("The following error occurred");
    }
    if(feof(in)){
    return false;
    }
    return true;
  }
}

/**
 * Quash entry point
 *
 * @param argc argument count from the command line
 * @param argv argument vector from the command line
 * @return program exit status
 *
int main(int argc, char** argv) { 
  command_t cmd; //< Command holder argument
  
  char tmpCmd[1024];
  memset(tmpCmd, 0, 1024);

  memset(backprocess,0,sizeof(command_t)*MAX_BACKGROUND_TASKS);

  struct sigaction sa;
  sigset_t mask_set;  /* used to set a signal masking set. *
  /* setup mask_set *
  sigemptyset(&mask_set);
  sigfillset(&mask_set);//prevent other interupts from interupting intrupts
  sa.sa_mask = mask_set;
  sa.sa_handler = catch_sigchld;
  sigaction(SIGCHLD,&sa,NULL);

  start();
  
  puts("Welcome to Quash!");
  printf("$PATH is: %s\n", pPath);
  printf("$HOME is: %s\n", hHome);
  puts("Type \"exit\" to quit");

  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {
    //Copy the command string to a temporary variable.
    memset(tmpCmd, 0, 1024);
    strcpy(tmpCmd, cmd.cmdstr);

    //Split tmpCmd to get the command 
    char *tok;
    tok = strtok(tmpCmd, " ");
    
    //If not receiving any command from user, skip iteration to prevent segmentation fault.
    if ((strlen(tmpCmd) == 0)||(tok == NULL)){
      continue;
    }
    if(cmd.cmdstr[cmd.cmdlen-1] == '&'){//is a background process
      cmd.cmdstr[cmd.cmdlen-1] = '\0';
      if(cmd.cmdstr[cmd.cmdlen-2] == ' '){
        cmd.cmdstr[cmd.cmdlen-2] = '\0';
      }
      cmd.background = true;

    }else{
      cmd.background = false;
    }

    // The commands should be parsed, then executed.
    if ((!strcmp(tmpCmd, "exit")) ||(!strcmp(tmpCmd, "quit")))//if the command is "exit"
    {
      terminate(); // Exit Quash
    }
    else if(strcmp(tok, "echo") == 0){
      echo(strtok(NULL, ""));
    }
    else if(strcmp(tok, "cd") == 0){
      cd(strtok(NULL, ""));
    }
    else if(strcmp(tok, "pwd") == 0){
      pwd();
    }
    else if(strcmp(tok, "set") == 0){
      set(strtok(NULL, ""));
    }
    else if(strcmp(tok, "jobs") == 0){
      jobs();
    }
    else if(strcmp(tok, "kill") == 0){
      char *sigNum = strtok(NULL, " ");
      char *jobID = strtok(NULL, " ");
      
      if (sigNum == NULL || jobID == NULL){
          printf("Error. Invalid number of arguments.\n");
          continue;
      }
      else{
          int intSigNum = atoi(sigNum);
          int intjobID = atoi(jobID);
   
          killBack(intSigNum, intjobID);
      }
    }
    else 
    {
      memset(tmpCmd, 0, 1024);
      strcpy(tmpCmd, cmd.cmdstr);
      FILE *infileptr = NULL;
      FILE *outfileptr = NULL;
      int infiledsc = -1;
      int outfiledsc = -1;
      if(strchr(tmpCmd,'<') != NULL){
        char *tmp = strchr(tmpCmd,'<')+1;
        if(strncmp(tmp," ",1)==0)//if space, move ahead by one.
          tmp++;
        strtok(tmp, " ");//find next space or end of string. and put \0
        infileptr = fopen(tmp,"r");
        if(infileptr != NULL){
          infiledsc = fileno(infileptr);
        }else{
          perror ("The following error occurred");
          continue;
        }
      }
      strcpy(tmpCmd, cmd.cmdstr);
      if(strchr(tmpCmd,'>') != NULL){
        char *tmp = strchr(tmpCmd,'>')+1;
        if(strncmp(tmp," ",1)==0)//if space, move ahead by one.
          tmp++;
        strtok(tmp, " ");//find next space or end of string. and put \0

        outfileptr = fopen(tmp,"w+");
        outfiledsc = fileno(outfileptr);
      }
      strtok(cmd.cmdstr, "<>");//add \0 to begining of <> segment to designate end of string.
      cmd.cmdlen = strlen(cmd.cmdstr);
      
      genCmd(&cmd,infiledsc,outfiledsc);
      
      if(infileptr != NULL)
        fclose(infileptr);
      if(outfileptr != NULL)
        fclose(outfileptr);
    }
  }
  return EXIT_SUCCESS;
}

*/








