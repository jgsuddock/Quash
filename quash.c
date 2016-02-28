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
    cmd->args[i] = strtok(cmd->cmdstr," ");
    while (cmd->args[i] != NULL)
    {
        i++;
        cmd->args[i] = strtok (NULL, " ");
    }
    
    return true;
  }
  else
    return false;
}

void run_executable(command_t cmd) {
    int foundExecutable = 0;
    char* paths[100];
    char dirBuffer[1024];
    int i = 0;
    paths[i] = strtok(getenv("PATH")," ");
    while (paths[i] != NULL)
    {
        i++;
        paths[i] = strtok (NULL, " ");
        sprintf(dirBuffer, "%s/%s", paths[i], cmd.args[0]);
        if(access(dirBuffer, "F_OK"))
        {
            foundExecutable = 1;
            if (fork() == 0){
                if ( (execl(dirBuffer, dirBuffer, (char *) 0)) < 0) {
                    fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
                    return EXIT_FAILURE;
                }
            }
        }
    }
    
    if(foundExecutable == 0)
    {
        puts("executable not found");
    }

    
    
    //This is a basic filler for now. Plan to have this function search all paths in PATH for executable.
//    if (fork() == 0){
//        if ( (execl(cmd.cmdstr, cmd.cmdstr, (char *) 0)) < 0) {
//            fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
//            return EXIT_FAILURE;
//        }
//    }
}

void set_var(command_t cmd) {
    printf("arg1: %s\n",cmd.args[1]);    //all printfs here are debug info.
    printf("arg2: %s\n",cmd.args[2]);    //shall be removed before turning in.
    printf("PATH: %s\n",getenv("PATH"));
    setenv(cmd.args[1], cmd.args[2]);
    printf("PATH: %s\n",getenv("PATH"));
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
    if (!strcmp(cmd.cmdstr, "exit"))
        terminate(); // Exit Quash
    else if(!strcmp(cmd.args[0], "set"))
        set_var(cmd);
 //   else if(!strncmp(cmd.cmdstr, "", 2))
 //       run_executable(cmd);
    else 
        run_executable(cmd);
     // puts(cmd.cmdstr); // Echo the input string
  }
  return EXIT_SUCCESS;
}















