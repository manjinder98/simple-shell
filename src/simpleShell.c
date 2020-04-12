#include "simpleShell.h"

bool checkAliasCmd = false;
int numArgs = 0;

/**
 * Init function for shell terminal starting point
 *
 */
void init() {
    String username = getenv("USER");
    printf("\nuser@%s", username);
    printf("\n");
    printf("$ ");
}

/**
 * Change working directory to HOME environment
 *
 */
void chwDir() {
    char cwd[ARG_MAX];
    printf("Old working dir: %s\n", getcwd(cwd, sizeof(cwd)));
    // Set the current working directory to home
    chdir(getenv("HOME"));
    printf("Current working dir: %s\n", getcwd(cwd, sizeof(cwd)));
}

/**
 * Read User input
 *
 * @param oldPath
 */
void readInput(String oldPath) {
    String buffer = malloc(sizeof(char) * ARG_MAX);
    char history[ARR_SIZE][ARG_MAX] = {0};
    int cmdNumber = 0;
    int numAliases = 0;

    printf("Previous history \n");
    previousHistory(&cmdNumber, history);
    printf("\n");

    printf("Previous alias \n");
    loadAlias(&numAliases);
    printf("\n");

    String copyBuffer;

    while (1) {
        init();
        String c = fgets(buffer, ARG_MAX, stdin);
        copyBuffer = strdup(buffer);

        // Remove leading spaces when exit is entered
        while (isspace(*buffer)) {
            ++buffer;
        }
        // Check for exit
        if (c == NULL || strcmp(buffer, "exit\n") == 0) {
            // Restore old Path
            setenv("PATH", oldPath, 1);
            getPath();
            // Set working directory to home
            chwDir();
            writeHistory(history, &cmdNumber);
            saveAlias(array , &numAliases);
            break;
        }

        fflush(stdin);

        if (buffer[0] != '\0') {
            // Check tokens before checking alias
            // for getting num arguments
            getTokens(buffer);
            bool copyAlias = checkAlias(&buffer);
            while (copyAlias != false) {
                copyAlias = checkAlias(&buffer);
            }
            checkInput(getTokens(buffer), buffer, history, &cmdNumber, true, &numAliases, copyBuffer, copyAlias);
        }
    }
}

/**
 * Check parsed input for built-in or external commands
 *
 * @param tokens
 * @param buffer
 * @param history
 * @param cmdNumber
 * @param delimiters
 */
void checkInput(String *tokens, char *buffer, char history[ARR_SIZE][ARG_MAX], int *cmdNumber, bool storeHis,
                int *numAliases, String copyBuffer, bool copyAlias) {
    // Check if need to copy the alias
    if (copyAlias == true) {
        buffer = strdup(copyBuffer);
    }
    if (buffer[0] != '!' && (storeHis == true)) {
        storeHistory(history, cmdNumber, buffer, tokens);
    }
    // Check if need to get the alias command
    if (checkAliasCmd == true) {
        checkAlias(&buffer);
        tokens = getTokens(buffer);
    }
    if (strncmp(tokens[0], "getpath", 7) == 0) {
        if (tokens[1] == NULL) {
            getPath();
        } else {
            printf(ERR_ARG_MAX);
        }
    } else if (strncmp(tokens[0], "setpath", 7) == 0) {
        // Check if there's a value for the path
        // and check if there are no other arguments
        if (tokens[1] && (tokens[2] == NULL)) {
            setPath(tokens[1]);
        } else if (tokens[1] == NULL) {
            printf("Empty value\n");
        } else {
            printf(ERR_ARG_MAX);
        }
    } else if ((strncmp(tokens[0], "cd", 2) == 0)) {
        // Check if there are arguments
        if (tokens[1] && tokens[2]) {
            printf(ERR_ARG_MAX);
        } else if (tokens[1] == NULL) {
            chwDir();
        } else {
            if (chdir(tokens[1]) < 0) {
                perror(tokens[1]);
            }
        }
    } else if (strcmp(tokens[0], "history") == 0) {
        if (tokens[1] == NULL) {
            getFullHistory(history);
        } else {
            printf("History command not valid!\n");
        }

    } else if (buffer[0] == '!') {
        extractHistory(tokens, history, cmdNumber, numAliases, copyBuffer, copyAlias);
    } else if (strncmp(tokens[0], "alias", 5) == 0) {
        addAlias(tokens, numAliases);
    } else if (strncmp(tokens[0], "unalias", 7) == 0) {
        unAlias(tokens, numAliases);
    } else {
        runCommand(tokens);
    }
}

