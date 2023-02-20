//
// Created by justin on 3/8/22.
//

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

int runInForeground;
char error_message[30];
int exitWhileFlag;
char cwd[BUFSIZE];
extern char **environ;


/*Main shell*/
void runShell(char *args[], int inArgCount) {


    /*The shell will first recognize args[0] and call the appropriate built-in below.  If the built-in is not hard coded
     * (ad not required by this assignment, the shell will pass the arg to execvp to execute out of /bin.  A detailed
     * list can be found in the readme*/
    pid_t pid;
    if (strcmp(args[0], "exit") == 0) {
        f_exit();
    } else if (strcmp(args[0], "cd") == 0) {
        char *path = args[1];
        f_cd(path);
    } else if (strcmp(args[0], "clear") == 0) {
        f_clear();
    } else if (strcmp(args[0], "environ") == 0) {
        f_environ();
    } else if (strcmp(args[0], "dir") == 0) {
        f_dir(args);
    } else if (strcmp(args[0], "path") == 0) {
        f_path(args, inArgCount);
    } else if (strcmp(args[0], "echo") == 0) {
        f_echo(args, inArgCount);
    } else if (strcmp(args[0], "pause") == 0) {
        f_pause();
    } else if (strcmp(args[0], "help") == 0) {
        f_help();
    } else {

        /*Find any & (if any)  This is needed to determine if the function should execute in background mode, or
         * in foreground mode.  There are two functions below.  Background entry in atleast one argument detected,
         * or background mode not detected at all.  It is possible to have background and not background tasks execute
         * together in one line*/
        for (int i = 0; i <= inArgCount - 1; ++i) { // LAST ENTRY WILL ALWAYS BE NULL
            if (strcmp(args[i], "&") == 0) {
                runInForeground = 0;

            } else {

            }
        }

        /*If background & flag is not detected, the commands will execute in foreground mode only.  This will fork() a
         * child process so that execvp() can execute and the user will be return to the shell.  This is mandatory
         * due to the way that execvp will exit upon completion*/
        if (runInForeground == 1) {

            pid = fork();
            if (pid < 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
            } else if (pid == 0) { /* child process */
                execvp(*args, args);

                /* The below will only run if execvp fails*/
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            } else { /* parent process */
                if (runInForeground == 1) {
                    waitpid(pid, NULL, 0);
                } else {
                    runInForeground = 1;
                }
            }
            redirectIn("/dev/tty"); // redirect back to console for user
            redirectOut("/dev/tty"); // redirect back to console for user

        /*If background & flag is  detected, multiple steps need to be taken to ensure that there is no data loss and
         * the correct vector is being processed.
         * Step 1: This will run a for loop that will catch the first & up to the last entry in the argv.  If if loop
         * reaches the last entry it will remove the '&' and replace with null and send the argv to execvp()
         * Step 2: For this loop we have a vector increment.  This will determine where the & is found in a multi line
         * string and do the following
         *  -loop i=0->argc
         *  -set '&' to NULL
         *  -move the vector from *arg to *args + vectorIncrement
         *  -Fork() and send vector to execvp()
         *  -RESET:vectorIncrement to 0
         *  -vectorIncrement = 0 + i
         *  LOOP RETURN: This way if another & is found before reaching the end, the current vector will be 0 + i + 1
         *  WHY + 1?  The argv needs to be sent AFTER the null value!*/
        } else {

            int vectorIncrement = 0;
            for (int i = 0; i < inArgCount; ++i) {

                if (strcmp(args[i], "&") == 0) {

                    args[i] = '\0';
                    args = args + vectorIncrement; // 0 on first run around

                    pid = fork();
                    if (pid < 0) {
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    } else if (pid == 0) { /* child process */
                        execvp(*args, args);

                        /* The below will only run if execvp fails*/
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        exit(1);
                    } else { /* parent process */
                        if (runInForeground == 1) {
                            waitpid(pid, NULL, 0);
                        } else {
                            runInForeground = 1;
                        }
                    }
                    args = args - vectorIncrement;
                    vectorIncrement = 0;
                    runInForeground = 0;
                    vectorIncrement = vectorIncrement + i + 1; // new vector increment will be i
                    redirectIn("/dev/tty"); // redirect back to console for user
                    redirectOut("/dev/tty"); // redirect back to console for user
                } else if (i == inArgCount - 1) {

                    args = args + vectorIncrement; // 0 on first run around
                    pid = fork();
                    if (pid < 0) {
                        write(STDERR_FILENO, error_message, strlen(error_message));
                    } else if (pid == 0) { /* child process */
                        execvp(*args, args);

                        /* The below will only run if execvp fails*/
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        exit(1);
                    } else { /* parent process */
                        if (runInForeground == 1) {
                            waitpid(pid, NULL, 0);

                        } else {
                            runInForeground = 1;

                        }
                    }

                }
            }
        }

// LEAVE IN FOR TESTING PURPOSES.  SEE J.GALLAGHER FOR MORE INFO.
//        pid = fork();
//        if (pid < 0) {
//            write(STDERR_FILENO, error_message, strlen(error_message));
//        } else if (pid == 0) { /* child process */
//            execvp(*args, args);
//
//            /* The below will only run if execvp fails*/
//            write(STDERR_FILENO, error_message, strlen(error_message));
//            exit(1);
//        } else { /* parent process */
//            if (runInForeground == 1) {
//                waitpid(pid, NULL, 0);
//
//            } else {
//                runInForeground = 1;
//
//            }
//        }
//        redirectIn("/dev/tty"); // redirect back to console for user
//        redirectOut("/dev/tty"); // redirect back to console for user
    }
}

