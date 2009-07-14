#ifndef UTF_H
#define UTF_H

int utf8_encode(int codepoint, char *buffer, int *size);

int utf8_check_first(char byte);
int utf8_check_full(const char *buffer, int size);

int utf8_check_string(const char *string, int length);

#endif
