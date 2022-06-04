//Ben Levi 318811304

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


// History array will contain Commands.
struct Command {
    pid_t runnerPid;
    char* strCommand;
};

const char SEP[1] = " ";
bool SHOULD_STOP = false;
struct Command* history[100];
int indexCommands = 0;
char tempLine[101];
bool FLAG_COMMAND = false;


void prompt() {
    printf("$ ");
    fflush(stdout);
}

// The function fix the '\n' suffix after takeLine.
void fixSuffix(char* str) {
    while (*str == ' ') {
        str++;
    }
    if (*str != '\n') {
        // The user enter command.
        FLAG_COMMAND = true;
        while ((str != NULL) && (*str != '\n')) {
            str++;
        }
        if (str != NULL) {
            *str = '\0';
        }
    }
}

void takeLine(char* line) {
    fgets(line, 100, stdin);
    fixSuffix(line);
}

void addToHistory(char* line, pid_t pid) {
    history[indexCommands] = (struct Command*)malloc(sizeof(struct Command));
    history[indexCommands]->strCommand = line;
    history[indexCommands]->runnerPid = pid;
    indexCommands++;
}

// Built in command - history.
void showHistory() {
    int i;
    for(i = 0; i < indexCommands; i++) {
        printf("%d %s\n", history[i]->runnerPid, history[i]->strCommand);
    }
}

// Built in command - exit.
// The function free dynamic allocations and set SHOULD_STOP.
void exitFunc() {
    int i;
    for(i = 0; i < indexCommands; i++) {
        free(history[i]->strCommand);
        free(history[i]);
    }
    SHOULD_STOP = true;
}

// Built in command - cd.
void cdCommand(char* line) {
    while(*line == ' ') {
        line++;
    }
    strcpy(tempLine, line);
    // Start from the space char.
    char* sepIndex = strchr(tempLine, ' ');
    // If space char after 'cd' exist.
    if(sepIndex != NULL) {
        char* startPath = sepIndex;
        bool flag = false;
        // Take path after spaces.
        while (*startPath == ' ') {
            startPath++;
            if((startPath != NULL)&&(*startPath!='\0')&&(*startPath!=' '))  {
                flag = true;
            }
        }
        // If exist path after spaces.
        if(flag) {
            if((chdir(startPath)) != 0) {
                perror("chdir failed");
            }
        }
    }
}

// SystemCommand - not built in commands.
void systemCommand(char* line) {
    int stat;
    strcpy(tempLine, line);
    char* token = strtok(tempLine, SEP);
    int num = 0;

    // Count amount of arguments.
    while (token != NULL) {
        num++;
        token = strtok(NULL,SEP);
    }

    char* arguments[num+1];
    strcpy(tempLine, line);
    token = strtok(tempLine, SEP);
    int i = 0;

    // Push the arguments into array with NULL last element.
    while (token != NULL) {
        arguments[i] = token;
        i++;
        token = strtok(NULL,SEP);
    }
    arguments[i] = NULL;

    // Create child.
    pid_t pid = fork();
    if(pid < 0) {
        // If fork failed - no child.
        addToHistory(line, getpid());
        perror("fork failed");
    } else if(pid == 0) {
        // The child do execvp.
        int result = execvp(arguments[0], arguments);
        if(result == -1) {
            perror("execvp failed");
            exit(-1);
        }
    } else {
        waitpid(pid,&stat,0);
        // Fork success.
        addToHistory(line, pid);
    }
}

// The function add all arguments to PATH environment var.
void addToPath(int amount, char* arguments[]) {
    int i;
    for(i=1; i<amount; i++) {
        setenv("PATH", strcat(strcat(getenv("PATH"),":"),arguments[i]),0);
    }
}


int main(int argc, char *argv[]) {
    addToPath(argc, argv);
    pid_t pid = getpid();

    // The shell loop run until get exit.
    while (!SHOULD_STOP) {
        prompt();
        char* line = (char*) malloc(101);
        takeLine(line);

        if (FLAG_COMMAND == true) {
            // Take the command word.
            strcpy(tempLine, line);
            char* token = strtok(tempLine, SEP);

            if(strcmp(line,"exit") == 0) {
                exitFunc();
                free(line);
            } else if(strcmp(line,"history") == 0) {
                addToHistory(line, pid);
                showHistory();
            } else if(strcmp(token,"cd") == 0) {
                addToHistory(line, pid);
                cdCommand(line);
            } else {
                systemCommand(line);
            }
        } else {
            free(line);
        }

        FLAG_COMMAND = false;
    }

    return 0;
}

