#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "USAGE: cxd <output-dir> <input-files...>\n");
        return -1;
    }

    const char *output_ext = ".h";
    char *output_dir = argv[1];
    char *output_file = NULL;

    for (int i = 2; i < argc; ++i) {
        char *input_file = argv[i];
        size_t output_file_size = strlen(output_dir) + strlen(input_file) + strlen(output_ext) + 10;

        output_file = malloc(sizeof(char) * output_file_size);

        snprintf(output_file, output_file_size, "%s/%s%s", output_dir, input_file, output_ext);
        fclose(fopen(output_file, "w"));

        free(output_file);
    }

    /* TODO: cxd tools is not implemented */

    return 0;
}
