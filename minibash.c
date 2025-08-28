#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

#define MAX_LINE 1024 // Maximum length of the command line
#define MAX_ARGS 64   // Maximum number of arguments
pid_t bg_pid = 0; // Global variable to store background process ID


// Function to count the number of elements in an array until a NULL element is found
int countElementsUntilNull(char *string_array[]) {
    int count = 0;
    while (string_array[count] != NULL) {
        count++;
    }
    return count;
}

// helper Function to print the elements of the array
void printArray(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        printf("args[%d]: %s\n", i, args[i]);
        i++;
    }
}

// Helper function to trim leading and trailing whitespace
char* trimWhitespace(char* str) {
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

// Function to parse the command line input into arguments
void parseCommand(char *line, char **args, char* sep) {
    int i = 0;
    char *token = strtok(line, sep);
    while (token != NULL) {
        args[i++] = trimWhitespace(token); // Trim whitespace from each token
        token = strtok(NULL, sep);
    }
    args[i] = NULL; // Null-terminate the array
}



// Function to execute a command
void executeCommand(char **argsToExec) {
    pid_t pid = fork(); // Fork a new process

    int size = countElementsUntilNull(argsToExec);


    if (size < 1 || size > 4) {
        printf("Incorrect number of arguments\n");
        return;
    }




    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        if (execvp(argsToExec[0], argsToExec) == -1) {
            perror("Exec failed");
        }
        exit(1);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for child process to complete
    }
}

