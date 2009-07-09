#include <stdio.h>
#include <jansson.h>

int main(int argc, char *argv[])
{
    json_t *json;
    json_error_t error;

    if(argc != 3) {
        fprintf(stderr, "usage: %s infile outfile\n", argv[0]);
        return 2;
    }

    json = json_load(argv[1], &error);
    if(!json) {
        fprintf(stderr, "%d\n%s\n", error.line, error.text);
        return 1;
    }

    json_dump(json, argv[2], 0);
    json_decref(json);

    return 0;
}