/**
 * Run external commands
 *
 * @param ls_args
 */
void runCommand(char *ls_args[]) {
    pid_t c_pid;
    c_pid = fork();

    if (c_pid < 0) {
        perror("fork failed");
        exit(1);
    }
    if (c_pid == 0) {
        if (execvp(ls_args[0], ls_args) < 0) {
            perror(ls_args[0]);
            exit(1);
        }
        exit(0);
    } else {
        wait(NULL);
    }
}

/**
 * Return the PATH environment
 *
 */
void getPath() {
    printf("%s\n", getenv("PATH"));
}

/**
 * Get String tokens with specified delimiters
 *
 * @param cmd
 * @param delimiters
 * @return
 */
String *getTokens(String cmd) {
    static String tokensList[] = {NULL};
    // Copy variable in temp variable
    String tempStr = calloc(strlen(cmd) + 1, sizeof(char));
    strcpy(tempStr, cmd);
    // Divide in tokens
    String token = strtok(tempStr, DELIMITERS);
    int i = 0;
    // walk through other tokens
    while (token != NULL) {
        tokensList[i] = token;
        token = strtok(NULL, DELIMITERS);
        i++;
        numArgs = i;
    }
    numArgs = i;
    tokensList[i] = NULL;
    return tokensList;
}

/**
 * Set the environment PATH
 *
 * @param newPath
 */
void setPath(String newPath) {
    if (checkDirectory(newPath)) {
        setenv("PATH", newPath, 1);
    } else {
        perror(newPath);
    }
}

/**
 * Store history with full command String
 *
 * @param history
 * @param cmdNum
 * @param cmd
 * @param tokens
 */
