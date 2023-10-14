#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
    Searches for a user-specified search in one or more files specified on the command line. 
    Prints lines containing the search term and handles open failures exiting with an error message
    Exit code 0 on success, code 1 on failure 
*/

int main(int argc, char *argv[]) {
    // Check if there is a command-line argument
    if (argc < 2) {
        fprintf(stdout, "%s searchterm [file ...]\n", argv[0]);
        return 1; // Exit with status code 1 
    }

    // Get the search term 
    char *searchTerm = argv[1];

    // Loop through each file 
    for (int i = 2; i < argc; i++) {
        FILE *file;

        // If the filename is "-", read from stdin
        if (strcmp(argv[i], "-") == 0) {
            file = stdin;
        } else {
            // Open the file for reading
            file = fopen(argv[i], "r");

            // Check if fopen() succeeded
            if (file == NULL) {
                fprintf(stderr, "%s: cannot open file '%s'\n", argv[0], argv[i]);
                return 1; // Exit with status code 1 
            }
        }

        char *line = NULL;
        size_t lineSize = 0;

        // Read and process each line of the file
        while (getline(&line, &lineSize, file) != -1) {
            // Check if the search term exists in the line
            if (strstr(line, searchTerm) != NULL) {
                printf("%s", line);
            }
        }

        // Free dynamically allocated memory 
        free(line);

        // Close the file
        if (file != stdin) {
            fclose(file);
        }
    }

    return 0; // Exit with status code 0 
}
