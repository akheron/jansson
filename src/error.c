#include <string.h>
#include "jansson_private.h"
#include "utf.h"

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
        strncpy(error->source, source, length + 1);
    else {
        size_t extra = length - JSON_ERROR_SOURCE_LENGTH + 4;
        strncpy(error->source, "...", 3);
        strncpy(error->source + 3, source + extra, length - extra + 1);
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

static size_t json_error_get_utf8_column(json_error_t *error, const char *src)
{
    size_t i = 0;
    const char *s = src;
    const char *colend = src + error->column;

    while (s < colend) {
        if (!(s = utf8_iterate(s, colend - s, NULL)))
            return error->column;
        i++;
    }

    return i;
}

static char *json_error_get_source_text(json_error_t *error, const char *src)
{
    const char *start;
    const char *end;
    size_t len;
    char *s;

    // TODO: Pick start properly so we don't split a UTF-8 code point.
    start = &src[(error->position - error->column)];
    end = strchr(start, '\n');

    if (!end) {
        end = src + strlen(src);
    }

    len = (end - start) + 2;

    if (!(s = jsonp_malloc(len))) {
        return NULL;
    }

    if (snprintf(s, len - 1, "%.*s", (int)len - 2, start) < 0) {
        free(s);
        return NULL;
    }

    return s;
}

static char *json_error_get_arrow(json_error_t *error,
    const char *src, size_t flags)
{
    size_t msglen;
    int offset = 0;
    int ret = 0;
    char *msg;
    size_t utf8_column = 0;
    int arrowlen = flags & JSON_ERROR_ARROW_MAXLEN;
    #define DEFAULT_ARROWLEN 5
    const char padchars[] = "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";

    if (strlen(src) < 2) {
        return jsonp_strdup("");
    }

    if (arrowlen < 0) {
        arrowlen = DEFAULT_ARROWLEN;
    }

    if (arrowlen >= (int)sizeof(padchars)) {
        arrowlen = sizeof(padchars);
    }

    msglen = (error->column + strlen(error->text) + arrowlen + 1) * 2;

    if (!(msg = jsonp_malloc(msglen))) {
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

    // TODO: Make sure this works on Windows.
    // TODO: How to check if console supports UTF-8? If not we want normal column value.
    // Get the error column based on UTF-8 code points
    // so that the arrow points in the correct position.
    utf8_column = json_error_get_utf8_column(error, src);

    if ((utf8_column + strlen(error->text) + (arrowlen + 1)) > JSON_ERROR_SOURCE_LENGTH) {
        // Flip the arrow if the column is too far to the right.
        if ((ret = snprintf(&msg[offset], msglen - offset,
                    "%*.*s^", arrowlen, arrowlen, padchars)) < 0) {
            goto fail;
        }
    } else {

        // Print the arrow.
        if ((ret = snprintf(&msg[offset], msglen - offset,
                    "%*s^%*.*s",
                    (int)utf8_column, "",
                    arrowlen, arrowlen, padchars)) < 0) {
            goto fail;
        }
    }

    offset += ret;

    #ifndef _WIN32
    if (flags & JSON_ERROR_COLOR) {
        if (snprintf(&msg[offset], msglen - offset,
            "%s", "\x1b[0m\x1b[0m") < 0) {
            goto fail;
        }
    }
    #endif // _WIN32

    return msg;
fail:
    if (msg) jsonp_free(msg);
    return NULL;
}

char *json_error_get_detailed(json_error_t *error, const char *src, size_t flags)
{
    char *problem_src = NULL;
    char *arrow = NULL;
    char *s = NULL;
    size_t len;
    size_t arrowlen = (flags & JSON_ERROR_ARROW_MAXLEN) + 1;
    size_t textlen;
    size_t utf8_column;
    size_t srclen;
    size_t total;

    if (!(problem_src = json_error_get_source_text(error, src))) {
        return NULL;
    }

    if (!(arrow = json_error_get_arrow(error, src, flags))) {
        goto fail;
    }

    textlen = strlen(error->text);
    srclen = strlen(problem_src);
    utf8_column = json_error_get_utf8_column(error, src);
    total = (utf8_column + arrowlen + textlen + 3);
    //problem_src[error->column]= '_';

    len = (srclen + arrowlen + textlen) * 2;

    if (!(s = malloc(len))) {
        goto fail;
    }

    // If the error message goes outside of the console width, flip it!
    if (total > JSON_ERROR_SOURCE_LENGTH) {
        if (snprintf(s, len - 1, "%s\n%*s(%s) %s\n",
                problem_src, 
                (int)(utf8_column - textlen - 3 - arrowlen), "", 
                error->text, arrow) < 0) {
            goto fail;
        }
    } else {

        if (snprintf(s, len - 1, "%s\n%s (%s)\n",
                problem_src, arrow, error->text) < 0) {
            goto fail;
        }
    }

    jsonp_free(problem_src);
    jsonp_free(arrow);

    return s;
fail:
    if (problem_src) jsonp_free(problem_src);
    if (arrow) jsonp_free(arrow);
    if (s) jsonp_free(s);
    return NULL;
}

void json_error_print_detailed(FILE *fd, json_error_t *error, const char *src, size_t flags)
{
    char *d;

    if (!(d = json_error_get_detailed(error, src, flags))) {
        // Since we're reporting an error, at least report something!
        fprintf(fd, "%s\n", error->text);
        return;
    }

    fprintf(fd, "%s", d);
    jsonp_free(d);
}

