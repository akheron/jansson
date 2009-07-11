#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>

#include <jansson.h>
#include "strbuffer.h"

#define TOKEN_INVALID         -1
#define TOKEN_EOF              0
#define TOKEN_STRING         256
#define TOKEN_INTEGER        257
#define TOKEN_REAL           258
#define TOKEN_TRUE           259
#define TOKEN_FALSE          260
#define TOKEN_NULL           261

typedef struct {
    const char *input;
    const char *start;
    int token;
    int line, column;
    union {
        char *string;
        int integer;
        double real;
    } value;
} lex_t;


/*** error reporting ***/

static void error_set(json_error_t *error, const lex_t *lex,
                      const char *msg, ...)
{
    va_list ap;
    char text[JSON_ERROR_TEXT_LENGTH];

    if(!error)
        return;

    va_start(ap, msg);
    vsnprintf(text, JSON_ERROR_TEXT_LENGTH, msg, ap);
    va_end(ap);

    if(lex)
    {
        error->line = lex->line;
        if(*lex->start)
        {
            int n = (int)(lex->input - lex->start);
            snprintf(error->text, JSON_ERROR_TEXT_LENGTH,
                     "%s near '%.*s'", text, n, lex->start);
        }
        else
        {
            snprintf(error->text, JSON_ERROR_TEXT_LENGTH,
                     "%s near end of file", text);
        }
    }
    else
    {
        error->line = -1;
        snprintf(error->text, JSON_ERROR_TEXT_LENGTH, "%s", text);
    }
}


/*** lexical analyzer ***/

static void lex_scan_string(lex_t *lex)
{
    /* skip the " */
    const char *p = lex->input + 1;
    char *t;

    lex->token = TOKEN_INVALID;

    while(*p != '"') {
        if(*p == '\0') {
            /* unterminated string literal */
            goto out;
        }

        if(0 <= *p && *p <= 0x1F) {
            /* control character */
            goto out;
        }
        else if(*p == '\\') {
            p++;
            if(*p == 'u') {
                p++;
                for(int i = 0; i < 4; i++, p++) {
                    if(!isxdigit(*p))
                        goto out;
                }
            }
            else if(*p == '"' || *p == '\\' || *p == '/' || *p == 'b' ||
                    *p == 'f' || *p == 'n' || *p == 'r' || *p == 't')
                p++;
            else
                goto out;
        }
        else
            p++;
    }

    /* the actual value is at most of the same length as the source
       string */
    lex->value.string = malloc(p - lex->start);
    if(!lex->value.string) {
        /* this is not very nice, since TOKEN_INVALID is returned */
        goto out;
    }

    /* the target */
    t = lex->value.string;

    p = lex->input + 1;
    while(*p != '"') {
        if(*p == '\\') {
            p++;
            if(*p == 'u') {
                /* TODO: \uXXXX not supported yet */
                free(lex->value.string);
                lex->value.string = NULL;
                goto out;
            } else {
                switch(*p) {
                    case '"': case '\\': case '/':
                        *t = *p; break;
                    case 'b': *t = '\b'; break;
                    case 'f': *t = '\f'; break;
                    case 'n': *t = '\n'; break;
                    case 'r': *t = '\r'; break;
                    case 't': *t = '\t'; break;
                    default: assert(0);
                }
            }
        }
        else
            *t = *p;

        t++;
        p++;
    }
    /* skip the " */
    p++;

    *t = '\0';
    lex->token = TOKEN_STRING;

out:
    lex->input = p;
}

