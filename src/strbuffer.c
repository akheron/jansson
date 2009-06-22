#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include "strbuffer.h"
#include "util.h"

#define STRBUFFER_MIN_SIZE  16
#define STRBUFFER_FACTOR    2

int strbuffer_init(strbuffer_t *strbuff)
{
    strbuff->size = STRBUFFER_MIN_SIZE;
    strbuff->length = 0;

    strbuff->value = malloc(strbuff->size);
    if(!strbuff->value)
        return -1;

    memset(strbuff->value, 0, strbuff->size);
    return 0;
}

void strbuffer_close(strbuffer_t *strbuff)
{
    free(strbuff->value);
    strbuff->size = 0;
    strbuff->length = 0;
    strbuff->value = NULL;
}

const char *strbuffer_value(strbuffer_t *strbuff)
{
    return strbuff->value;
}

char *strbuffer_steal_value(strbuffer_t *strbuff)
{
    char *result = strbuff->value;
    strbuffer_init(strbuff);
    return result;
}

int strbuffer_append(strbuffer_t *strbuff, const char *string)
{
    return strbuffer_append_bytes(strbuff, string, strlen(string));
}

int strbuffer_append_bytes(strbuffer_t *strbuff, const char *data, int size)
{
    if(strbuff->length + size >= strbuff->size)
    {
        strbuff->size = max(strbuff->size * STRBUFFER_FACTOR,
                            strbuff->length + size + 1);

        strbuff->value = realloc(strbuff->value, strbuff->size);
        if(!strbuff->value)
            return -1;

        memset(strbuff->value + strbuff->length + size, 0,
               strbuff->size - strbuff->length - size);
    }

    memcpy(strbuff->value + strbuff->length, data, size);
    strbuff->length += size;

    return 0;
}
