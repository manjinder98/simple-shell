/* Header File */
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#define ARG_MAX 512
#define ARR_SIZE 20
#define MAX_ALIAS 10
#define ERR_ARG_MAX "Too many arguments!\n"
#define DELIMITERS " \t\n;&><|"
#define ERR_EMPTY_HIS "History is empty!\n"

struct alias {
    char aliasName[ARG_MAX];
    char aliasCommand[ARG_MAX];
};
typedef struct alias aliases;
typedef char *String;
typedef enum {
    true, false
} bool;


aliases array[ARG_MAX];


void init();
void readInput(String currPath);
int checkDirectory(String s);
void getPath();
String *getTokens(String cmd);
void setPath(String newPath);
void runCommand(String tokens[]);
String trimWhiteSpace(String str);
void chwDir();
void storeHistory(char history[ARR_SIZE][ARG_MAX], int *cmdNum, String cmd, String *tokens);
void getFullHistory(char history[ARR_SIZE][ARG_MAX]);
void
getHistory(char history[ARR_SIZE][ARG_MAX], int index, String args, int *cmdNumber, int *numAliases, String copyBuffer,
           bool copyAlias);
void extractHistory(String *tokens, char history[ARR_SIZE][ARG_MAX], int *cmdNumber, int *numAliases, String copyBuffer,
                    bool copyAlias);
void getIndexHistory(String charIndex, char history[ARR_SIZE][ARG_MAX], int *cmdNumber, bool isRemainder, String arg,
                     int *numAliases, String copyBuffer, bool copyAlias);
void checkInput(String *tokens, char *buffer, char history[ARR_SIZE][ARG_MAX], int *cmdNumber, bool storeHis,
                int *numAliases, String copyBuffer, bool copyAlias);
void previousHistory(int *cmdNum, char history[ARR_SIZE][ARG_MAX]);
void writeHistory(char history[ARR_SIZE][ARG_MAX], const int *size);
void unAlias(String *token, int *NumberOfAliases);
void addAlias(String *token, int *NumberOfAlias);
bool checkAlias(String *input);
void saveAlias(aliases *input, const int *numAliases);
void loadAlias(int *numberOfAliases);