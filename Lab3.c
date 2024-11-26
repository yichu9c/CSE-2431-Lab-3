/* DO NOT REMOVE THIS COMMENT: CSE 2431 Lab 3 SP 23 Code 08032011 */
/* STUDENT NAME: (Yihone Chu) */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAXLINE 40 /* 40 chars per line, per command, is enough */

/* function to add the last user-entered command to historyBuff */
void updateHistory(char* historyBuff, char commandCopy[], int numCommands) {
    if (numCommands <= 5) {
        strcpy(historyBuff + (numCommands - 1) * 40, commandCopy);
    }
    else {
        numCommands = 5;
        int i;
        for (i = 0; i < numCommands; i++) {
            strcpy(historyBuff + i * 40, historyBuff + (i + 1) * 40);
        }
        strcpy(historyBuff + (numCommands - 1) * 40, commandCopy);
    }
}

/* function to print out commands in the current history buffer */
void printHistory(int numCommands, char* historyBuff) {
    int i;
    if (numCommands >= 5) {
        numCommands = 5;
    }
    for (i = 0; i < numCommands; i++) {
        printf("%d.\t%s\n", i + 1, historyBuff + (i * 40));
    }
}

/* function to check if the command is 'h' or 'history' */
int isHistory(char commandCopy[]) {
    char* h = "h\n";
    char* history = "history\n";
    if (strcmp(h, commandCopy) == 0 || strcmp(history, commandCopy) == 0) {
        return 1;
    }
    return 0;
}

/* function to check if the command is a rerun command */
int isRerun(char commandCopy[]) {
    char* runArray[6] = { "rr\n", "r1\n", "r2\n", "r3\n", "r4\n", "r5\n" };
    int i;
    for (i = 0; i < 6; i++) {
        if (strcmp(runArray[i], commandCopy) == 0) {
            return 1;
        }
    }
    return 0;
}

/* function to determine the index of the command to rerun */
int rerunIndex(int numCommands, char commandCopy[]) {
    if (numCommands > 5) {
        numCommands = 5;
    }
    if (strcmp(commandCopy, "rr\n\0") == 0) {
        return numCommands;
    }
    else {
        return commandCopy[1] - '0';
    }
}

/* setup() function to parse user input into command and arguments */
void setup(char inputBuff[], char* args[], int* background, char commandCopy[], int rerun) {
    int length, i, j, start;
    /* Read the user input if it's not a rerun command */
    if (rerun == 0) {
        length = read(STDIN_FILENO, inputBuff, MAXLINE);
        inputBuff[length] = '\0';  /* Null terminate the input */
        strcpy(commandCopy, inputBuff);  /* Copy to commandCopy */
    }
    else {
        strcpy(inputBuff, commandCopy);  /* For rerun, copy the previous command */
        length = strlen(inputBuff);
    }

    j = 0;
    start = -1;
    if (length == 0) exit(0);  /* Handle Ctrl-D (EOF) */
    if (length < 0) {
        perror("error reading command");
        exit(-1);  /* Terminate with error code of -1 */
    }

    /* Parse input buffer into arguments */
    for (i = 0; i < length; i++) {
        switch (inputBuff[i]) {
        case ' ':
        case '\t':  /* Argument separators */
            if (start != -1) {
                args[j] = &inputBuff[start];  /* Set up pointer */
                j++;
            }
            inputBuff[i] = '\0';  /* Add a null char */
            start = -1;
            break;
        case '\n':  /* Final char examined */
            if (start != -1) {
                args[j] = &inputBuff[start];
                j++;
            }
            inputBuff[i] = '\0';
            args[j] = NULL;  /* End of argument list */
            break;
        case '&':
            *background = 1;  /* Background process flag */
            inputBuff[i] = '\0';
            break;
        default:
            if (start == -1) start = i;
        }
    }
    args[j] = NULL;  /* Just in case input line was > 50 */
}

int main(void) {
    char inputBuff[MAXLINE];  /* Input buffer to hold the command entered */
    char* args[MAXLINE / 2 + 1];  /* Command line arguments */
    int background;  /* Equals 1 if a command is followed by '&', else 0 */
    char commandCopy[MAXLINE];  /* Store copy of command in setup function */
    char historyBuff[5 * MAXLINE];  /* Buffer to hold up to 5 commands */
    char* commandPtrs[6] = { NULL, &historyBuff[0], &historyBuff[40], &historyBuff[80], &historyBuff[120], &historyBuff[160] };
    int numCommands = 0;  /* Tracks the number of commands entered */
    int rerun;  /* Boolean to track if it's a rerun command */

    while (1) {
        background = 0;
        pid_t pid;

        printf("CS2431Sh$ ");  /* Shell prompt */
        fflush(0);
        rerun = 0;  /* Reset rerun flag */

        setup(inputBuff, args, &background, commandCopy, rerun);  /* Get next command */

        if (isHistory(commandCopy)) {
            printHistory(numCommands, historyBuff);  /* If it's a history command, print history */
        }
        else {
            rerun = isRerun(commandCopy);
            if (rerun) {
                /* If rerun command, echo and execute the previous command */
                printf("%s", commandPtrs[rerunIndex(numCommands, commandCopy)]);
                setup(inputBuff, args, &background, commandPtrs[rerunIndex(numCommands, commandCopy)], rerun);
            }
            else {
                /* If it's not a history or rerun, add to history and execute command */
                numCommands++;
                updateHistory(historyBuff, commandCopy, numCommands);
            }

            pid = fork();
            if (pid < 0) {
                printf("ERROR: fork failed!\n");
                printf("Terminating shell\n");
                exit(0);
            }
            else if (pid == 0) {
                execvp(args[0], args);  /* Execute command in child process */
            }
            else {
                if (background == 0) waitpid(pid, NULL, 0);  /* Wait for foreground process */
            }
        }
    }
    return 0;
}
