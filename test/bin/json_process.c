/*
 * Copyright (c) 2009-2016 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef HAVE_CONFIG_H
#include <jansson_private_config.h>
#endif

#include <ctype.h>
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#if _WIN32
#include <fcntl.h> /* for _O_BINARY */
#include <io.h>    /* for _setmode() */

static const char dir_sep = '\\';
#else
static const char dir_sep = '/';
#endif

struct config {
    int indent;
    int compact;
    int preserve_order;
    int ensure_ascii;
    int sort_keys;
    int strip;
    int have_hashseed;
    int hashseed;
    int precision;
} conf;

#define l_isspace(c) ((c) == ' ' || (c) == '\n' || (c) == '\r' || (c) == '\t')

/* Return a pointer to the first non-whitespace character of str.
   Modifies str so that all trailing whitespace characters are
   replaced by '\0'. */
static const char *strip(char *str) {
    size_t length;
    char *result = str;
    while (*result && l_isspace(*result))
        result++;

    length = strlen(result);
    if (length == 0)
        return result;

    while (l_isspace(result[length - 1]))
        result[--length] = '\0';

    return result;
}

static char *loadfile(FILE *file) {
    size_t fsize, ret;
    char *buf;

    fseek(file, 0, SEEK_END);
    fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    buf = malloc(fsize + 1);
    ret = fread(buf, 1, fsize, file);
    if (ret != fsize)
        exit(1);
    buf[fsize] = '\0';

    return buf;
}

static void read_conf(FILE *conffile) {
    char *buffer, *line, *val;
    conf.have_hashseed = 0;

    buffer = loadfile(conffile);
    for (line = strtok(buffer, "\r\n"); line; line = strtok(NULL, "\r\n")) {
        val = strchr(line, '=');
        if (!val) {
            printf("invalid configuration line\n");
            break;
        }
        *val++ = '\0';

        if (!strcmp(line, "JSON_INDENT"))
            conf.indent = atoi(val);
        if (!strcmp(line, "JSON_COMPACT"))
            conf.compact = atoi(val);
        if (!strcmp(line, "JSON_ENSURE_ASCII"))
            conf.ensure_ascii = atoi(val);
        if (!strcmp(line, "JSON_PRESERVE_ORDER"))
            conf.preserve_order = atoi(val);
        if (!strcmp(line, "JSON_SORT_KEYS"))
            conf.sort_keys = atoi(val);
        if (!strcmp(line, "JSON_REAL_PRECISION"))
            conf.precision = atoi(val);
        if (!strcmp(line, "STRIP"))
            conf.strip = atoi(val);
        if (!strcmp(line, "HASHSEED")) {
            conf.have_hashseed = 1;
            conf.hashseed = atoi(val);
        }
    }

    free(buffer);
}

static int cmpfile(const char *str, const char *path, const char *fname) {
    char filename[1024], *buffer;
    int ret;
    FILE *file;

    sprintf(filename, "%s%c%s", path, dir_sep, fname);
    file = fopen(filename, "rb");
    if (!file) {
        if (conf.strip)
            strcat(filename, ".strip");
        else
            strcat(filename, ".normal");
        file = fopen(filename, "rb");
    }
    if (!file) {
        printf("Error: test result file could not be opened.\n");
        exit(1);
    }

    buffer = loadfile(file);
    if (strcmp(buffer, str) != 0) {
        fprintf(stderr, "=== Expected %s ===\n", fname);
        fprintf(stderr, "%s\n", buffer);
        fprintf(stderr, "=== Actual %s ===\n", fname);
        fprintf(stderr, "%s\n", str);
        ret = 1;
    } else {
        ret = 0;
    }

    free(buffer);
    fclose(file);

    return ret;
}

int use_conf(char *test_path) {
    int ret;
    size_t flags = 0;
    char filename[1024], errstr[1024];
    char *buffer;
    FILE *infile, *conffile;
    json_t *json;
    json_error_t error;

    sprintf(filename, "%s%cinput", test_path, dir_sep);
    if (!(infile = fopen(filename, "rb"))) {
        fprintf(stderr, "Could not open \"%s\"\n", filename);
        return 2;
    }

    sprintf(filename, "%s%cenv", test_path, dir_sep);
    conffile = fopen(filename, "rb");
    if (conffile) {
        read_conf(conffile);
        fclose(conffile);
    }

    if (conf.indent < 0 || conf.indent > 31) {
        fprintf(stderr, "invalid value for JSON_INDENT: %d\n", conf.indent);
        fclose(infile);
        return 2;
    }
    if (conf.indent)
        flags |= JSON_INDENT(conf.indent);

    if (conf.compact)
        flags |= JSON_COMPACT;

    if (conf.ensure_ascii)
        flags |= JSON_ENSURE_ASCII;

    if (conf.preserve_order)
        flags |= JSON_PRESERVE_ORDER;

    if (conf.sort_keys)
        flags |= JSON_SORT_KEYS;

    if (conf.precision < 0 || conf.precision > 31) {
        fprintf(stderr, "invalid value for JSON_REAL_PRECISION: %d\n", conf.precision);
        fclose(infile);
        return 2;
    }
    if (conf.precision)
        flags |= JSON_REAL_PRECISION(conf.precision);

    if (conf.have_hashseed)
        json_object_seed(conf.hashseed);

    if (conf.strip) {
        /* Load to memory, strip leading and trailing whitespace */
        buffer = loadfile(infile);
        json = json_loads(strip(buffer), 0, &error);
        free(buffer);
    } else {
        json = json_loadf(infile, 0, &error);
    }

    fclose(infile);

    if (!json) {
        sprintf(errstr, "%d %d %d\n%s\n", error.line, error.column, error.position,
                error.text);

        ret = cmpfile(errstr, test_path, "error");
        return ret;
    }

    buffer = json_dumps(json, flags);
    ret = cmpfile(buffer, test_path, "output");
    free(buffer);
    json_decref(json);

    return ret;
}

int main(int argc, char *argv[]) {
    int i;
    char *test_path = NULL;

#ifdef HAVE_SETLOCALE
    setlocale(LC_ALL, "");
#endif

    if (argc < 2) {
        goto usage;
    }

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--strip"))
            conf.strip = 1;
        else
            test_path = argv[i];
    }

    if (!test_path) {
        goto usage;
    }

    return use_conf(test_path);

usage:
    fprintf(stderr, "usage: %s [--strip] test_dir\n", argv[0]);
    return 2;
}