static void lex_scan_number(lex_t *lex)
{
    const char *p = lex->input;
    char *end;

    lex->token = TOKEN_INVALID;

    if(*p == '-')
        p++;

    if(*p == '0') {
        p++;
        if(isdigit(*p))
          goto out;
    }
    else /* *p != '0' */ {
        p++;
        while(isdigit(*p))
            p++;
    }

    if(*p != '.' && *p != 'E' && *p != 'e') {
        lex->token = TOKEN_INTEGER;

        lex->value.integer = strtol(lex->start, &end, 10);
        assert(end == p);

        goto out;
    }

    if(*p == '.') {
        p++;
        if(!isdigit(*p))
            goto out;

        p++;
        while(isdigit(*p))
            p++;
    }

    if(*p == 'E' || *p == 'e') {
        p++;
        if(*p == '+' || *p == '-')
            p++;

        if(!isdigit(*p))
            goto out;

        p++;
        while(isdigit(*p))
            p++;
    }

    lex->token = TOKEN_REAL;

    lex->value.real = strtod(lex->start, &end);
    assert(end == p);

out:
    lex->input = p;
}

static int lex_scan(lex_t *lex)
{
    char c;

    if(lex->token == TOKEN_STRING) {
      free(lex->value.string);
      lex->value.string = NULL;
    }

    c = *lex->input;
    while(c == ' ' || c == '\t' || c == '\n' || c == '\r')
    {
        if(c == '\n')
            lex->line++;

        lex->input++;
        c = *lex->input;
    }

    lex->start = lex->input;
    c = *lex->input;

    if(c == '\0')
        lex->token = TOKEN_EOF;

    else if(c == '{' || c == '}' || c == '[' || c == ']' ||
            c == ':' || c == ',')
    {
        lex->token = c;
        lex->input++;
    }

    else if(c == '"')
        lex_scan_string(lex);

    else if(isdigit(c) || c == '-')
        lex_scan_number(lex);

    else if(isupper(c) || islower(c)) {
        /* eat up the whole identifier for clearer error messages */
        int len;

        while(isupper(*lex->input) || islower(*lex->input))
            lex->input++;
        len = lex->input - lex->start;

        if(strncmp(lex->start, "true", len) == 0)
            lex->token = TOKEN_TRUE;
        else if(strncmp(lex->start, "false", len) == 0)
            lex->token = TOKEN_FALSE;
        else if(strncmp(lex->start, "null", len) == 0)
            lex->token = TOKEN_NULL;
        else
            lex->token = TOKEN_INVALID;
    }

    else {
        lex->token = TOKEN_INVALID;
        lex->input++;
    }

    return lex->token;
}

static int lex_init(lex_t *lex, const char *input)
{
    lex->input = input;
    lex->token = TOKEN_INVALID;
    lex->line = 1;

    lex_scan(lex);
    return 0;
}

static void lex_close(lex_t *lex)
{
    if(lex->token == TOKEN_STRING)
        free(lex->value.string);
}


/*** parser ***/

static json_t *parse(lex_t *lex, json_error_t *error);

static json_t *parse_object(lex_t *lex, json_error_t *error)
{
    json_t *object = json_object();
    if(!object)
        return NULL;

    lex_scan(lex);
    if(lex->token == '}')
        return object;

    while(lex->token) {
        char *key;
        json_t *value;

        if(lex->token != TOKEN_STRING) {
            error_set(error, lex, "string expected");
            goto error;
        }

        key = strdup(lex->value.string);
        if(!key)
            return NULL;

        lex_scan(lex);
        if(lex->token != ':') {
            free(key);
            error_set(error, lex, "':' expected");
            goto error;
        }

        lex_scan(lex);

        value = parse(lex, error);
        if(!value) {
            free(key);
            goto error;
        }

        if(json_object_set(object, key, value)) {
            free(key);
            json_decref(value);
            goto error;
        }

        json_decref(value);
        free(key);

        if(lex->token != ',')
            break;

        lex_scan(lex);
    }

    if(lex->token != '}') {
        error_set(error, lex, "'}' expected");
        goto error;
    }

    return object;

error:
    json_decref(object);
    return NULL;
}