void storeHistory(char history[ARR_SIZE][ARG_MAX], int *cmdNum, String cmd, String *tokens) {
    if (strcmp(tokens[0], "history") == 0) {
        // Check if call is history without any rubbish arguments
        if (*cmdNum == 0 && tokens[1] == NULL) {
            printf(ERR_EMPTY_HIS);
            return;
        }
        // Check if last command was history
        int tempCmdNum = *cmdNum;
        if (tempCmdNum > 20) {
            tempCmdNum = 20;
        }
        --tempCmdNum;
        if (strcmp(trimWhiteSpace(history[tempCmdNum]), "history") == 0) {
            return;
        }
    }
    if (*cmdNum >= ARR_SIZE) {
        // Shift elements to the left by one
        // to add the new one
        for (int i = 0; i < ARR_SIZE - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        strcpy(history[ARR_SIZE - 1], cmd);
    } else {
        strcpy(history[*cmdNum], cmd);
    }
    *cmdNum = *cmdNum + 1;
}

String trimWhiteSpace(String str) {
    String tempStr = malloc(sizeof(char) * ARG_MAX);
    strcpy(tempStr, str);
    String end;

    // Trim leading space
    while (isspace((unsigned char) *tempStr)) tempStr++;

    if (*tempStr == 0)  // All spaces?
        return tempStr;

    // Trim trailing space
    end = tempStr + strlen(tempStr) - 1;
    while (end > tempStr && isspace((unsigned char) *end)) end--;

    // Write new null terminator character
    end[1] = '\0';

    return tempStr;
}

/**
 * Print full history of commands
 *
 * @param history
 * @param size
 */
void getFullHistory(char history[ARR_SIZE][ARG_MAX])
{
    printf("This is the current history\n");
    for (int i = 0; i < ARR_SIZE; i++) {
        if (strcmp(history[i], "") > 0) {
            printf("%d. %s", i + 1, history[i]);
       }
    }

}

/**
* Run history command selected through index
*
* @param history
* @param index
* @param DELIMITERS
*/
void getHistory(char history[ARR_SIZE][ARG_MAX], int index, String args, int *cmdNumber, int *numAliases,
                String copyBuffer, bool copyAlias) {
    int lessIndex = index;
    if (lessIndex > 20) {
        lessIndex = 20;
    }
    // Get command from history
    String cmd = history[--lessIndex];
    bool storeHis = false;

    // Copy Alias internal command
    if (copyAlias == true) {
        cmd = strdup(copyBuffer);
        storeHis = true;
        copyAlias = false;
    } else {
        // Bool check for alias in checkInput
        copyBuffer = strdup(cmd);
        checkAliasCmd = true;
        copyAlias = true;
    }

    if (args && strlen(args) != 0) {
        String tempCmd = malloc(sizeof(char) * ARG_MAX);;
        strcpy(tempCmd, cmd);
        // Remove new line from string
        tempCmd[strlen(tempCmd) - 1] = '\0';
        strcat(tempCmd, " ");
        strcat(tempCmd, args);
        strcat(tempCmd, "\n");
        storeHis = true;
        checkInput(getTokens(tempCmd), tempCmd, history, cmdNumber, storeHis, numAliases, copyBuffer, copyAlias);
    } else {
        checkInput(getTokens(cmd), cmd, history, &index, storeHis, numAliases, copyBuffer, copyAlias);
    }
}

/**
 * Check if string passed is numeric
 *
 * @param s
 * @return
 */
int isNumeric(const char *s) {
    if (s == NULL || *s == '\0' || isspace(*s))
        return 0;
    char *p;
    strtod(s, &p);
    return *p == '\0';
}

/**
 * Extract history by looking at which command to use
 *
 * @param tokens
 * @param history
 * @param cmdNumber
 * @param delimiters
 */
void extractHistory(String *tokens, char history[ARR_SIZE][ARG_MAX], int *cmdNumber, int *numAliases,
                    String copyBuffer, bool copyAlias) {
    if (strcmp(tokens[0], "!!") == 0) {
        if (tokens[1]) {
            printf(ERR_ARG_MAX);
            return;
        }
        if (*cmdNumber > 0) {
            getHistory(history, *cmdNumber, NULL, NULL, numAliases, copyBuffer, copyAlias);
        } else {
            printf(ERR_EMPTY_HIS);
        }
    } else {
        String charIndex = NULL;
        String args = malloc(sizeof(char) * ARG_MAX);
        int i = 1;
        // Add extra parameters if any
        while (tokens[i] != NULL) {
            strcat(args, tokens[i]);
            i++;
        }
        // Check if remainder command or normal
        // by checking if command starts with !-
        if (strncmp(tokens[0], "!-", strlen("!-")) == 0 && isNumeric(&tokens[0][2])) {
            // Take command number for remainder
            charIndex = &tokens[0][2];
            getIndexHistory(charIndex, history, cmdNumber, true, args, numAliases, copyBuffer, copyAlias);
        } else if (isNumeric(&tokens[0][1])) {
            // Take command number
            charIndex = &tokens[0][1];
            getIndexHistory(charIndex, history, cmdNumber, false, args, numAliases, copyBuffer, copyAlias);
        } else {
            printf("Enter a valid number! \n");
        }
    }
}

/**
 * Get the index of specific history command
 *
 * @param charIndex
 * @param history
 * @param cmdNumber
 * @param delimiters
 * @param isRemainder
 */
void getIndexHistory(String charIndex, char history[ARR_SIZE][ARG_MAX], int *cmdNumber, bool isRemainder, String arg,
                     int *numAliases, String copyBuffer, bool copyAlias) {
    char *ptr;
    // Convert number from char to int
    int index = (int) strtol(charIndex, &ptr, 10);
    if (isRemainder == true) {
        if (*cmdNumber > 20) {
            // Reset commands number to 20 (MAXIMUM)
            *cmdNumber = 20;
        }
        // Get remainder
        // Shift index to the left
        // because of starting with index 1
        index--;
        index = *cmdNumber - index;
    }

    // Check if index is in stored history
    if (index > ARR_SIZE) {
        printf("Only 20 history commands allowed!\n");
    } else if (index <= 0 || index > *cmdNumber) {
        printf("History command not found!\n");
    } else {
        getHistory(history, index, arg, cmdNumber, numAliases, copyBuffer, copyAlias);
    }

}

/**
 * Write history into file
 *
 * @param history
 * @param size
 *
 */
void writeHistory(char history[ARR_SIZE][ARG_MAX], const int *size) {
    FILE *fp;
    //file pointer to open file
    fp = fopen(".hist_list", "w+");
    //writing each line of history
    for (int i = 0; i < ARR_SIZE && i < *size; i++) {
        fprintf(fp, "%s", history[i]);
    }
    fclose(fp);
}

/**
 * Load previous history from file
 *
 * @param cmdNum
 * @param history
 */
void previousHistory(int *cmdNum, char history[ARR_SIZE][ARG_MAX]) {
    static const char filename[] = ".hist_list";
    FILE *fp;
    fp = fopen(filename, "r");

    int index = 0;
    if (fp != NULL) {
        char line[512];

        while (fgets(line, sizeof line, fp) != NULL) /* read a line */
        {
            fputs(line, stdout); /* write the line */
            strcpy(history[index], line);
            index++;
        }

        *cmdNum = index;

        fclose(fp);
    } else {
        printf("No previous history \n");
    }
}


/**
 * Check if directory exists
 *
 * @param s:path
 * @return
 */
int checkDirectory(String s) {
    DIR *dir = opendir(s);
    if (dir) {
        // Directory or file exists
        closedir(dir);
        return 1;
    } else if (ENOENT == errno) {
        // Directory or file does not exist
        return 0;
    } else {
        // opendir() failed for some other reason
        return -1;
    }
}

void addAlias(String *token, int *NumberOfAlias) {
    //If there is more than one command
    char wholeLineCommand[512] = {'\0'};
    int t = 2;

    if (token[1] == NULL) {
        if (*NumberOfAlias == 0) {
            printf("There are no current alias\n");
        } else {
            printf("Current Aliases:\n");
            for (int i = 0; i <= MAX_ALIAS; i++)
            {
                if (array[i].aliasCommand[0] != '\0')
                {
                    printf("%s %s\n",array[i].aliasName, array[i].aliasCommand);
                }
            }
        }
    } else if (token[2] == NULL) {
        printf("Too Few Arguments\n");
    } else {
        while (token[t] != NULL) {
            strcat(wholeLineCommand, token[t]);
            strcat(wholeLineCommand, " ");
            t++;
        }

        //Null Terminating the whitespace
        int len = strlen(wholeLineCommand);
        wholeLineCommand[len - 1] = '\0';

        //Look for duplicate aliases
        for (int i = 1; i <= 11; i++) {
            if (strcmp(array[i].aliasName, token[1]) == 0) {
                for (int a = 1 ; a <= 11; a++) {
                    if ((strcmp(array[a].aliasName, token[2]) == 0) && (*NumberOfAlias > 0)) {
                        for (int d = 1; d <=11; d++) {
                            if ((strcmp(array[d].aliasCommand, token[1]) == 0)) {
                                printf("Circular Alias \n");
                                return;
                            }
                        }
                    }
                }
                printf("Overwriting alias %s\n", token[1]);
                strcpy(array[i].aliasName, token[1]);
                strcpy(array[i].aliasCommand, wholeLineCommand);
                return;
            }
            //Finding empty position
            if (strcmp(array[i].aliasName, "") == 0) {
                if (*NumberOfAlias >= MAX_ALIAS) {
                    printf("Alias list full\n");
                    return;
                }
                for (int a = 1 ; a <= 11; a++) {
                    if ((strcmp(array[a].aliasName, token[2]) == 0) && (*NumberOfAlias > 0)) {
                        for (int d = 1; d <=11; d++) {
                            if ((strcmp(array[d].aliasCommand, token[1]) == 0)) {
                                printf("Circular Alias \n");
                                return;
                            }
                        }
                    }
                }
                    strcpy(array[i].aliasName, token[1]);
                    strcpy(array[i].aliasCommand, wholeLineCommand);
                    *NumberOfAlias = *NumberOfAlias + 1;
                    printf("New Alias %s -- %s\n", token[1], wholeLineCommand);
                    return;
            }
        }

    }

}

void unAlias(String *token, int *NumberOfAliases) {
    int set = 0;
    if (token[1] == NULL) {
        printf("Not enough arguments\n");
        return;
    }
    if (token[2] != NULL) {
        printf(ERR_ARG_MAX);
        return;
    }
    if (*NumberOfAliases <= 0) {
        printf("Alias list is empty!\n");
        return;
    }


    for (int i = 0; i <= MAX_ALIAS; i++) {
        if (strcmp(array[i].aliasName, token[1]) == 0) {
            strcpy(array[i].aliasName, "");
            strcpy(array[i].aliasCommand, "");
            strcpy(array[i].aliasName, array[i + 1].aliasName);
            strcpy(array[i].aliasCommand, array[i + 1].aliasCommand);
            strcpy(array[i + 1].aliasName, "");
            strcpy(array[i + 1].aliasCommand, "");
            *NumberOfAliases = *NumberOfAliases - 1;
            printf("Alias Removed %s\n", token[1]);
            return;
        } else {
            set = 1;
        }


    }
    if (set == 1) {
        printf("There are no such aliases in the list\n");
    }


}

bool checkAlias(String *input) {
    char *token;
    char line[512] = {'\0'};
    bool status = false;
    //get command
    token = strtok(*input, DELIMITERS);
    //look for an alias and get the alias command if one is found
    for (int j = 0; j <= MAX_ALIAS; j++) {
        if (token != NULL && array[j].aliasName != NULL && (strcmp(token, array[j].aliasName) == 0)) {
            token = array[j].aliasCommand;
            status = true;
        }
    }
    //Building the command
    if (token != NULL) {
        strcpy(line, token);
    }
    //get rest of original line after the possible alias
    token = strtok(NULL, "");
    if (token != NULL) {
        strcat(line, " ");
        strcat(line, token);
    }
    // Add new line at the end of the string
    // if the number of arguments is greater than 1
    if (numArgs < 2) {
        strcat(line, "\n");
    }
    strcpy(*input, line);

    if (strstr(*input, "!") != NULL) {
        status = false;
    }

    return status;
}

void saveAlias(aliases *input, const int *numAliases)
{
    FILE *fp;

    //file pointer to open file
    fp = fopen(".aliases", "w+");

    //writing each line of alias
    for (int i = 1; (i <= *numAliases) && (i <= MAX_ALIAS); i++) {
        if ((strcmp(input[i].aliasName, "")) != 0 && (strcmp(input[i].aliasCommand, "") != 0))
        {
            fprintf(fp, "%s %s \n", input[i].aliasName, input[i].aliasCommand);
        }
    }
    fclose(fp);
}

void loadAlias(int *numberOfAliases)
{
    static const char filename[] = ".aliases";
    FILE *fp;
    fp = fopen(filename, "r");

    int index = 1;

    if (fp != NULL) {
        char line[ARG_MAX];

        while (fgets(line, sizeof line, fp) != NULL) /* read a line */
        {
            String commandForAlias = malloc(sizeof(char) * ARG_MAX);

            fputs(line, stdout); /* write the line */

            char **tokensList = getTokens(line);
            int indexForCommand = 1;

            strcpy(array[index].aliasName, tokensList[0]);

            // Used for storing parameters
            while(tokensList[indexForCommand] != NULL)
            {
                strcat(commandForAlias, tokensList[indexForCommand]);
                strcat(commandForAlias, " ");
                indexForCommand++;
            }
            strcpy(array[index].aliasCommand, commandForAlias);

            index++;
        }

        *numberOfAliases = index;

        fclose(fp);
    } else {
        printf("No previous aliases \n");
    }
}
