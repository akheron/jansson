#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <jansson.h>
#include "jansson_private.h"
#include "strbuffer.h"

#define json_isdigit(c) ('0' <= (c) && (c) <= '9')

typedef ssize_t (*fill_func)(void *buf, size_t len, void *data);

#define MAX_BUF_LEN     1024

typedef struct
{
    fill_func fill; /* fill function (or NULL, if we read from a buffer) */
    void *data;
    char *buffer; /* read buffer */
    size_t pos; /* current position in the buffer */
    size_t buflen;
    size_t stream_pos; /* position (from the beginning of the stream) */
} stream_t;

static void error_set(json_error_t *error, stream_t *stream,
                      const char *msg, ...)
{
    va_list ap;
    char msg_text[JSON_ERROR_TEXT_LENGTH];

    size_t pos = 0;
    const char *result = msg_text;

    if(!error)
        return;

    if (stream)
        pos = stream->stream_pos + stream->pos;

    va_start(ap, msg);
    vsnprintf(msg_text, JSON_ERROR_TEXT_LENGTH, msg, ap);
    va_end(ap);

    jsonp_error_set(error, -1, -1, pos, "%s", result);
}

static void init_stream(stream_t *stream, const void *buffer, size_t buflen,
                        fill_func fill, void *data)
{
    if (fill) {
        stream->buffer = jsonp_malloc(MAX_BUF_LEN);
        stream->buflen = 0;
    } else {
        /* load from the given buffer */
        stream->buffer = (char *) buffer;
        stream->buflen = buflen;
    }
    stream->fill = fill;
    stream->data = data;
    stream->pos = 0;
    stream->stream_pos = 0;
}

static void finish_stream(stream_t *stream)
{
    if (stream->fill)
        jsonp_free(stream->buffer);
}

static int stream_refill(stream_t *stream)
{
    ssize_t read;

    if (!stream->fill)
        return 0;

    assert(stream->pos <= stream->buflen);
    stream->stream_pos += stream->pos;
    stream->buflen -= stream->pos;
    memmove(stream->buffer, &stream->buffer[stream->pos], stream->buflen);
    stream->pos = 0;

    if (MAX_BUF_LEN == stream->buflen)
        return 0;
    read = stream->fill(&stream->buffer[stream->buflen],
                        MAX_BUF_LEN - stream->buflen, stream->data);
    if (read > 0)
        stream->buflen += read;
    return read;
}

static int stream_peek(stream_t *stream)
{
    if (stream->pos >= stream->buflen) {
        if (stream_refill(stream) <= 0)
            return EOF;
    }
    return stream->buffer[stream->pos];
}

static int stream_getc(stream_t *stream)
{
    int c = stream_peek(stream);
    if (c != EOF)
        stream->pos++;
    return c;
}

static ssize_t search(stream_t *stream, int c)
{
    char *ptr = NULL;

    while (1) {
        ptr = memchr(&stream->buffer[stream->pos], c,
                     stream->buflen - stream->pos);
        if (ptr != NULL)
            break;
        if (stream_refill(stream) <= 0)
            return -1;
    }
    return ptr - stream->buffer;
}

static int validate_number(stream_t *stream, json_error_t *error)
{
    /* strtol functions allow extra spaces before the number */
    int c = stream_peek(stream);
    if (c != '-' && !json_isdigit(c)) {
        error_set(error, stream, "invalid number: %c", c);
        return -1;
    }
    return 0;
}

static char *parse_string(stream_t *stream, size_t flags,
                          json_error_t *error)
{
    char *endptr;
    ssize_t colon;
    size_t pos = 0;
    size_t length;
    char *string;

    (void) flags;

    colon = search(stream, ':');
    if (colon < 0) {
        error_set(error, stream, "unterminated string length");
        return NULL;
    }

    if (validate_number(stream, error))
        return NULL;

    /* can overflow, but who cares? */
    length = strtoul(&stream->buffer[stream->pos], &endptr, 10);
    if (endptr != &stream->buffer[colon]) {
        error_set(error, stream, "invalid string length");
        return NULL;
    }
    stream->pos = colon + 1;

    string = jsonp_malloc(length + 1);
    if (!string) {
        error_set(error, stream, "out of memory (string length %zd)", length);
        return NULL;
    }
    string[length] = '\0';

    while (pos < length) {
        char *zero;

        size_t chunk = stream->buflen - stream->pos;
        if (chunk == 0) {
            if (stream_refill(stream) <= 0) {
                error_set(error, stream, "partial string: %zd/%zd", pos,
                          length);
                goto error;
            }
            continue;
        }
        if (chunk > length - pos)
            chunk = length - pos;

        /* null bytes are not allowed inside strings */
        zero = memchr(&stream->buffer[stream->pos], '\0', chunk);
        if (zero) {
            stream->pos = zero - stream->buffer;
            error_set(error, stream, "string contains a zero byte");
            goto error;
        }

        memcpy(&string[pos], &stream->buffer[stream->pos], chunk);
        stream->pos += chunk;
        pos += chunk;
    }
    return string;

error:
    jsonp_free(string);
    return NULL;
}

#if JSON_INTEGER_IS_LONG_LONG
#define json_strtoint     strtoll
#else
#define json_strtoint     strtol
#endif

