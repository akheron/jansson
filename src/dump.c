#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <jansson.h>

typedef int (*dump_func)(const char *buffer, int size, void *data);

static int dump_to_file(const char *buffer, int size, void *data)
{
    FILE *dest = (FILE *)data;
    if(fwrite(buffer, size, 1, dest) != 1)
        return -1;
    return 0;
}

static int dump_to_fd(const char *buffer, int size, void *data)
{
    int *fd = (int *)data;
    if(write(*fd, buffer, size) != size)
        return -1;
    return 0;
}

static int dump_indent(uint32_t flags, int depth, dump_func dump, void *data)
{
    if(JSON_INDENT(flags) > 0)
    {
        char *ws_buffer;
        int ws_count = JSON_INDENT(flags) * depth;

        if(dump("\n", 1, data))
            return -1;

        if(ws_count == 0)
            return 0;

        ws_buffer = alloca(ws_count);
        memset(ws_buffer, ' ', ws_count);
        return dump(ws_buffer, ws_count, data);
    }
    else
        return dump(" ", 1, data);
}

static int dump_string(const char *str, dump_func dump, void *data)
{
    const char *end;

    if(dump("\"", 1, data))
        return -1;

    end = str;
    while(*end)
    {
        while(*end && *end != '\\' && *end != '"')
            end++;

        if(end != str)
            if(dump(str, end - str, data))
                return -1;

        if(*end == '\\')
        {
            if(dump("\\\\", 2, data))
                return -1;
            end++;
        }
        else if(*end == '"')
        {
            if(dump("\\\"", 2, data))
                return -1;
            end++;
        }
        str = end;
    }

    return dump("\"", 1, data);
}

static int do_dump(const json_t *json, uint32_t flags, int depth,
                   dump_func dump, void *data)
{
    switch(json_typeof(json)) {
        case JSON_NULL:
            return dump("null", 4, data);

        case JSON_TRUE:
            return dump("true", 4, data);

        case JSON_FALSE:
            return dump("false", 5, data);

        case JSON_NUMBER:
        {
            char *buffer;
            int size, ret;

            size = asprintf(&buffer, "%.17f", json_number_value(json));
            if(size == -1)
                return -1;

            ret = dump(buffer, size, data);
            free(buffer);
            return ret;
        }

        case JSON_STRING:
            return dump_string(json_string_value(json), dump, data);

        case JSON_ARRAY:
        {
            int i;
            int n = json_array_size(json);

            if(dump("[", 1, data))
                return -1;
            if(n == 0)
                dump("]", 1, data);
            if(dump_indent(flags, depth + 1, dump, data))
                return -1;

            for(i = 0; i < n; ++i) {
                if(do_dump(json_array_get(json, i), flags, depth + 1,
                           dump, data))
                    return -1;

                if(i < n - 1)
                {
                    if(dump(",", 1, data) ||
                       dump_indent(flags, depth + 1, dump, data))
                        return -1;
                }
                else
                {
                    if(dump_indent(flags, depth, dump, data))
                        return -1;
                }
            }
            return dump("]", 1, data);
        }

        case JSON_OBJECT:
        {
            void *iter = json_object_iter((json_t *)json);

            if(dump("{", 1, data))
                return -1;
            if(!iter)
                return dump("}", 1, data);
            if(dump_indent(flags, depth + 1, dump, data))
                return -1;

            while(iter)
            {
                void *next = json_object_iter_next((json_t *)json, iter);

                dump_string(json_object_iter_key(iter), dump, data);
                if(dump(": ", 2, data) ||
                   do_dump(json_object_iter_value(iter), flags, depth + 1,
                           dump, data))
                    return -1;

                if(next)
                {
                    if(dump(",", 1, data) ||
                       dump_indent(flags, depth + 1, dump, data))
                        return -1;
                }
                else
                {
                    if(dump_indent(flags, depth, dump, data))
                        return -1;
                }

                iter = next;
            }
            return dump("}", 1, data);
        }

        default:
            /* not reached */
            return -1;
    }
}


int json_dumpf(const json_t *json, FILE *output, uint32_t flags)
{
    if(do_dump(json, flags, 0, dump_to_file, (void *)output))
        return -1;
    return dump_to_file("\n", 1, (void *)output);
}

int json_dumpfd(const json_t *json, int fd, uint32_t flags)
{
    if(do_dump(json, flags, 0, dump_to_fd, (void *)&fd))
        return -1;
    return dump_to_fd("\n", 1, (void *)&fd);
}
