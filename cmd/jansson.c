/*
 * Copyright (c) 2017 Red Hat, Inc.
 * Author: Nathaniel McCallum <npmccallum@redhat.com>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <jansson.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>

typedef bool (func_t)(json_t *stk, const char *arg,
                      json_t *cur, json_t *lst, bool *not);

static bool
cmd_not(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    (void) stk;
    (void) arg;
    (void) cur;
    (void) lst;
    *not = true;
    return true;
}

static bool
cmd_object(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;
    (void) lst;

    ret = *not ^ json_is_object(cur);
    *not = false;
    return ret;
}

static bool
cmd_array(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;
    (void) lst;

    ret = *not ^ json_is_array(cur);
    *not = false;
    return ret;
}

static bool
cmd_string(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;
    (void) lst;

    ret = *not ^ json_is_string(cur);
    *not = false;
    return ret;
}

static bool
cmd_integer(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;
    (void) lst;

    ret = *not ^ json_is_integer(cur);
    *not = false;
    return ret;
}

static bool
cmd_real(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;
    (void) lst;

    ret = *not ^ json_is_real(cur);
    *not = false;
    return ret;
}

static bool
cmd_number(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;
    (void) lst;

    ret = *not ^ json_is_number(cur);
    *not = false;
    return ret;
}

static bool
cmd_true(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;
    (void) lst;

    ret = *not ^ json_is_true(cur);
    *not = false;
    return ret;
}

static bool
cmd_false(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;
    (void) lst;

    ret = *not ^ json_is_false(cur);
    *not = false;
    return ret;
}

static bool
cmd_boolean(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;
    (void) lst;

    ret = *not ^ json_is_boolean(cur);
    *not = false;
    return ret;
}

static bool
cmd_null(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;
    (void) lst;

    ret = *not ^ json_is_null(cur);
    *not = false;
    return ret;
}

static bool
cmd_equal(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    bool ret;
    (void) stk;
    (void) arg;

    ret = *not ^ json_equal(lst, cur);
    *not = false;
    return ret;
}

static bool
cmd_input(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    const int flags = JSON_DISABLE_EOF_CHECK | JSON_DECODE_ANY;
    json_auto_t *v = NULL;
    (void) cur;
    (void) lst;
    (void) not;

    v = json_loads(arg, JSON_DECODE_ANY, NULL);
    if (!v) {
        FILE *file = NULL;

        if (strcmp(arg, "-") == 0)
            file = fdopen(dup(STDIN_FILENO), "r");
        else
            file = fopen(arg, "r");
        if (!file)
            return false;

        v = json_loadf(file, flags, NULL);
        fclose(file);
    }

    return json_array_insert(stk, 0, v) >= 0;
}

static bool
cmd_output(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    const int wflags = JSON_ENCODE_ANY | JSON_COMPACT | JSON_SORT_KEYS;
    FILE *file = NULL;
    (void) stk;
    (void) lst;
    (void) not;

    if (strcmp("-", arg) == 0)
        file = fdopen(dup(STDOUT_FILENO), "w");
    else
        file = fopen(arg, "w");
    if (!file)
        return false;

    if (json_dumpf(cur, file, wflags) < 0)
        goto error;

    if (isatty(fileno(file)) && fwrite("\n", 1, 1, file) != 1)
        goto error;

    fclose(file);
    return true;

error:
    fclose(file);
    return false;
}

static bool
cmd_list(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    const int wflags = JSON_ENCODE_ANY | JSON_COMPACT | JSON_SORT_KEYS;
    FILE *file = NULL;
    (void) stk;
    (void) lst;
    (void) not;

    if (!json_is_array(cur) && !json_is_object(cur))
        return false;

    if (strcmp("-", arg) == 0)
        file = fdopen(dup(STDOUT_FILENO), "w");
    else
        file = fopen(arg, "w");
    if (!file)
        return false;

    if (json_is_array(cur)) {
        json_t *v = NULL;
        size_t i = 0;

        json_array_foreach(cur, i, v) {
            if (json_dumpf(v, file, wflags) < 0 ||
                fprintf(file, "\n") < 0)
                goto error;
        }
    } else if (json_is_object(cur)) {
        const char *k = NULL;
        json_t *v = NULL;

        json_object_foreach(cur, k, v) {
            if (fprintf(file, "%s=", k) < 0 ||
                json_dumpf(v, file, wflags) < 0 ||
                fprintf(file, "\n") < 0)
                goto error;
        }
    }

    fclose(file);
    return true;

error:
    fclose(file);
    return false;
}

static bool
cmd_unquote(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    FILE *file = NULL;
    (void) stk;
    (void) lst;
    (void) not;

    if (!json_is_string(cur))
        return false;

    if (strcmp("-", arg) == 0)
        file = fdopen(dup(STDOUT_FILENO), "w");
    else
        file = fopen(arg, "w");
    if (!file)
        return false;

    if (fprintf(file, "%s\n", json_string_value(cur)) < 0) {
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

static bool
cmd_copy(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    (void) arg;
    (void) lst;
    (void) not;
    return json_array_insert_new(stk, 0, json_deep_copy(cur)) >= 0;
}

static bool
cmd_stack(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    (void) arg;
    (void) cur;
    (void) lst;
    (void) not;
    return json_array_insert_new(stk, 0, json_deep_copy(stk)) >= 0;
}

static bool
cmd_move(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    size_t off = 0;
    (void) lst;
    (void) not;

    if (sscanf(arg, "%zu", &off) != 1)
        return false;

    if (json_array_insert(stk, off + 1, cur) < 0)
        return false;

    if (json_array_remove(stk, 0) < 0)
        return false;

    return true;
}

static bool
cmd_pop(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    (void) arg;
    (void) cur;
    (void) lst;
    (void) not;
    return json_array_remove(stk, 0) >= 0;
}

static size_t
convert_int(const json_t *arr, const char *arg)
{
    ssize_t indx = 0;

    if (sscanf(arg, "%zd", &indx) != 1)
        return SIZE_MAX;

    if (indx < 0)
        indx += json_array_size(arr);

    if (indx < 0)
        return SIZE_MAX;

    return indx;
}

static bool
cmd_trunc(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    size_t num;
    size_t s;
    (void) stk;
    (void) lst;
    (void) not;

    num = convert_int(cur, arg);
    if (num == SIZE_MAX)
        return false;

    for (s = json_array_size(cur); s > num; s--) {
        if (json_array_remove(cur, s - 1) < 0)
            return false;
    }

    return true;
}

static bool
cmd_insert(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    size_t indx;
    (void) stk;
    (void) not;

    indx = convert_int(cur, arg);
    if (indx == SIZE_MAX)
        return false;

    return json_array_insert(lst, indx, cur) >= 0;
}

static bool
cmd_append(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    (void) stk;
    (void) arg;
    (void) not;

    if (json_is_array(lst))
        return json_array_append(lst, cur) >= 0;

    if (json_is_object(lst))
        return json_object_update_missing(lst, cur) >= 0;

    return false;
}

static bool
cmd_extend(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    (void) stk;
    (void) arg;
    (void) not;

    if (json_is_array(lst))
        return json_array_extend(lst, cur) >= 0;

    if (json_is_object(lst))
        return json_object_update(lst, cur) >= 0;

    return false;
}

static bool
cmd_delete(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    (void) stk;
    (void) lst;
    (void) not;

    if (json_is_array(cur)) {
        size_t indx;

        indx = convert_int(cur, arg);
        if (indx == SIZE_MAX)
            return false;

        return json_array_remove(cur, indx) >= 0;
    }

    if (json_is_object(cur))
        return json_object_del(cur, arg) >= 0;

    return false;
}

static bool
cmd_count(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    size_t count = 0;
    (void) arg;
    (void) lst;
    (void) not;

    if (json_is_array(cur))
        count = json_array_size(cur);
    else if (json_is_object(cur))
        count = json_object_size(cur);
    else if (json_is_string(cur))
        count = json_string_length(cur);
    else
        return false;

    return json_array_insert_new(stk, 0, json_integer(count)) >= 0;
}

static bool
cmd_empty(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    (void) stk;
    (void) arg;
    (void) lst;
    (void) not;

    if (json_is_array(cur))
        return json_array_clear(cur) >= 0;

    if (json_is_object(cur))
        return json_object_clear(cur) >= 0;

    return false;
}

static bool
cmd_get(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    json_t *v = NULL;
    (void) lst;
    (void) not;

    if (json_is_array(cur)) {
        size_t indx;

        indx = convert_int(cur, arg);
        if (indx == SIZE_MAX)
            return false;

        v = json_array_get(cur, indx);
    } else if (json_is_object(cur)) {
        v = json_object_get(cur, arg);
    } else {
        return false;
    }

    return json_array_insert(stk, 0, v) >= 0;
}

static bool
cmd_set(json_t *stk, const char *arg, json_t *cur, json_t *lst, bool *not)
{
    (void) stk;
    (void) not;

    if (json_is_array(lst)) {
        size_t indx;

        indx = convert_int(lst, arg);
        if (indx == SIZE_MAX)
            return false;

        return json_array_set(lst, indx, cur) >= 0;
    }

    if (json_is_object(lst))
        return json_object_set(lst, arg, cur) >= 0;

    return false;
}

static const struct {
    int val;
    const char *arg;
    func_t *func;
    const char *desc;
} descs[] = {
    { 'X', NULL,   cmd_not,     "Invert the following assertion" },
    { 'O', NULL,   cmd_object,  "Assert TOP to be an object" },
    { 'A', NULL,   cmd_array,   "Assert TOP to be an array" },
    { 'S', NULL,   cmd_string,  "Assert TOP to be a string" },
    { 'I', NULL,   cmd_integer, "Assert TOP to be an integer" },
    { 'R', NULL,   cmd_real,    "Assert TOP to be a real" },
    { 'N', NULL,   cmd_number,  "Assert TOP to be a number" },
    { 'T', NULL,   cmd_true,    "Assert TOP to be true" },
    { 'F', NULL,   cmd_false,   "Assert TOP to be false" },
    { 'B', NULL,   cmd_boolean, "Assert TOP to be a boolean" },
    { '0', NULL,   cmd_null,    "Assert TOP to be null" },
    { 'E', NULL,   cmd_equal,   "Assert TOP to be equal to PREV" },

    { 'i', "JSON", cmd_input,   "Parse JSON constant, push onto TOP" },
    { 'i', "FILE", cmd_input,   "Read from FILE, push onto TOP" },
    { 'i', "-",    cmd_input,   "Read from STDIN, push onto TOP" },
    { 'o', "FILE", cmd_output,  "Write TOP to FILE" },
    { 'o', "-",    cmd_output,  "Write TOP to STDOUT" },
    { 'l', "FILE", cmd_list,    "Write TOP (obj./arr.) to FILE, one line/item" },
    { 'l', "-",    cmd_list,    "Write TOP (obj./arr.) to STDOUT, one line/item" },
    { 'u', "FILE", cmd_unquote, "Write TOP (str.) to FILE without quotes" },
    { 'u', "-",    cmd_unquote, "Write TOP (str.) to STDOUT without quotes" },

    { 'z', NULL,   cmd_copy,    "Deep copy TOP, push onto TOP" },
    { 'q', NULL,   cmd_stack,   "Deep copy the current stack, push onto TOP" },
    { 'm', "#",    cmd_move,    "Move TOP back # places on the stack" },
    { 'p', NULL,   cmd_pop,     "Discard TOP from the stack" },

    { 't', "#",    cmd_trunc,   "Shrink TOP (arr.) to length #" },
    { 't', "-#",   cmd_trunc,   "Discard last # items from TOP (arr.)" },
    { 'n', "#",    cmd_insert,  "Insert TOP into PREV (arr.) at #" },
    { 'a', NULL,   cmd_append,  "Append TOP to the end of PREV (arr.)" },
    { 'a', NULL,   cmd_append,  "Set missing values from TOP (obj.) into PREV (obj.)" },
    { 'x', NULL,   cmd_extend,  "Append items from TOP to the end of PREV (arr.)" },
    { 'x', NULL,   cmd_extend,  "Set all values from TOP (obj.) into PREV (obj.)" },

    { 'd', "NAME", cmd_delete,  "Delete NAME from TOP (obj.)" },
    { 'd', "#",    cmd_delete,  "Delete # from TOP (arr.)" },
    { 'c', NULL,   cmd_count,   "Push length of TOP (arr./str./obj.) to TOP" },
    { 'e', NULL,   cmd_empty,   "Erase all items from TOP (arr./obj.)" },
    { 'g', "NAME", cmd_get,     "Push item with NAME from TOP (obj.) to TOP" },
    { 'g', "#",    cmd_get,     "Push item # from TOP (arr.) to TOP" },
    { 's', "NAME", cmd_set,     "Sets TOP into PREV (obj.) with NAME" },
    { 's', "#",    cmd_set,     "Sets TOP into PREV (obj.) at #" },

    { 'h', NULL,   NULL,        "Display this help" },
    { 0, NULL, NULL, NULL }
};

static const struct option lopts[] = {
    { "not",      no_argument,       .val = 'X' },
    { "object",   no_argument,       .val = 'O' },
    { "array",    no_argument,       .val = 'A' },
    { "string",   no_argument,       .val = 'S' },
    { "integer",  no_argument,       .val = 'I' },
    { "real",     no_argument,       .val = 'R' },
    { "number",   no_argument,       .val = 'N' },
    { "true",     no_argument,       .val = 'T' },
    { "false",    no_argument,       .val = 'F' },
    { "boolean",  no_argument,       .val = 'B' },
    { "null",     no_argument,       .val = '0' },
    { "equal",    no_argument,       .val = 'E' },

    { "input",    required_argument, .val = 'i' },
    { "output",   required_argument, .val = 'o' },
    { "list",     required_argument, .val = 'l' },
    { "unquote",  required_argument, .val = 'u' },

    { "copy",     no_argument,       .val = 'z' },
    { "stack",    no_argument,       .val = 'q' },
    { "move",     required_argument, .val = 'm' },
    { "pop",      no_argument,       .val = 'p' },

    { "truncate", required_argument, .val = 't' },
    { "insert",   required_argument, .val = 'n' },
    { "append",   no_argument,       .val = 'a' },
    { "extend",   no_argument,       .val = 'x' },

    { "delete",   required_argument, .val = 'd' },
    { "count",    no_argument,       .val = 'c' },
    { "empty",    no_argument,       .val = 'e' },
    { "get",      required_argument, .val = 'g' },
    { "set",      required_argument, .val = 's' },

    { "help",     no_argument,       .val = 'h' },
    { NULL, 0, NULL, 0 }
};

static char sopts[sizeof(lopts) / sizeof(*lopts) * 2 + 2];

static int
getopts(int argc, char *const argv[])
{
    size_t i;

    if (!sopts[0]) {
        for (i = 0; lopts[i].name; i++) {
            char c = lopts[i].val;

            if (!isalnum(lopts[i].val))
                continue;

            strncat(sopts, &c, 1);

            switch (lopts[i].has_arg) {
            case optional_argument: /* fallthrough */
            case required_argument: strcat(sopts, ":"); break;
            default:  break;
            }
        }
    }

    return getopt_long(argc, argv, sopts, lopts, NULL);
}

