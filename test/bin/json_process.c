/*
 * Copyright (c) 2009-2013 Petri Lehtinen <petri@digip.org>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <jansson.h>

#ifdef HAVE_LOCALE_H
#include <locale.h>
 #endif

#if _WIN32
#include <io.h>  /* for _setmode() */
#include <fcntl.h>  /* for _O_BINARY */

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
} conf;

#define l_isspace(c) ((c) == ' ' || (c) == '\n' || (c) == '\r' || (c) == '\t')

/* Return a pointer to the first non-whitespace character of str.
   Modifies str so that all trailing whitespace characters are
   replaced by '\0'. */
static const char *strip(char *str)
{
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


static char *loadfile(FILE *file)
{
    long fsize, ret;
    char *buf;
    
    fseek(file, 0, SEEK_END);
    fsize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    buf = malloc(fsize+1);
    ret = fread(buf, 1, fsize, file);
    if (ret != fsize)
        exit(1);
    buf[fsize] = '\0';
    
    return buf;
}


static void read_conf(FILE *conffile)
{
    char *buffer, *line, *val;
    
    buffer = loadfile(conffile);
    line = strtok(buffer, "\r\n");
    while (line) {
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
        if (!strcmp(line, "STRIP"))
            conf.strip = atoi(val);
        
        line = strtok(NULL, "\r\n");
    }
    
    free(buffer);
}


static int cmpfile(const char *str, const char *path, const char *fname)
{
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
    if (strcmp(buffer, str) != 0)
        ret = 1;
    else
        ret = 0;
    free(buffer);
    fclose(file);
    
    return ret;
}


int main(int argc, char *argv[])
{
    int ret;
    size_t flags = 0;
    char filename[1024], errstr[1024];
    char *buffer;
    FILE *infile, *conffile;
    json_t *json;
    json_error_t error;

    #ifdef HAVE_SETLOCALE
    setlocale(LC_ALL, "");
    #endif

    if (argc != 2) {
        fprintf(stderr, "usage: %s test_dir\n", argv[0]);
        return 2;
    }
    
    sprintf(filename, "%s%cinput", argv[1], dir_sep);
    if (!(infile = fopen(filename, "rb"))) {
        fprintf(stderr, "Could not open \"%s\"\n", filename);
        return 2;
    }
    
    sprintf(filename, "%s%cenv", argv[1], dir_sep);
    conffile = fopen(filename, "rb");
    if (conffile) {
        read_conf(conffile);
        fclose(conffile);
    }

    if (conf.indent < 0 || conf.indent > 255) {
        fprintf(stderr, "invalid value for JSON_INDENT: %d\n", conf.indent);
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

    if (conf.strip) {
        /* Load to memory, strip leading and trailing whitespace */
        buffer = loadfile(infile);
        json = json_loads(strip(buffer), 0, &error);
        free(buffer);
    }
    else
        json = json_loadf(infile, 0, &error);

    fclose(infile);
    
    if (!json) {
        sprintf(errstr, "%d %d %d\n%s\n",
                error.line, error.column, error.position,
                error.text);
        
        ret = cmpfile(errstr, argv[1], "error");
        return ret;
    }

    buffer = json_dumps(json, flags);
    ret = cmpfile(buffer, argv[1], "output");
    free(buffer);
    json_decref(json);

    return ret;
}
