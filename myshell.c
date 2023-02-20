/* Justin Gallagher, 25JAN2022
Project 2: The C Shell
Due Feb 16 by 1pm Points 10 Submitting a file upload Available Jan 25 at 6pm - Apr 27 at 11:59pm 3 months
CIS 3207, Section 004 //Spring 2022 //Instructor: Professor Gene Kwatny
Project 2: Creating a Linux Type Shell Program
Due Mar 16 by 1pm Points 8

Submitting a file upload File Types docx, odt, gdoc, txt, c, cpp, and zip Available after Feb 13 at 6pm

PROJECT REQUIREMENTS
 USER mode:  DONE
 BATCH mode: INCOMPLETE
 File redirection (<, >, >>)  DONE
 Built in Commands
 a. cd       DONE
 b. clr      DONE
 c. dir      DONE
 d. environ  DONE
 e. echo     DONE
 f. help     DONE
 g. pause    DONE
 h. quit     DONE
 i. path     DONE

[O] Program	Demo (1.2 pts)	1.2
[X] Makefile (0.6 pts)	0.6
[X] Reads and processes From Input File [Batch] (0.8 pts)	0.8
[X] Built-in Commands (0.8 pts)	0.8
[X] Background Execution (0.8 pts)	0.8
[X] I/O Redirection (1.2 pts)	1.2
[X] Pipes (1.0 pts)	1
[X] Report	Readme document  (0.8 pts)	0.8
[X] Implementation details and source comments  (0.8 pts)	0.8
[X] Testing, Discussion, Issues, and Results (0.8 pts)	0.8

 8.8/8.8 Updated 20MAR2022
 */

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <dirent.h>
#include "myshell.h"


#define MAX_LINE 1024 // The maximum length command
#define BUFSIZE 1000

int runInForeground = 1; // flag to determine if process should runShell in the background
int exitWhileFlag = 0;
char cwd[BUFSIZE];
char error_message[30] = "An error has occurred\n";

int main(int argc, char **argv, char **envp) {
    char *args[MAX_LINE]; // command line arguments


    f_clear();  // Clear console window
    splashScreen(); // Show welcome message
    f_getCWD(cwd, 0); // Get current working dir.
    setenv("SHELL", cwd, 1);

    /*Catch too many arguments.  This program will only allow console mode and batch mode!*/
    if (argc >= 3) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
    }

    /*Batch Mode - reads input from a batch file and executes commands found in the file.  If argc = 2, this will run
     * and simulate the return keystroke between lines.  This can be used for routine submissions*/
    if (argc == 2) {
        char *filename = argv[1];
        FILE *fp = fopen(filename, "r");
        char *line = NULL;
        size_t len = 0;
        ssize_t read;


        /*error for file not found*/
        if (fp == NULL) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }

        /*error for file not found*/
        while ((read = getline(&line, &len, fp)) != -1) {
            fflush(stdout);
            runInForeground = 1; // If sleep was triggered, this will reset back to console


            char *tokens;
            tokens = tokenize(line);

            char *arg = strtok(tokens, " ");
            int inArgCounter = 0;

            /*After the tokenized input has been received, this will look through the string for the conditional flags.
             * if present, this will redirect input and output based on the flags to satisfy the required control of
             * variables.  This includes redirections <, >, and >> along with pipes*/
            while (arg) {
                if (strcmp(arg, "<") == 0) {
                    redirectIn(strtok(NULL, " "));
                } else if (strcmp(arg, ">>") == 0) {
                    redirectAppend(strtok(NULL, " "));
                } else if (strcmp(arg, ">") == 0) {
                    redirectOut(strtok(NULL, " "));
                } else if (strcmp(arg, "|") == 0) {
                    args[inArgCounter] = NULL;
                    createPipe(args);
                    inArgCounter = 0;
                } else {
                    args[inArgCounter] = arg;
                    inArgCounter++;
                }
                arg = strtok(NULL, " ");
            }

            args[inArgCounter] = NULL;


            if (inArgCounter > 0) {
                runShell(args, inArgCounter);
            }
        }

        fclose(fp); // close the file
        exit(0);
    }


    /*User Mode - reads input from a console*/
    while (exitWhileFlag == 0) {

        runInForeground = 1; // If sleep was triggered, this will reset back to console
        printf("%s/myshell> ", cwd); //print user prompt
        fflush(stdout);



        char input[MAX_LINE];
        fgets(input, MAX_LINE, stdin);

        char *tokens;
        tokens = tokenize(input);

        char *arg = strtok(tokens, " ");
        int inArgCounter = 0;


        while (arg) {
            if (strcmp(arg, "<") == 0) {
                redirectIn(strtok(NULL, " "));
            } else if (strcmp(arg, ">>") == 0) {
                redirectAppend(strtok(NULL, " "));
            } else if (strcmp(arg, ">") == 0) {
                redirectOut(strtok(NULL, " "));
            } else if (strcmp(arg, "|") == 0) {
                args[inArgCounter] = NULL;
                createPipe(args);
                inArgCounter = 0;
            } else {
                args[inArgCounter] = arg;
                inArgCounter++;
            }
            arg = strtok(NULL, " ");
        }

        args[inArgCounter] = NULL;


        /*Main Shell*/
        if (inArgCounter > 0) {
            runShell(args, inArgCounter);
        }



        /*Check to kill off any zombie processes*/
        waitpid(-1, NULL, WNOHANG);


    }
    return 0;
}