int
main(int argc, char *argv[])
{
    json_auto_t *stk = NULL;
    unsigned char ret = 0;
    bool not = false;
    int amax = 0;
    int nmax = 0;
    size_t i;
    size_t j;
    int opt;

    json_object_seed(0);

    stk = json_array();
    if (!stk)
        return -1;

    for (opt = getopts(argc, argv); opt != -1; opt = getopts(argc, argv)) {
        json_t *lst = NULL;
        json_t *cur = NULL;
        bool ok = false;

        if (not && !strchr("OASIRNTFB0E", opt))
            return ret;

        cur = json_array_get(stk, 0);
        lst = json_array_get(stk, 1);
        ret++;

        for (i = 0; descs[i].desc; i++) {
            if (descs[i].val == opt) {
                if (!descs[i].func)
                    goto usage;

                ok = descs[i].func(stk, optarg, cur, lst, &not);
                break;
            }
        }

        if (!ok)
            return ret;
    }

    if (not)
        return ret;

    if (ret > 0)
        return EXIT_SUCCESS;

usage:
    fprintf(stderr, "Usage: jansson [OPTIONS]"
"\n"
"\nThis program provides a mechanism for building and parsing JSON objects"
"\nfrom the command line. It operates as a simple stack machine. All commands"
"\noperate on the TOP item of the stack, and occasionally the PREV item of the"
"\nstack, unless otherwise specified. Commands that require a specific type of"
"\nvalue will indicate it in parentheses. For example: \"TOP (arr.)\"."
"\n"
"\nThis program returns 0 on success or the index of the option which failed."
"\n"
"\n"
);

    for (i = 0; lopts[i].name; i++) {
        for (j = 0; descs[j].desc; j++) {
            int x = 0;

            if (descs[j].val != lopts[i].val)
                continue;

            if (descs[j].arg)
                x = strlen(descs[j].arg);
            if (x > amax)
                amax = x;

            if (lopts[i].name)
                x = strlen(lopts[i].name);
            if (x > nmax)
                nmax = x;
        }
    }

    for (i = 0; lopts[i].name; i++) {
        for (j = 0; descs[j].desc; j++) {
            int n = 0;

            if (descs[j].val != lopts[i].val)
                continue;

            if (lopts[i].name)
                n = strlen(lopts[i].name);

            if (descs[j].arg) {
                int a = strlen(descs[j].arg);
                fprintf(stderr, "-%c %s,%*s --%s=%-*s %s\n",
                        lopts[i].val, descs[j].arg, amax - a, "",
                        lopts[i].name, amax + nmax - n, descs[j].arg,
                        descs[j].desc);
            } else {
                fprintf(stderr, "-%c, %-*s --%s %-*s %s\n",
                        lopts[i].val, amax, "",
                        lopts[i].name, amax + nmax - n, "",
                        descs[j].desc);
            }
        }
    }

    fprintf(stderr, "\n");
    return -2;
}
