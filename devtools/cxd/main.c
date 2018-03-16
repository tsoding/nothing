#include <stdio.h>

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "USAGE: cxd <input-file> <output-file>\n");
        return -1;
    }

    char *output_file = argv[2];
    fclose(fopen(output_file, "w"));

    /* TODO: cxd tools is not implemented */

    return 0;
}
