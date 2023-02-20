//
// Created by justin on 3/8/22.
//

void redirectIn(char *fileName);
void redirectOut(char *fileName);
void redirectAppend(char *fileName);
void runShell(char *args[], int inArgCount);
void createPipe(char *args[]);
char *tokenize(char *input);
void splashScreen();
void f_help();
int f_exit();
void f_getCWD(char *, int);
void f_cd(char *);
void f_clear();
void f_environ();
void f_dir(char *args[]);
void f_path(char *args[], int inArgCount);
void f_echo(char *args[], int inArgCount);
void f_pause();