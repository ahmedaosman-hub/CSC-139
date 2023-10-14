#include <stdio.h>
#include <stdlib.h>

/* 
    Reads one or more files specified on the command line and prints their contents.  
    Handles file open failures and exit with the appropriate error message. 
*/

int main(int argc, char *argv[]) {

    // Check if there are any files 
    if (argc < 2) {
        return 0; // Exit with status code 0 
    }

    // Loop through each file 
    for (int i = 1; i < argc; i++) {
        // Open the file for reading
        FILE *file = fopen(argv[i], "r");

        // Check if fopen() succeeded
        if (file == NULL) {
fprintf(stdout, "wcat: cannot open file\n");
              return 1; // Exit with status code 1 if failed
        }

        // Read and print the file line by line
        char buffer[1024]; 
        while (fgets(buffer, sizeof(buffer), file) != NULL) {
            printf("%s", buffer);
        }

        // Close the file
        fclose(file);
    }

    return 0; // Exit with status code 0 
}


