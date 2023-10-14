#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024

void execute_command(char* command) {
    // Tokenize the command into arguments
    char* args[MAX_INPUT_SIZE];
    int arg_count = 0;
    char* token = strtok(command, " \t\n");
    while (token != NULL) {
        args[arg_count] = token;
        arg_count++;
        token = strtok(NULL, " \t\n");
    }
    args[arg_count] = NULL;

    // Handle built-in commands
    if (arg_count > 0) {
        if (strcmp(args[0], "exit") == 0) {
            exit(0);
        }
    }

    // Create a child process to execute the command
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("wish");
            exit(1);
        }
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("wish");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [batch_file]\n", argv[0]);
        exit(1);
    }

    FILE* batch_file = NULL;
    if (argc == 2) {
        batch_file = fopen(argv[1], "r");
        if (!batch_file) {
            perror("wish");
            exit(1);
        }
    }

    char input[MAX_INPUT_SIZE];

    if (batch_file == NULL) {
        // Interactive mode
        while (1) {
            printf("wish> ");
            fflush(stdout);
            if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
                break;
            }
            execute_command(input);
        }
    } else {
        // Batch mode
        while (fgets(input, MAX_INPUT_SIZE, batch_file) != NULL) {
            execute_command(input);
        }
        fclose(batch_file);
    }

    return 0;
}
