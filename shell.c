#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[MAX_COMMAND_LINE_LEN]; // Updated to allow for dynamic length of the prompt
char delimiters[] = " \t\r\n";
extern char **environ;

// Function to update the prompt to print the current working directory
void updatePrompt() {
    char cwd[MAX_COMMAND_LINE_LEN];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(prompt, sizeof(prompt), "%s> ", cwd);
    } else {
        perror("getcwd");
        snprintf(prompt, sizeof(prompt), "> ");
    }
}

int main() {
    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];

    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS];

    while (true) {
        do {
            // Update and print the shell prompt with the current working directory.
            updatePrompt();
            printf("%s", prompt);
            fflush(stdout);

            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")

            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
        } while (command_line[0] == 0x0A);  // while just ENTER pressed

        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }

        // 1. Tokenize the command line input (split it on whitespace)
        char *token = strtok(command_line, delimiters);
        int argCount = 0;
        while (token != NULL && argCount < MAX_COMMAND_LINE_ARGS) {
            arguments[argCount++] = token;
            token = strtok(NULL, delimiters);
        }
        arguments[argCount] = NULL; // Null-terminate the argument array.

        // 2. Implement Built-In Commands
        // You can check if the first argument is a built-in command and handle it accordingly.
        // For example: if (strcmp(arguments[0], "cd") == 0) { ... }

        // 3. Create a child process which will execute the command line input
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) { // Child process
            // Execute the command using execvp
            if (execvp(arguments[0], arguments) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else { // Parent process
            // 4. The parent process should wait for the child to complete unless it's a background process
            if (wait(NULL) == -1) {
                perror("wait");
                exit(EXIT_FAILURE);
            }
        }
    }
    // This should never be reached.
    return -1;
}
