#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    (void) argv;

    if (argc < 3) {
        fprintf(stderr, "USAGE: cxd <output-dir> <input-files...>\n");
        return -1;
    }

    /* TODO: cxd tools is not implemented */

    return 0;
}