// Function to execute a command with redirection
void executeRedirectionCommand(char **argsToExec) {

    int size = countElementsUntilNull(argsToExec);



    if (size < 1 || size > 4) {
        printf("Incorrect number of arguments\n");
        return;
    }

    pid_t pid = fork(); // Fork a new process

    


    
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        int i = 0;
        int input = -1, output = -1, append = -1;

        // Check for input, output, and append redirections
        while (argsToExec[i] != NULL) {
            if (strcmp(argsToExec[i], "<") == 0) {
                input = open(argsToExec[i + 1], O_RDONLY);
                if (input < 0) {
                    perror("open input file error");
                    exit(1);
                }
                dup2(input, STDIN_FILENO); // Redirect input
                close(input);
                argsToExec[i] = NULL;
            } else if (strcmp(argsToExec[i], ">") == 0) {
                output = open(argsToExec[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output < 0) {
                    perror("open output file error");
                    exit(1);
                }
                dup2(output, STDOUT_FILENO); // Redirect output
                close(output);
                argsToExec[i] = NULL;
            } else if (strcmp(argsToExec[i], ">>") == 0) {
                append = open(argsToExec[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                if (append < 0) {
                    perror("open append file error");
                    exit(1);
                }
                dup2(append, STDOUT_FILENO); // Redirect output (append mode)
                close(append);
                argsToExec[i] = NULL;
            }
            i++;
        }

        if (execvp(argsToExec[0], argsToExec) == -1) {
            perror("Exec failed");
        }
        exit(1);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for child process to complete
    }
}

// Function to count words in a file
void wordCountInFile(char **args) {
    char *argsWc[] = {"wc", "-w", args[1], NULL};
    executeCommand(argsWc); // Execute word count command
}

// Function to filter array elements and optionally append a new element
char** filterArray(char *arr[], int size, const char *removal, int *newSize, char *toAppend) {
    char **newArr = (char **)malloc(MAX_ARGS * sizeof(char *));
    int j;

    if (toAppend != NULL){
        newArr[0] = (char *)malloc(10 * sizeof(char));
        newArr[0] = toAppend;
        j = 1;
    } else {
        j = 0;
    }

    for (int i = 0; i < size; i++) {
        if (strcmp(arr[i], removal) != 0) {
            newArr[j] = (char *)malloc((strlen(arr[i]) + 1) * sizeof(char));
            strcpy(newArr[j], arr[i]);
            j++;
        }
    }

    newArr[j] = NULL;
    
    *newSize = j;
    return newArr;
}

// Function to concatenate files
void concatenateFiles(char **args) {
    int argCount = 0;
    for (int i = 0; args[i] != NULL; i++) {
        argCount++;
    }
    int newSize;

    char **updatedArray = filterArray(args, argCount, "~", &newSize, "cat");
    
    if (newSize < 1 || newSize > 4) {
        printf("Incorrect number of arguments in concatenation\n");
        return;
    }

    executeCommand(updatedArray);
}

// Function to execute piped commands
void executePipedCommands(char **args) {
    int numCommands = 0;
    while (args[numCommands] != NULL) {
        numCommands++;
    }

    if (numCommands < 1 || numCommands > 4) {
        printf("Incorrect number of arguments in pipe\n");
        return;
    }

    int pipefds[2 * (numCommands - 1)];

    for (int i = 0; i < (numCommands - 1); i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    int j = 0;
    for (int i = 0; i < numCommands; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            if (i != 0) {
                if (dup2(pipefds[j - 2], 0) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            if (i != numCommands - 1) {
                if (dup2(pipefds[j + 1], 1) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            for (int k = 0; k < 2 * (numCommands - 1); k++) {
                close(pipefds[k]);
            }

            char *commandArgs[MAX_ARGS];
            parseCommand(args[i], commandArgs, " \t\n");
            int size = countElementsUntilNull(commandArgs);
            if (size < 1 || size > 4) {
                printf("Incorrect number of arguments\n");
                return;
            }
            execvp(commandArgs[0], commandArgs);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        j += 2;
    }

    for (int i = 0; i < 2 * (numCommands - 1); i++) {
        close(pipefds[i]);
    }

    for (int i = 0; i < numCommands; i++) {
        wait(NULL);
    }
}

// Function to execute commands sequentially
void executeSequentially(char **args) {
    int numCommands = 0;
    while (args[numCommands] != NULL) {
        numCommands++;
    }

    if (numCommands < 1 || numCommands > 4) {
        printf("Incorrect number of arguments in sequential\n");
        return;
    }

    for (int i = 0; i < numCommands; i++) {
        char *commandArgs[MAX_ARGS];
        parseCommand(args[i], commandArgs, " \t\n");
        executeCommand(commandArgs);
    }
}

// Function to bring a background process to the foreground
void bringToForeground() {
    if (bg_pid != 0) {
        printf("Bringing process to foreground...\n");
        tcsetpgrp(STDIN_FILENO, bg_pid);  // Set the background process to the foreground
        kill(bg_pid, SIGCONT);  // Continue the process if it was stopped
        int status;
        if (waitpid(bg_pid, &status, WUNTRACED) == -1) {
            perror("waitpid failed");
        }
        tcsetpgrp(STDIN_FILENO, getpgrp());  // Return control to minibash
        bg_pid = 0;  // Reset the background process ID
    } else {
        printf("No background process to bring to foreground.\n");
    }
}

// Function to execute a command in the background
void executeBackgroundCommand(char **args) {
    int size = countElementsUntilNull(args);



    if (size < 1 || size > 4) {
        printf("Incorrect number of arguments\n");
        return;
    }

    pid_t pid = fork(); // Fork a new process
    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        setpgid(0, 0); // Set a new process group ID

        if (execvp(args[0], args) < 0) { // Execute the command
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    } else {
        // Parent process
        bg_pid = pid; // Store the background process ID
        printf("Running in background\n");
    }
}

// Function to count specific strings in an array
int countSpecificStrings(char *string_array[], int array_size, const char *target_string) {
    int count = 0;
    for (int i = 0; i < array_size; i++) {
        if (strcmp(string_array[i], target_string) == 0) {
            count++;
        }
    }
    return count;
}




// Function to execute conditional commands (&& and ||), input string is the arg
void executeConditionalCommands(char *input) {
    char *commands[5];
    int i = 0;

    // Split input by &&
    commands[i] = strtok(input, "&&");
    while (commands[i] != NULL) {
        i++;
        commands[i] = strtok(NULL, "&&");
    }

    for (int j = 0; j < i; j++) {
        char *subcommands[5];
        int k = 0;

        // Split each command by ||
        subcommands[k] = strtok(commands[j], "||");
        while (subcommands[k] != NULL) {
            k++;
            subcommands[k] = strtok(NULL, "||");
        }

        int success = 0; // Flag to track if any command in || chain succeeded
        for (int l = 0; l < k; l++) {
            char *args[20];
            parseCommand(subcommands[l], args, " \t\n");
            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                execvp(args[0], args); // Execute the command
                perror("execvp failed");
                exit(EXIT_FAILURE);
            } else {
                // Parent process
                int status;
                wait(&status); // Parent waits for child process to complete
                if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                    success = 1; // Command succeeded
                    break;
                }
            }
        }

        // If the command sequence in the || chain failed, stop processing further
        if (!success) {
            break;
        }
    }
}



// Function to handle the input command and execute the appropriate function
void handleCommand(char *input) {
    char *args[MAX_ARGS];

    if (input[0] == '#') {
        parseCommand(input, args, " \t\n");
        wordCountInFile(args);
    } else if (strstr(input, "~")) {
        parseCommand(input, args, " \t\n");
        concatenateFiles(args);
    } else if (strstr(input, "|")) {
        parseCommand(input, args, "|");
        executePipedCommands(args);
    } else if (strstr(input, ";")) {
        parseCommand(input, args, ";");
        executeSequentially(args);
    } else if (strstr(input, "+")) {
        input[strlen(input)-1] = '\0'; 
        parseCommand(input, args, " \t\n");
        executeBackgroundCommand(args);   
    } else if (strstr(input, "fore")) {
        bringToForeground();
    } else if (strstr(input, ">") || strstr(input, ">>") || strstr(input, "<")) {
        parseCommand(input, args, " \t\n");
        executeRedirectionCommand(args);  
    } else if (strstr(input, "&&") || strstr(input, "||")) {
        executeConditionalCommands(input);
    }
    else {
        parseCommand(input, args, " \t\n");
        executeCommand(args);
    }
}

// Main function to run the minibash shell
int main() {
    char line[MAX_LINE];
    
    while (1) {
        printf("minibash$ ");
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // Remove trailing newline character
        line[strcspn(line, "\n")] = '\0';
        
        if (strcmp(line, "dter") == 0) {
            break;
        }

        handleCommand(line);
    }
    
    return 0;
}