/*
Redirects stdin from a file.
*/
void redirectIn(char *fileName) {
    int in = open(fileName, O_RDONLY);
    dup2(in, 0);
    close(in);
}

/*
Redirects stdout to a file.
*/
void redirectOut(char *fileName) {
    int out = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0600);
    dup2(out, 1);
    close(out);
}

/*
Redirects stdout to append a file.
*/
void redirectAppend(char *fileName) {
    int out = open(fileName, O_WRONLY | O_APPEND | O_CREAT, 0600);
    dup2(out, 1);
    close(out);
}


/*
Creates a pipe.
*/
void createPipe(char *args[]) {
    int fd[2];
    pipe(fd);

    dup2(fd[1], 1);
    close(fd[1]);

    runShell(args, 0);

    dup2(fd[0], 0);
    close(fd[0]);
}

/*
Creates a tokenized form of the input with spaces to separate words.
*/
char *tokenize(char *input) {
    int i;
    int j = 0;
    char *tokenized = (char *) malloc((MAX_LINE * 2) * sizeof(char));

    // add spaces around special characters
    for (i = 0; i < strlen(input); i++) {
        if (input[i] != '<' && input[i] != '|') {
//        if (input[i] != '>' && input[i] != '<' && input[i] != '|') {
            tokenized[j++] = input[i];
        } else {
            tokenized[j++] = ' ';
            tokenized[j++] = input[i];
            tokenized[j++] = ' ';
        }
    }
    tokenized[j++] = '\0';

    // add null to the end
    char *end;
    end = tokenized + strlen(tokenized) - 1;
    end--;
    *(end + 1) = '\0';

    return tokenized;
}

/*
Function will exit.
*/
int f_exit() {
    exitWhileFlag = 1;
    return 0; // return 0 to parent process in runShell.c
}

/*
Function will call f_getCWD and pass inArgVal[1] to be copied into the new path/cwd.
*/
void f_cd(char *path) {
    int ret = chdir(path);
    if (ret == 0) // path could be changed if cd successful
    {
        f_getCWD(cwd, 0);
    } else write(STDERR_FILENO, error_message, strlen(error_message));
}

/*
pwd function
*/
void f_getCWD(char *cwdstr, int command) {
    char temp[BUFSIZE];
    char *path = getcwd(temp, sizeof(temp));
    if (path != NULL) {
        strcpy(cwdstr, temp);
        if (command == 1)  // check if pwd is to be printed
        {
            printf("%s\n", cwdstr);
        }
    } else write(STDERR_FILENO, error_message, strlen(error_message));

}

/*
clear the screen.  Does regex magic
*/
void f_clear() {
    const char *blank = "\e[1;1H\e[2J";
    write(STDOUT_FILENO, blank, 12);


}

/*
calls environ from shell and will print loop on each line.  Standard implement for environ
*/
void f_environ() {
    char **s = environ;

    for (; *s; s++) {
        printf("%s\n", *s);
    }

}

/*
print contents of dir specified.  If none is specifies, this will print the contents of the current dir "."
*/
void f_dir(char *args[]) {

    if (args[1] != NULL) {
        DIR *d;
        struct dirent *dir;
        d = opendir(args[1]);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                printf("%s\n", dir->d_name);
            }
            closedir(d);
        }
    } else {
        DIR *d;
        struct dirent *dir;
        d = opendir(".");
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                printf("%s\n", dir->d_name);
            }
            closedir(d);
        }
    }

}

/*
Path Stuff
*/
void f_path(char *args[], int inArgCount) {

    char path[PATH_MAX] = "";

    if (inArgCount <= 1) {
        setenv("PATH", path, 1);
    } else {
        setenv("PATH", args[1], 1);
    }

}

/*
Simple echo function to repeat all arguments following a successful echo function call.
*/
void f_echo(char *args[], int inArgCount) {
    int i;
    if (inArgCount <= 1) {
        printf("Nothing to echo!");
    } else {
        for (i = 1; i < inArgCount; ++i) {
            printf("%s ", args[i]);
        }

    }
    printf("\n");
}

/*
Pause input until Enter is received from user.
*/
void f_pause() {

    printf("Press [Enter] to continue . . .");
    fflush(stdout);
    getchar();
}

/*
display help text
*/
void f_help() {
    char *args[] = {"more", "help.txt", NULL};

    pid_t pid = fork();
    if (pid == -1) {
        write(STDERR_FILENO, error_message, strlen(error_message));
    } else if (pid == 0) {
        if (execvp(args[0], args) < 0) {
            // Anything below should not be run on success of execvp
        }
    } else {
        int status;
        wait(&status);
    }
}

/*
mimic splashScreen like logo functionality from ubuntu
*/
void splashScreen() {
    char *welcomestr = "\n\n"
                       " ╔╗ ╔═╦╗╔╗  ╔╦═╗╔══╦╗\n"
                       " ║╠╦╣═╣╚╬╬═╦╣║═╣║══╣╚╦═╦╗╔╗\n"
                       "╔╣║║╠═║╔╣║║║╠╬═║╠══║║║╩╣╚╣╚╗\n"
                       "╚═╩═╩═╩═╩╩╩═╝╚═╝╚══╩╩╩═╩═╩═╝\n";
    printf("%s", welcomestr);
}