/**
 * @file quash.h
 *
 * Quash essential functions and structures.
 */

#ifndef QUASH_H
#define QUASH_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/**
 * Specify the maximum number of characters accepted by the command string
 */
#define MAX_COMMAND_LENGTH (1024)
#define MAX_BACKGROUND_TASKS (12)

/**
 * Holds information about a command.
 */
typedef struct command_t {
  char cmdstr[MAX_COMMAND_LENGTH]; ///< character buffer to store the
                                   ///< command string. You may want
                                   ///< to modify this to accept
                                   ///< arbitrarily long strings for
                                   ///< robustness.
  size_t cmdlen;                   ///< length of the cmdstr character buffer
  pid_t pid;
  bool background;
  int numArgs;
  //int argNum;
  //char* args[100];
  char* cmds[100];                 ///< user input string parsed into 
                                   ///< separate piped commands
  int cmdNum;                      ///< Number of commands within user 
                                   ///< input that are pipe delimited
  // Extend with more fields if needed
} command_t;

/*******************************************************
 * Query if quash should accept more input or not.
 *
 * @return True if Quash should accept more input and false otherwise
 *******************************************************/
bool is_running();

/*******************************************************
 * Causes the execution loop to end.
 *******************************************************/
void terminate();

/*******************************************************
 * runs executable from commandline.
 *******************************************************/
int run_executable(char* args[], int numArgs, int infile, int outfile);

/*******************************************************
 * Sets a given environment variable to a given value, both stored in cmd struct.
 *******************************************************/
void set_var(char * args[], int numArgs);
/*******************************************************
 * This function prints the current jobs.
 *******************************************************/
void print_jobs();
/*******************************************************
 * echo environment variable
 *******************************************************/
void echo(char * args[], int numArgs);
/*******************************************************
 * change directory
 *******************************************************/
void cd(char * args[], int numArgs);
/*******************************************************
 * print the current working directory
 *******************************************************/
void pwd();
/*******************************************************
 *  Read in a command and setup the #command_t struct. Also perform some minor
 *  modifications to the string to remove trailing newline characters.
 *
 *  @param cmd - a command_t structure. The #command_t.cmdstr and
 *               #command_t.cmdlen fields will be modified
 *  @param in - an open file ready for reading
 *  @return True if able to fill #command_t.cmdstr and false otherwise
 *******************************************************/
bool get_command(command_t* cmd, FILE* in);

#endif // QUASH_H
