#include <stdio.h>

int main(int argc, char *argv[]) {
    // Check if there is at least one command-line argument
    if (argc < 2) {
        fprintf(stdout, "wunzip: file1 [file2 ...]\n");
        return 1; // Exit with status code 1 for missing arguments
    }

    // Loop through each file specified on the command line
    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");

        // Check if fopen() succeeded
        if (file == NULL) {
            fprintf(stderr, "%s: cannot open file '%s'\n", argv[0], argv[i]);
            return 1; // Exit with status code 1 for file open failure
        }

        int count;
        char character;

        while (fread(&count, sizeof(int), 1, file) == 1) {
            fread(&character, sizeof(char), 1, file);

            for (int j = 0; j < count; j++) {
                putchar(character);
            }
        }

        fclose(file);
    }

    return 0; // Exit with status code 0 for successful execution
}
