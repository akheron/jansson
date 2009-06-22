#ifndef STRBUFFER_H
#define STRBUFFER_H

typedef struct {
    char *value;
    int length;
    int size;
} strbuffer_t;

int strbuffer_init(strbuffer_t *strbuff);
void strbuffer_close(strbuffer_t *strbuff);

const char *strbuffer_value(strbuffer_t *strbuff);
char *strbuffer_steal_value(strbuffer_t *strbuff);

int strbuffer_append(strbuffer_t *strbuff, const char *string);
int strbuffer_append_bytes(strbuffer_t *strbuff, const char *data, int size);

#endif
