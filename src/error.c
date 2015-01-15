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
        if(source)
            jsonp_error_set_source(error, source);
        else
            error->source[0] = '\0';
    }
}

void jsonp_error_set_source(json_error_t *error, const char *source)
{
    size_t length;

    if(!error || !source)
        return;

    length = strlen(source);
    if(length < JSON_ERROR_SOURCE_LENGTH)
        strcpy(error->source, source);
    else {
        size_t extra = length - JSON_ERROR_SOURCE_LENGTH + 4;
        strcpy(error->source, "...");
        strcpy(error->source + 3, source + extra);
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
    error->position = (int)position;

    vsnprintf(error->text, JSON_ERROR_TEXT_LENGTH, msg, ap);
    error->text[JSON_ERROR_TEXT_LENGTH - 1] = '\0';
}

char *json_error_get_source_text(json_error_t *error, const char *src)
{
    const char *start;
    const char *end;
    size_t len;
    char *s;

    start = &src[(error->position - error->column)];
    end = strchr(start, '\n');

    if (!end) {
        end = src + strlen(src);
    }

    len = (end - start);

    if (!(s = malloc(len))) {
        return NULL;
    }

    if (snprintf(s, len - 1, "%*s", len, start) < 0) {
        free(s);
        return NULL;
    }

    return s;
}

char *json_error_get_arrow(json_error_t *error, const char *src, int length, size_t flags)
{
    size_t msglen;
    int offset = 0;
    int ret = 0;
    char *msg;
    #define ARROWLEN 5
    const char padchars[] = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    if (strlen(src) < 2) {
        return strdup("");
    }

    if (length < 0) {
        length = ARROWLEN;
    }

    if (length >= sizeof(padchars)) {
        length = sizeof(padchars);
    }

    msglen = (error->column + strlen(error->text) + ARROWLEN + 1) * 2;

    if (!(msg = malloc(msglen))) {
        return NULL;
    }

    msglen--;

    #ifndef _WIN32
    if (flags & JSON_ERROR_COLOR) {
        if ((ret = snprintf(&msg[offset], msglen, "%s", "\x1b[01;32m")) < 0) {
            goto fail;
        }
    }
    offset += ret;
    #endif // _WIN32

    // Print the arrow.
    if ((ret = snprintf(&msg[offset], msglen - offset,
                "%*s^%*.*s",
                error->column, "",
                length, length, padchars)) < 0) {
        goto fail;
    }

    offset += ret;

    #ifndef _WIN32
	if (flags & JSON_ERROR_COLOR) {
        if (snprintf(&msg[offset], msglen - offset, "%s", "\x1b[0m\x1b[0m") < 0) {
            goto fail;
        }
    }
    #endif // _WIN32

    return msg;
fail:
    if (msg) free(msg);
    return NULL;
}

char *json_error_get_detailed(json_error_t *error, const char *src, size_t flags)
{
    char *problem_src = NULL;
    char *arrow = NULL;
    char *s = NULL;
    size_t len;
	int arrow_length = flags & JSON_ERROR_ARROW_MAXLEN;

	if (!arrow_length)
		arrow_length = -1;

    if (!(problem_src = json_error_get_source_text(error, src))) {
        return NULL;
    }

    if (!(arrow = json_error_get_arrow(error, src, arrow_length, flags))) {
        goto fail;
    }

    len = (strlen(problem_src)
         + strlen(arrow)
         + strlen(error->text)) * 2;

    if (!(s = malloc(len))) {
        goto fail;
    }

    // TODO: If the error message goes outside of the console width, flip it!
    if (snprintf(s, len - 1, "%s\n%s (%s)\n",
            problem_src, arrow, error->text) < 0) {
        goto fail;
    }

    free(problem_src);
    free(arrow);

    return s;
fail:
    if (problem_src) free(problem_src);
    if (arrow) free(arrow);
    if (s) free(s);
    return NULL;
}

