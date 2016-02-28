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
bool is_running() {
  return running;
}

void terminate() {
  running = false;
}

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
    
    return true;
  }
  else
    return false;
}

void run_executable(command_t cmd) {
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


/**
 * Sets a given environment variable to a given value, both stored in cmd struct.
 */
void set_var(command_t cmd) {
    //char pathBuffer[1024];
    //sprintf(pathBuffer, (strlen(cmd.args[2]) != 0) ? "%s:%s" : "%s%s", getenv("PATH"), cmd.args[2]);
    setenv(cmd.args[1], cmd.args[2]);
}

/**
 * This function prints the current jobs.
 */
void print_jobs(){
  for (int i = 0; i < MAX_BACKGROUND_TASKS; n++){
    if(backprocess[i].pid != 0){
      printf("[%u] %u %s\n", n+1, backprocess[i].pid, backprocess[i].cmdstr);
    }
  }
  return;
}


/**
 * Quash entry point
 *
 * @param argc argument count from the command line
 * @param argv argument vector from the command line
 * @return program exit status
 */
int main(int argc, char** n) { 
  command_t cmd; //< Command holder argument
  
  start();
  
  puts("Welcome to Quash!");
  puts("Type \"exit\" to quit");
  
  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {
    // The commands should be parsed, then executed.
    
    //check for special io cases: <,>,or |
    bool takesIn = (strpbrk(cmd.cmdstr,"<")!=NULL);
    bool sendsOut = (strpbrk(cmd.cmdstr,">")!=NULL);
    bool piping = (strpbrk(cmd.cmdstr,"|")!=NULL);
    
    if(cmd.cmdstr[cmd.cmdlen-1] == '&')//is a background process
    {
      cmd.cmdstr[cmd.cmdlen-1] = '\0';
      if(cmd.cmdstr[cmd.cmdlen-2] == ' ')
      {
        cmd.cmdstr[cmd.cmdlen-2] = '\0';
      }
      cmd.background = true;

    }
    else
    {
      cmd.background = false;
    }
    
    
    if (!strcmp(cmd.cmdstr, "exit") || !strcmp(cmd.cmdstr, "quit"))
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
    else 
        run_executable(cmd);
  }
  return EXIT_SUCCESS;
}