static json_t *parse_array(lex_t *lex, json_error_t *error)
{
    json_t *array = json_array();
    if(!array)
        return NULL;

    lex_scan(lex);
    if(lex->token == ']')
        return array;

    while(lex->token) {
        json_t *elem = parse(lex, error);
        if(!elem)
            goto error;

        if(json_array_append(array, elem)) {
            json_decref(elem);
            goto error;
        }
        json_decref(elem);

        if(lex->token != ',')
            break;

        lex_scan(lex);
    }


    if(lex->token != ']') {
        error_set(error, lex, "']' expected");
        goto error;
    }

    return array;

error:
    json_decref(array);
    return NULL;
}

static json_t *parse(lex_t *lex, json_error_t *error)
{
    json_t *json;

    switch(lex->token) {
        case TOKEN_STRING: {
            json = json_string(lex->value.string);
            break;
        }

        case TOKEN_INTEGER: {
            json = json_integer(lex->value.integer);
            break;
        }

        case TOKEN_REAL: {
            json = json_real(lex->value.real);
            break;
        }

        case TOKEN_TRUE:
            json = json_true();
            break;

        case TOKEN_FALSE:
            json = json_false();
            break;

        case TOKEN_NULL:
            json = json_null();
            break;

        case '{':
          json = parse_object(lex, error);
            break;

        case '[':
            json = parse_array(lex, error);
            break;

        case TOKEN_INVALID:
            error_set(error, lex, "invalid token");
            return NULL;

        default:
            error_set(error, lex, "unexpected token");
            return NULL;
    }

    if(!json)
        return NULL;

    lex_scan(lex);
    return json;
}

json_t *json_load(const char *path, json_error_t *error)
{
    json_t *result;
    FILE *fp;

    fp = fopen(path, "r");
    if(!fp)
    {
        error_set(error, NULL, "unable to open %s: %s",
                       path, strerror(errno));
        return NULL;
    }

    result = json_loadf(fp, error);

    fclose(fp);
    return result;
}

json_t *json_loads(const char *string, json_error_t *error)
{
    lex_t lex;
    json_t *result = NULL;

    if(lex_init(&lex, string))
        return NULL;

    if(lex.token != '[' && lex.token != '{') {
        error_set(error, &lex, "'[' or '{' expected");
        goto out;
    }

    result = parse(&lex, error);
    if(!result)
        goto out;

    if(lex.token != TOKEN_EOF) {
        error_set(error, &lex, "end of file expected");
        json_decref(result);
        result = NULL;
    }

out:
    lex_close(&lex);
    return result;
}

#define BUFFER_SIZE 4096

json_t *json_loadf(FILE *input, json_error_t *error)
{
    strbuffer_t strbuff;
    char buffer[BUFFER_SIZE];
    size_t length;
    json_t *result = NULL;

    if(strbuffer_init(&strbuff))
      return NULL;

    while(1)
    {
        length = fread(buffer, 1, BUFFER_SIZE, input);
        if(length == 0)
        {
            if(ferror(input))
            {
                error_set(error, NULL, "read error");
                goto out;
            }
            break;
        }
        if(strbuffer_append_bytes(&strbuff, buffer, length))
            goto out;
    }

    result = json_loads(strbuffer_value(&strbuff), error);

out:
    strbuffer_close(&strbuff);
    return result;
}

json_t *json_loadfd(int fd, json_error_t *error)
{
    strbuffer_t strbuff;
    char buffer[BUFFER_SIZE];
    ssize_t length;
    json_t *result = NULL;

    if(strbuffer_init(&strbuff))
      return NULL;

    while(1)
    {
        length = read(fd, buffer, BUFFER_SIZE);
        if(length == -1)
        {
            error_set(error, NULL, "read error: %s", strerror(errno));
            goto out;
        }
        else if(length == 0)
            break;

        if(strbuffer_append_bytes(&strbuff, buffer, length))
        {
            error_set(error, NULL, "error allocating memory");
            goto out;
        }
    }

    result = json_loads(strbuffer_value(&strbuff), error);

out:
    strbuffer_close(&strbuff);
    return result;
}