static json_t *parse_integer(stream_t *stream, size_t flags,
                             json_error_t *error)
{
    json_int_t value;
    char *endptr;
    ssize_t end;

    (void) flags;

    end = search(stream, 'e');
    if (end < 0) {
        error_set(error, stream, "unterminated integer");
        return NULL;
    }

    if (validate_number(stream, error))
        return NULL;

    errno = 0;
    value = json_strtoint(&stream->buffer[stream->pos], &endptr, 10);
    if (endptr != &stream->buffer[end]) {
        error_set(error, stream, "invalid integer");
        return NULL;
    }
    if(errno == ERANGE) {
        if(value < 0)
            error_set(error, stream, "too big negative integer");
        else
            error_set(error, stream, "too big integer");
        return NULL;
    }
    stream->pos = end + 1;

    return json_integer(value);
}

static json_t *parse_bencode(stream_t *stream, size_t flags,
                             json_error_t *error);

static json_t *parse_dict(stream_t *stream, size_t flags,
                          json_error_t *error)
{
    int c;
    char *key;
    json_t *value;
    json_t *object = json_object();
    if(!object)
        return NULL;

    c = stream_getc(stream);
    assert(c == 'd');

    while (1) {
        c = stream_peek(stream);
        if (c == EOF) {
            error_set(error, stream, "unterminated dictionary");
            goto error;
        }
        if (c == 'e')
            break;

        key = parse_string(stream, flags, error);
        if (!key)
            goto error;

        if(flags & JSON_REJECT_DUPLICATES) {
            if(json_object_get(object, key)) {
                jsonp_free(key);
                error_set(error, stream, "duplicate object key");
                goto error;
            }
        }

        value = parse_bencode(stream, flags, error);
        if (!value) {
            jsonp_free(key);
            goto error;
        }

        if(json_object_set_nocheck(object, key, value)) {
            jsonp_free(key);
            json_decref(value);
            goto error;
        }

        json_decref(value);
        jsonp_free(key);
    }
    stream_getc(stream);
    return object;

error:
    json_decref(object);
    return NULL;
}

static json_t *parse_list(stream_t *stream, size_t flags,
                          json_error_t *error)
{
    int c;
    json_t *elem;
    json_t *array = json_array();
    if(!array)
        return NULL;

    c = stream_getc(stream);
    assert(c == 'l');

    while (1) {
        c = stream_peek(stream);
        if (c == EOF) {
            error_set(error, stream, "unterminated list");
            goto error;
        }
        if (c == 'e')
            break;

        elem = parse_bencode(stream, flags, error);
        if (!elem)
            goto error;

        if(json_array_append(array, elem)) {
            json_decref(elem);
            goto error;
        }
        json_decref(elem);
    }
    stream_getc(stream);
    return array;

error:
    json_decref(array);
    return NULL;
}

static json_t *parse_bencode(stream_t *stream, size_t flags,
                             json_error_t *error)
{
    json_t *result = NULL;
    char *string;
    int c = stream_peek(stream);

    switch (c) {
    case 'd':
        result = parse_dict(stream, flags, error);
        break;
    case 'l':
        result = parse_list(stream, flags, error);
        break;
    case 'i':
        stream_getc(stream);
        result = parse_integer(stream, flags, error);
        break;
    case EOF:
        error_set(error, stream, "unexpected EOF");
        break;
    default:
        if (json_isdigit(c)) {
            string = parse_string(stream, flags, error);
            if (string) {
                result = json_string_nocheck(string);
                jsonp_free(string);
            }
        } else {
            error_set(error, stream, "invalid character: %c", c);
        }
    }
    return result;
}

json_t *json_bencode_loads(const char *string, size_t flags,
                           json_error_t *error)
{
    stream_t stream;

    jsonp_error_init(error, "<string>");

    if (string == NULL) {
        error_set(error, NULL, "wrong arguments");
        return NULL;
    }

    init_stream(&stream, string, strlen(string), NULL, NULL);

    return parse_bencode(&stream, flags, error);
}

json_t *json_bencode_loadb(const char *buffer, size_t buflen, size_t flags,
                           json_error_t *error)
{
    stream_t stream;

    jsonp_error_init(error, "<buffer>");

    if (buffer == NULL) {
        error_set(error, NULL, "wrong arguments");
        return NULL;
    }

    init_stream(&stream, buffer, buflen, NULL, NULL);

    return parse_bencode(&stream, flags, error);
}

static ssize_t file_fill(void *buf, size_t len, void *data)
{
    FILE *f = (FILE *) data;
    return fread(buf, 1, len, f);
}

json_t *json_bencode_loadf(FILE *input, size_t flags, json_error_t *error)
{
    stream_t stream;
    const char *source;
    json_t *result;

    if(input == stdin)
        source = "<stdin>";
    else
        source = "<stream>";

    jsonp_error_init(error, source);

    if (input == NULL) {
        error_set(error, NULL, "wrong arguments");
        return NULL;
    }

    init_stream(&stream, NULL, 0, file_fill, input);
    result = parse_bencode(&stream, flags, error);
    finish_stream(&stream);
    return result;
}
