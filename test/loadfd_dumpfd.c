#include <stdio.h>
#include <unistd.h>
#include <jansson.h>

int main(int argc, char *argv[])
{
    json_t *json;
    json_error_t error;

    if(argc != 1) {
        fprintf(stderr, "usage: %s\n", argv[0]);
        return 2;
    }

    json = json_loadfd(STDIN_FILENO, &error);
    if(!json) {
        fprintf(stderr, "%d\n%s\n", error.line, error.text);
        return 1;
    }

    json_dumpfd(json, STDOUT_FILENO, 0);
    json_decref(json);

    return 0;
}
