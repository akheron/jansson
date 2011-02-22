#include <string.h>
#include "jansson_private.h"

void jsonp_error_init(json_error_t *error, const char *source)
{
    if(error)
    {
        error->text[0] = '\0';
        error->line = -1;
        error->column = -1;
        error->position = 0;

        strncpy(error->source, source, JSON_ERROR_SOURCE_LENGTH);
        error->source[JSON_ERROR_SOURCE_LENGTH - 1] = '\0';
    }
}

void jsonp_error_set(json_error_t *error, int line, int column,
                     size_t position, const char *msg, ...)
{
    va_list ap;

    va_start(ap, msg);
    jsonp_error_vset(error, line, column, position, msg, ap);
    va_end(ap);
}

void jsonp_error_vset(json_error_t *error, int line, int column,
                      size_t position, const char *msg, va_list ap)
{
    if(!error)
        return;

    if(error->text[0] != '\0') {
        /* error already set */
        return;
    }

    error->line = line;
    error->column = column;
    error->position = position;

    vsnprintf(error->text, JSON_ERROR_TEXT_LENGTH, msg, ap);
}
