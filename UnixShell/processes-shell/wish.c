#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#ifdef REG_ENHANCED
#define REG_CFLAGS REG_EXTENDED | REG_ENHANCED
#endif

// Constants
#define MAX_PATHS 256
#define MAX_ARGUMENTS 256

// Structure to pass arguments to thread functions
struct ThreadArguments
{
    pthread_t thread;
    char *command;
};

// Function prototypes
void printErrorMessage();
void *parseInput(void *arg);
int searchExecutablePath(char path[], char *firstArg);
void redirectOutput(FILE *out);
void executeCommands(char *arguments[], int numArguments, FILE *out);
char *trimString(char *s);
void cleanupResources();

// Global variables
FILE *inputFile = NULL;
char *searchPaths[MAX_PATHS] = {"/bin", NULL};
char *inputLine = NULL;

int main(int argc, char *argv[])
{
    int executionMode = 1;
    inputFile = stdin;
    size_t linecap = 0;
    ssize_t nread;

    if (argc > 1)
    {
        executionMode = 2;
        if (argc > 2 || (inputFile = fopen(argv[1], "r")) == NULL)
        {
            printErrorMessage();
            exit(EXIT_FAILURE);
        }
    }

    while (1)
    {
        if (executionMode == 1)
            printf("wish> ");

        if ((nread = getline(&inputLine, &linecap, inputFile)) > 0)
        {
            char *command;
            int numCommands = 0;
            struct ThreadArguments threadArgs[MAX_ARGUMENTS];

            if (inputLine[nread - 1] == '\n')
                inputLine[nread - 1] = '\0';

            char *temp = inputLine;

            while ((command = strsep(&temp, "&")) != NULL)
            {
                if (command[0] != '\0')
                {
                    threadArgs[numCommands++].command = strdup(command);
                    if (numCommands >= MAX_ARGUMENTS)
                        break;
                }
            }

            for (size_t i = 0; i < numCommands; i++)
            {
                if (pthread_create(&threadArgs[i].thread, NULL, &parseInput, &threadArgs[i]) != 0)
                    printErrorMessage();
            }

            for (size_t i = 0; i < numCommands; i++)
            {
                if (pthread_join(threadArgs[i].thread, NULL) != 0)
                    printErrorMessage();
                if (threadArgs[i].command != NULL)
                    free(threadArgs[i].command);
            }
        }
        else if (feof(inputFile) != 0)
        {
            atexit(cleanupResources);
            exit(EXIT_SUCCESS);
        }
    }

    return 0;
}

// Function to print error messages
void printErrorMessage()
{
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

// Thread function for parsing and executing commands
void *parseInput(void *arg)
{
    char *arguments[MAX_ARGUMENTS];
    int numArguments = 0;
    FILE *outputFile = stdout;
    struct ThreadArguments *threadArgs = (struct ThreadArguments *)arg;
    char *commandLine = threadArgs->command;

    char *command = strsep(&commandLine, ">");
    if (command == NULL || *command == '\0')
    {
        printErrorMessage();
        return NULL;
    }

    command = trimString(command);

    if (commandLine != NULL)
    {
        regex_t reg;
        if (regcomp(&reg, "\\S\\s+\\S", REG_CFLAGS) != 0)
        {
            printErrorMessage();
            regfree(&reg);
            return NULL;
        }
        if (regexec(&reg, commandLine, 0, NULL, 0) == 0 ||
            strstr(commandLine, ">") != NULL)
        {
            printErrorMessage();
            regfree(&reg);
            return NULL;
        }

        regfree(&reg);

        if ((outputFile = fopen(trimString(commandLine), "w")) == NULL)
        {
            printErrorMessage();
            return NULL;
        }
    }

    char **argPtr = arguments;
    while ((*argPtr = strsep(&command, " \t")) != NULL)
    {
        if (**argPtr != '\0')
        {
            *argPtr = trimString(*argPtr);
            argPtr++;
            if (++numArguments >= MAX_ARGUMENTS)
            {
                break;
            }
        }
    }

    if (numArguments > 0)
    {
        executeCommands(arguments, numArguments, outputFile);
    }

    return NULL;
}

// Function to search for an executable file in the specified paths
int searchExecutablePath(char path[], char *firstArg)
{
    int i = 0;
    while (searchPaths[i] != NULL)
    {
        snprintf(path, 256, "%s/%s", searchPaths[i], firstArg);
        if (access(path, X_OK) == 0)
            return 0;
        i++;
    }
    return -1;
}

// Function to redirect output
void redirectOutput(FILE *out)
{
    int outFileno;
    if ((outFileno = fileno(out)) == -1)
    {
        printErrorMessage();
        return;
    }

    if (outFileno != STDOUT_FILENO)
    {
        if (dup2(outFileno, STDOUT_FILENO) == -1)
        {
            printErrorMessage();
            return;
        }
        if (dup2(outFileno, STDERR_FILENO) == -1)
        {
            printErrorMessage();
            return;
        }
        fclose(out);
    }
}

// Function to execute commands
void executeCommands(char *arguments[], int numArguments, FILE *out)
{
    if (strcmp(arguments[0], "exit") == 0)
    {
        if (numArguments > 1)
            printErrorMessage();
        else
        {
            atexit(cleanupResources);
            exit(EXIT_SUCCESS);
        }
    }
    else if (strcmp(arguments[0], "cd") == 0)
    {
        if (numArguments == 1 || numArguments > 2)
            printErrorMessage();
        else if (chdir(arguments[1]) == -1)
            printErrorMessage();
    }
    else if (strcmp(arguments[0], "path") == 0)
    {
        size_t i = 0;
        searchPaths[0] = NULL;
        for (; i < numArguments - 1; i++)
            searchPaths[i] = strdup(arguments[i + 1]);

        searchPaths[i + 1] = NULL;
    }
    else
    {
        char path[256];
        if (searchExecutablePath(path, arguments[0]) == 0)
        {
            pid_t pid = fork();
            if (pid == -1)
                printErrorMessage();
            else if (pid == 0)
            {
                redirectOutput(out);

                if (execv(path, arguments) == -1)
                    printErrorMessage();
            }
            else
                waitpid(pid, NULL, 0);
        }
        else
            printErrorMessage();
    }
}

// Function to trim leading and trailing spaces from a string
char *trimString(char *s)
{
    while (isspace(*s))
        s++;

    if (*s == '\0')
        return s;

    char *end = s + strlen(s) - 1;
    while (end > s && isspace(*end))
        end--;

    end[1] = '\0';
    return s;
}

// Function to clean up allocated resources
void cleanupResources()
{
    free(inputLine);
    if (inputFile != stdin)
    {
        fclose(inputFile);
    }
}
