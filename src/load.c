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


#define JSON_TOKEN_INVALID         -1
#define JSON_TOKEN_EOF              0
#define JSON_TOKEN_STRING         256
#define JSON_TOKEN_NUMBER         257
#define JSON_TOKEN_TRUE           258
#define JSON_TOKEN_FALSE          259
#define JSON_TOKEN_NULL           260

typedef struct {
    const char *input;
    const char *start;
    int token;
    int line, column;
    union {
        char *string;
        double number;
    } value;
} json_lex;


/*** error reporting ***/

static void json_set_error(json_error_t *error, const json_lex *lex,
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
        snprintf(error->text, JSON_ERROR_TEXT_LENGTH, "%s", msg);
}


/*** lexical analyzer ***/

static void json_scan_string(json_lex *lex)
{
    /* skip the " */
    const char *p = lex->input + 1;
    char *t;

    lex->token = JSON_TOKEN_INVALID;

    while(*p != '"') {
        if(*p == '\0') {
            /* unterminated string literal */
            goto out;
        }

        if(0 <= *p && *p <= 31) {
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
            if(*p == '"' || *p == '\\' || *p == '/' || *p == 'b' ||
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
    lex->token = JSON_TOKEN_STRING;

out:
    lex->input = p;
}

static void json_scan_number(json_lex *lex)
{
    const char *p = lex->input;
    char *end;

    lex->token = JSON_TOKEN_INVALID;

    if(*p == '-')
        p++;

    if(*p == '0')
        p++;
    else /* *p != '0' */ {
        p++;
        while(isdigit(*p))
            p++;
    }

    if(*p == '.') {
        p++;
        if(!isdigit(*(p++)))
            goto out;

        while(isdigit(*p))
            p++;
    }

    if(*p == 'E' || *p == 'e') {
        p++;
        if(*p == '+' || *p == '-')
            p++;

        if(!isdigit(*(p++)))
            goto out;

        while(isdigit(*p))
            p++;
    }

    lex->token = JSON_TOKEN_NUMBER;

    lex->value.number = strtod(lex->start, &end);
    assert(end == p);

out:
    lex->input = p;
}

static int json_lex_scan(json_lex *lex)
{
    char c;

    if(lex->token == JSON_TOKEN_STRING) {
      free(lex->value.string);
      lex->value.string = NULL;
    }

    while(isspace(*lex->input)) {
        if(*lex->input == '\n')
            lex->line++;

        lex->input++;
    }

    lex->start = lex->input;
    c = *lex->input;

    if(c == '\0')
        lex->token = JSON_TOKEN_EOF;

    else if(c == '{' || c == '}' || c == '[' || c == ']' ||
            c == ':' || c == ',') {
        lex->token = c;
        lex->input++;
    }

    else if(c == '"')
        json_scan_string(lex);

    else if(isdigit(c) || c == '-')
        json_scan_number(lex);

    else if(isalpha(c)) {
        /* eat up the whole identifier for clearer error messages */
        int len;

        while(isalpha(*lex->input))
            lex->input++;
        len = lex->input - lex->start;

        if(strncmp(lex->start, "true", len) == 0)
            lex->token = JSON_TOKEN_TRUE;
        else if(strncmp(lex->start, "false", len) == 0)
            lex->token = JSON_TOKEN_FALSE;
        else if(strncmp(lex->start, "null", len) == 0)
            lex->token = JSON_TOKEN_NULL;
        else
            lex->token = JSON_TOKEN_INVALID;
    }

    else {
        lex->token = JSON_TOKEN_INVALID;
        lex->input++;
    }

    return lex->token;
}

static int json_lex_init(json_lex *lex, const char *input)
{
    lex->input = input;
    lex->token = JSON_TOKEN_INVALID;
    lex->line = 1;

    json_lex_scan(lex);
    return 0;
}

static void json_lex_close(json_lex *lex)
{
    if(lex->token == JSON_TOKEN_STRING)
        free(lex->value.string);
}


/*** parser ***/

static json_t *json_parse(json_lex *lex, json_error_t *error);

static json_t *json_parse_object(json_lex *lex, json_error_t *error)
{
    json_t *object = json_object();
    if(!object)
        return NULL;

    json_lex_scan(lex);
    if(lex->token == '}')
        return object;

    while(lex->token) {
        char *key;
        json_t *value;

        if(lex->token != JSON_TOKEN_STRING) {
            json_set_error(error, lex, "string expected");
            goto error;
        }

        key = strdup(lex->value.string);
        if(!key)
            return NULL;

        json_lex_scan(lex);
        if(lex->token != ':') {
            json_set_error(error, lex, "':' expected");
            goto error;
        }

        json_lex_scan(lex);

        value = json_parse(lex, error);
        if(!value)
            goto error;

        if(json_object_set(object, key, value)) {
            json_decref(value);
            goto error;
        }

        json_decref(value);
        free(key);

        if(lex->token != ',')
            break;

        json_lex_scan(lex);
    }

    if(lex->token != '}') {
        json_set_error(error, lex, "'}' expected");
        goto error;
    }

    return object;

error:
    json_decref(object);
    return NULL;
}

static json_t *json_parse_array(json_lex *lex, json_error_t *error)
{
    json_t *array = json_array();
    if(!array)
        return NULL;

    json_lex_scan(lex);
    if(lex->token != ']') {
        while(1) {
            json_t *elem = json_parse(lex, error);
            if(!elem)
                goto error;

            if(json_array_append(array, elem)) {
                json_decref(elem);
                goto error;
            }
            json_decref(elem);

            if(lex->token != ',')
                break;

            json_lex_scan(lex);
        }
    }

    if(lex->token != ']') {
        json_set_error(error, lex, "']' expected");
        goto error;
    }

    return array;

error:
    json_decref(array);
    return NULL;
}

static json_t *json_parse(json_lex *lex, json_error_t *error)
{
    json_t *json;

    switch(lex->token) {
        case JSON_TOKEN_STRING: {
            json = json_string(lex->value.string);
            break;
        }

        case JSON_TOKEN_NUMBER: {
            json = json_number(lex->value.number);
            break;
        }

        case JSON_TOKEN_TRUE:
            json = json_true();
            break;

        case JSON_TOKEN_FALSE:
            json = json_false();
            break;

        case JSON_TOKEN_NULL:
            json = json_null();
            break;

        case '{':
          json = json_parse_object(lex, error);
            break;

        case '[':
            json = json_parse_array(lex, error);
            break;

        case JSON_TOKEN_INVALID:
            json_set_error(error, lex, "invalid token");
            return NULL;

        default:
            json_set_error(error, lex, "unexpected token");
            return NULL;
    }

    if(!json)
        return NULL;

    json_lex_scan(lex);
    return json;
}

json_t *json_load(const char *path, json_error_t *error)
{
    json_t *result;
    FILE *fp;

    fp = fopen(path, "r");
    if(!fp)
    {
        json_set_error(error, NULL, "unable to open %s: %s",
                       path, strerror(errno));
        return NULL;
    }

    result = json_loadf(fp, error);

    fclose(fp);
    return result;
}

json_t *json_loads(const char *string, json_error_t *error)
{
    json_lex lex;
    json_t *result = NULL;

    if(json_lex_init(&lex, string))
        return NULL;

    if(lex.token != '[' && lex.token != '{') {
        json_set_error(error, &lex, "'[' or '{' expected");
        goto out;
    }

    result = json_parse(&lex, error);
    if(!result)
        goto out;

    if(lex.token != JSON_TOKEN_EOF) {
        json_set_error(error, &lex, "end of file expected");
        json_decref(result);
        result = NULL;
    }

out:
    json_lex_close(&lex);
    return result;
}

#define BUFFER_SIZE 4096

json_t *json_loadf(FILE *input, json_error_t *error)
{
    strbuffer_t strbuff;
    char buffer[BUFFER_SIZE];
    size_t length;
    json_t *result = NULL;

    strbuffer_init(&strbuff);

    while(1)
    {
        length = fread(buffer, 1, BUFFER_SIZE, input);
        if(length == 0)
        {
            if(ferror(input))
            {
                json_set_error(error, NULL, "read error");
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

    strbuffer_init(&strbuff);

    while(1)
    {
        length = read(fd, buffer, BUFFER_SIZE);
        if(length == -1)
        {
            json_set_error(error, NULL, "read error: %s", strerror(errno));
            goto out;
        }
        else if(length == 0)
            break;

        if(strbuffer_append_bytes(&strbuff, buffer, length))
        {
            json_set_error(error, NULL, "error allocating memory");
            goto out;
        }
    }

    result = json_loads(strbuffer_value(&strbuff), error);

out:
    strbuffer_close(&strbuff);
    return result;
}
