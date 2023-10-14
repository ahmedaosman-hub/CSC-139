#include <stdio.h>

int main(int argc, char *argv[]) {
    // Check if there is at least one command-line argument
    if (argc < 2) {
        fprintf(stdout, "wzip: file1 [file2 ...]\n");
        return 1; // Exit with status code 1 for missing arguments
    }

    char currentChar, prevChar = '\0'; // Initialize prevChar to a null character
    int count = 0;

    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");

        // Check if fopen() succeeded
        if (file == NULL) {
            fprintf(stderr, "%s: cannot open file '%s'\n", argv[0], argv[i]);
            return 1; // Exit with status code 1 for file open failure
        }

        while ((currentChar = fgetc(file)) != EOF) {
            if (currentChar == prevChar) {
                count++;
            } else {
                if (count > 0) {
                    fwrite(&count, sizeof(int), 1, stdout); // Write count
                    putchar(prevChar); // Write character
                }
                prevChar = currentChar;
                count = 1;
            }
        }

        fclose(file);
    }

    // Output the count and character for the last group of characters
    if (count > 0) {
        fwrite(&count, sizeof(int), 1, stdout);
        putchar(prevChar);
    }

    return 0; // Exit with status code 0 for successful execution
}
