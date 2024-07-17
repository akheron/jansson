/*
 * Copyright (c) 2009-2021 Petri Lehtinen <petri@digip.org>
 * and Basile Starynkevitch <basile@starynkevitch.net>
 *
 * Jansson is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "jansson_private.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "jansson.h"
#include "strbuffer.h"
#include "utf.h"

#define MAX_INTEGER_STR_LENGTH 25
#define MAX_REAL_STR_LENGTH    25

#define FLAGS_TO_INDENT(f)    ((f)&0x1F)
#define FLAGS_TO_PRECISION(f) (((f) >> 11) & 0x1F)

struct buffer {
    const size_t size;
    size_t used;
    char *data;
};


struct attribute_flag_entry {
  const char* attr_key;
  size_t attr_flags;
};

struct attribute_flag_table_st {
  unsigned atflag_size;	/* allocated size of atflag_entries */
  unsigned atflag_count;	/* counting number of used entries */
  struct attribute_flag_entry atflag_entries[]; /* flexible array member */
};

static struct attribute_flag_table_st* attribute_flag_hashtable;

static unsigned atflag_hash(const char*atkey)
{
  /* NB: most of the decimal integer constants here are prime numbers. */
  unsigned kl = (unsigned)strlen(atkey);
  unsigned h1 = 17*kl+3;
  unsigned h2 = 0;
  unsigned h = 0;
  for (unsigned i=0; i<kl; i++) {
    if (i%2 == 0) {
      unsigned nh1 = (h1*31 + i) ^ (atkey[i]*1471 + h2);
      unsigned nh2 = h2*11 + (h1&0xfff) - atkey[i]*13;
      h1 = nh1;
      h2 = nh2;
    }
    else {
      unsigned nh1 = (h1*53 - i) ^ (atkey[i]*1181 + 7*h2);
      unsigned nh2 = h2*89 - (h1&0xffff) + atkey[i]*67;
      h1 = nh1;
      h2 = nh2;
    }
  };
  h = h1 ^ h2;
  if (h==0) {
    h = h1?h1:11;
  }
  /* h is never 0 */
  return h;
}

static struct attribute_flag_entry*
attr_flag_find(const char*atkey)
{
  unsigned h=0, hsiz=0;
  if (!atkey || !atkey[0])
    return NULL;
  if (!attribute_flag_hashtable)
    return NULL;
  h = atflag_hash(atkey);
  hsiz = attribute_flag_hashtable->atflag_size;
  for (unsigned ix = h % hsiz; ix < hsiz; ix++)
    {
      struct attribute_flag_entry*curent
	= attribute_flag_hashtable->atflag_entries + ix;
      if (!curent->attr_key)
	return NULL;
      if (!strcmp(curent->attr_key, atkey))
	return curent;
    };
  for (int ix = (int)h % hsiz; ix >= 0; ix--)
    {
      struct attribute_flag_entry*curent
	= attribute_flag_hashtable->atflag_entries + ix;
      if (!curent->attr_key)
	return NULL;
      if (!strcmp(curent->attr_key, atkey))
	return curent;
    };
  return NULL;
}


static struct attribute_flag_entry*
attr_flag_really_put(const char*atkey, size_t atflags)
{
  unsigned h = 0, hsiz = 0;
  if (!atkey || !atkey[0]) return NULL;
  if (!attribute_flag_hashtable)
    return NULL;
  h = atflag_hash(atkey);
  hsiz = attribute_flag_hashtable->atflag_size;
  for (unsigned ix = h % hsiz; ix < hsiz; ix++)
    {
      struct attribute_flag_entry*curent
	= attribute_flag_hashtable->atflag_entries + ix;
      if (!curent->attr_key) {
	curent->attr_key = atkey;
	curent->attr_flags = atflags;
	attribute_flag_hashtable->atflag_count++;
	return curent;
      }
      if (!strcmp(curent->attr_key, atkey)) {
	curent->attr_key = atkey;
	curent->attr_flags = atflags;
	return curent;
      }
    };
  for (int ix = (int)h % hsiz; ix >= 0; ix--)
    {
      struct attribute_flag_entry*curent
	= attribute_flag_hashtable->atflag_entries + ix;
      if (!curent->attr_key) {
	curent->attr_key = atkey;
	curent->attr_flags = atflags;
	attribute_flag_hashtable->atflag_count++;
	return curent;
      }
      if (!strcmp(curent->attr_key, atkey)) {
	curent->attr_key = atkey;
	curent->attr_flags = atflags;
	return curent;
      }
    };
  return NULL;
}

// an array of primes, gotten with something similar to
//   /usr/games/primes 3  | awk '($1>p+p/9){print $1, ","; p=$1}' 
static const unsigned primes_array[] = {
  29, 37, 43, 53, 59, 67, 79, 89, 101, 113,
  127, 149, 167, 191, 223, 251, 281, 313, 349, 389, 433, 487, 547, 613,
  683, 761, 853, 953, 1061, 1181, 1319, 1471, 1637, 1823, 2027, 2267,
  2521, 2803, 3119, 3467, 3853, 4283, 4759, 5297, 5897, 6553, 7283, 8093,
  8999, 10007, 11119, 12373, 13751, 15287, 16987, 18899, 21001, 23339,
  25933, 28817, 32027, 35591, 39551, 43951, 48847, 54277, 60317, 67021,
  74471, 82757, 91957, 102181, 113537, 126173, 140197, 155777, 173087,
  192319, 213713, 237467, 263863, 293201, 325781, 361979, 402221, 446921,
  496579, 551767, 613097, 681221, 756919, 841063, 934517, 1038383, 1153759,
  1281961, 1424407, 1582697, 1758553, 1953949, 2171077, 2412323, 2680367,
  2978189, 3309107, 3676789, 4085339, 4539277, 5043653, 5604073, 6226757,
  6918619, 7687387, 8541551, 9490631, 10545167, 11716879, 13018757,
  14465291, 16072547, 17858389, 19842659, 22047401, 24497113, 27219019,
  30243359, 33603743, 37337497, 41486111, 46095719, 51217477, 56908337,
  63231499, 70257241, 78063641, 86737379, 96374881, 107083213, 118981367,
  132201521, 146890631, 163211821, 181346479, 201496157, 223884629,
  248760703, 276400823, 307112027, 341235667, 379150777, 421278643,
  468087391, 520097111, 577885681, 642095213, 713439127, 792710159,
  880789067, 978654533, 1087393949, 1208215531, 1342461719, 1491624137,
  1657360153, 1841511311, 2046123679, 2273470799, 2526078691, 2806754123,
  0
};

/* feature request #595 : https://github.com/akheron/jansson/issues/595 */
void
json_register_dump_attribute_flag(const char*attrname, size_t attrflags)
{
  unsigned ix;
  if (!attrname || !attrname[0])
    return;
  if (!attribute_flag_hashtable) {
    unsigned initsiz = 29;
    size_t bytsiz = sizeof(struct attribute_flag_table_st) + initsiz * sizeof(struct attribute_flag_entry);
    void *ptr = jsonp_malloc(bytsiz);
    if (!ptr)
      return;
    memset (ptr, 0, bytsiz);
    attribute_flag_hashtable = ptr;
    attribute_flag_hashtable->atflag_size = initsiz;
  }
  else if (attribute_flag_hashtable->atflag_size * 4 < attribute_flag_hashtable->atflag_count * 3) {
    struct attribute_flag_table_st*oldtable = attribute_flag_hashtable;
    unsigned newsiz = 4 * attribute_flag_hashtable->atflag_count  / 3 + 17;
    unsigned lowix = 0;
    unsigned highix = sizeof(primes_array)/sizeof(primes_array[0]) - 1;
    size_t bytsiz=0;
    unsigned oldsiz=0;
    void*ptr = NULL;
    while (lowix + 4 < highix) {
      unsigned midix = (lowix+highix)/2;
      if (primes_array[midix] < lowix)
	lowix = midix;
      else highix = midix;
    };
    for (ix = lowix; ix<highix; ix++)
      if (primes_array[ix] >= newsiz) {
	newsiz = primes_array[ix];
	break;
      };
    ptr = jsonp_malloc(bytsiz);
    if (!ptr)
      return;
    memset (ptr, 0, bytsiz);
    attribute_flag_hashtable = ptr;
    attribute_flag_hashtable->atflag_size = newsiz;
    oldsiz = oldtable->atflag_size;
    for (unsigned oldix = 0; oldix < oldsiz; oldix++) {
      struct attribute_flag_entry*oldent
	= oldtable->atflag_entries + oldix;
      if (oldent->attr_key)
	attr_flag_really_put(oldent->attr_key, oldent->attr_flags);
    };
    jsonp_free(oldtable);
  };
  attr_flag_really_put(attrname, attrflags);
}



static int dump_to_strbuffer(const char *buffer, size_t size, void *data) {
    return strbuffer_append_bytes((strbuffer_t *)data, buffer, size);
}

static int dump_to_buffer(const char *buffer, size_t size, void *data) {
    struct buffer *buf = (struct buffer *)data;

    if (buf->used + size <= buf->size)
        memcpy(&buf->data[buf->used], buffer, size);

    buf->used += size;
    return 0;
}

static int dump_to_file(const char *buffer, size_t size, void *data) {
    FILE *dest = (FILE *)data;
    if (fwrite(buffer, size, 1, dest) != 1)
        return -1;
    return 0;
}

static int dump_to_fd(const char *buffer, size_t size, void *data) {
#ifdef HAVE_UNISTD_H
    int *dest = (int *)data;
    if (write(*dest, buffer, size) == (ssize_t)size)
        return 0;
#endif
    return -1;
}

/* 32 spaces (the maximum indentation size) */
static const char whitespace[] = "                                ";

static int dump_indent(size_t flags, int depth, int space, json_dump_callback_t dump,
                       void *data) {
    if (FLAGS_TO_INDENT(flags) > 0) {
        unsigned int ws_count = FLAGS_TO_INDENT(flags), n_spaces = depth * ws_count;

        if (dump("\n", 1, data))
            return -1;

        while (n_spaces > 0) {
            int cur_n =
                n_spaces < sizeof whitespace - 1 ? n_spaces : sizeof whitespace - 1;

            if (dump(whitespace, cur_n, data))
                return -1;

            n_spaces -= cur_n;
        }
    } else if (space && !(flags & JSON_COMPACT)) {
        return dump(" ", 1, data);
    }
    return 0;
}

static int dump_string(const char *str, size_t len, json_dump_callback_t dump, void *data,
                       size_t flags) {
    const char *pos, *end, *lim;
    int32_t codepoint = 0;

    if (dump("\"", 1, data))
        return -1;

    end = pos = str;
    lim = str + len;
    while (1) {
        const char *text;
        char seq[13];
        int length;

        while (end < lim) {
            end = utf8_iterate(pos, lim - pos, &codepoint);
            if (!end)
                return -1;

            /* mandatory escape or control char */
            if (codepoint == '\\' || codepoint == '"' || codepoint < 0x20)
                break;

            /* slash */
            if ((flags & JSON_ESCAPE_SLASH) && codepoint == '/')
                break;

            /* non-ASCII */
            if ((flags & JSON_ENSURE_ASCII) && codepoint > 0x7F)
                break;

            pos = end;
        }

        if (pos != str) {
            if (dump(str, pos - str, data))
                return -1;
        }

        if (end == pos)
            break;

        /* handle \, /, ", and control codes */
        length = 2;
        switch (codepoint) {
            case '\\':
                text = "\\\\";
                break;
            case '\"':
                text = "\\\"";
                break;
            case '\b':
                text = "\\b";
                break;
            case '\f':
                text = "\\f";
                break;
            case '\n':
                text = "\\n";
                break;
            case '\r':
                text = "\\r";
                break;
            case '\t':
                text = "\\t";
                break;
            case '/':
                text = "\\/";
                break;
            default: {
                /* codepoint is in BMP */
                if (codepoint < 0x10000) {
                    snprintf(seq, sizeof(seq), "\\u%04X", (unsigned int)codepoint);
                    length = 6;
                }

                /* not in BMP -> construct a UTF-16 surrogate pair */
                else {
                    int32_t first, last;

                    codepoint -= 0x10000;
                    first = 0xD800 | ((codepoint & 0xffc00) >> 10);
                    last = 0xDC00 | (codepoint & 0x003ff);

                    snprintf(seq, sizeof(seq), "\\u%04X\\u%04X", (unsigned int)first,
                             (unsigned int)last);
                    length = 12;
                }

                text = seq;
                break;
            }
        }

        if (dump(text, length, data))
            return -1;

        str = pos = end;
    }

    return dump("\"", 1, data);
}

struct key_len {
    const char *key;
    int len;
};

static int compare_keys(const void *key1, const void *key2) {
    const struct key_len *k1 = key1;
    const struct key_len *k2 = key2;
    const size_t min_size = k1->len < k2->len ? k1->len : k2->len;
    int res = memcmp(k1->key, k2->key, min_size);

    if (res)
        return res;

    return k1->len - k2->len;
}

static int do_dump(const json_t *json, size_t flags, int depth, hashtable_t *parents,
                   json_dump_callback_t dump, void *data) {
    int embed = flags & JSON_EMBED;

    flags &= ~JSON_EMBED;

    if (!json)
        return -1;

    switch (json_typeof(json)) {
        case JSON_NULL:
            return dump("null", 4, data);

        case JSON_TRUE:
            return dump("true", 4, data);

        case JSON_FALSE:
            return dump("false", 5, data);

        case JSON_INTEGER: {
            char buffer[MAX_INTEGER_STR_LENGTH];
            int size;

            size = snprintf(buffer, MAX_INTEGER_STR_LENGTH, "%" JSON_INTEGER_FORMAT,
                            json_integer_value(json));
            if (size < 0 || size >= MAX_INTEGER_STR_LENGTH)
                return -1;

            return dump(buffer, size, data);
        }

        case JSON_REAL: {
            char buffer[MAX_REAL_STR_LENGTH];
            int size;
            double value = json_real_value(json);

            size = jsonp_dtostr(buffer, MAX_REAL_STR_LENGTH, value,
                                FLAGS_TO_PRECISION(flags));
            if (size < 0)
                return -1;

            return dump(buffer, size, data);
        }

        case JSON_STRING:
            return dump_string(json_string_value(json), json_string_length(json), dump,
                               data, flags);

        case JSON_ARRAY: {
            size_t n;
            size_t i;
            /* Space for "0x", double the sizeof a pointer for the hex and a
             * terminator. */
            char key[2 + (sizeof(json) * 2) + 1];
            size_t key_len;

            /* detect circular references */
            if (jsonp_loop_check(parents, json, key, sizeof(key), &key_len))
                return -1;

            n = json_array_size(json);

            if (!embed && dump("[", 1, data))
                return -1;
            if (n == 0) {
                hashtable_del(parents, key, key_len);
                return embed ? 0 : dump("]", 1, data);
            }
            if (dump_indent(flags, depth + 1, 0, dump, data))
                return -1;

            for (i = 0; i < n; ++i) {
                if (do_dump(json_array_get(json, i), flags, depth + 1, parents, dump,
                            data))
                    return -1;

                if (i < n - 1) {
                    if (dump(",", 1, data) ||
                        dump_indent(flags, depth + 1, 1, dump, data))
                        return -1;
                } else {
                    if (dump_indent(flags, depth, 0, dump, data))
                        return -1;
                }
            }

            hashtable_del(parents, key, key_len);
            return embed ? 0 : dump("]", 1, data);
        }

        case JSON_OBJECT: {
            void *iter;
            const char *separator;
            int separator_length;
            char loop_key[LOOP_KEY_LEN];
            size_t loop_key_len;

            if (flags & JSON_COMPACT) {
                separator = ":";
                separator_length = 1;
            } else {
                separator = ": ";
                separator_length = 2;
            }

            /* detect circular references */
            if (jsonp_loop_check(parents, json, loop_key, sizeof(loop_key),
                                 &loop_key_len))
                return -1;

            iter = json_object_iter((json_t *)json);

            if (!embed && dump("{", 1, data))
                return -1;
            if (!iter) {
                hashtable_del(parents, loop_key, loop_key_len);
                return embed ? 0 : dump("}", 1, data);
            }
            if (dump_indent(flags, depth + 1, 0, dump, data))
                return -1;

            if (flags & JSON_SORT_KEYS) {
                struct key_len *keys;
                size_t size, i;

                size = json_object_size(json);
                keys = jsonp_malloc(size * sizeof(struct key_len));
                if (!keys)
                    return -1;

                i = 0;
                while (iter) {
                    struct key_len *keylen = &keys[i];

                    keylen->key = json_object_iter_key(iter);
                    keylen->len = json_object_iter_key_len(iter);

                    iter = json_object_iter_next((json_t *)json, iter);
                    i++;
                }
                assert(i == size);

                qsort(keys, size, sizeof(struct key_len), compare_keys);

                for (i = 0; i < size; i++) {
                    const struct key_len *key;
                    json_t *value;
		    struct attribute_flag_entry*atent=NULL;
                    key = &keys[i];
                    value = json_object_getn(json, key->key, key->len);
                    assert(value);
		    assert(key->key[key->len]==(char)0);
		    atent= attr_flag_find(key->key);
                    dump_string(key->key, key->len, dump, data, flags);
		    if (atent)
		      flags |= atent->attr_flags;
                    if (dump(separator, separator_length, data) ||
                        do_dump(value, flags, depth + 1, parents, dump, data)) {
                        jsonp_free(keys);
                        return -1;
                    }

                    if (i < size - 1) {
                        if (dump(",", 1, data) ||
                            dump_indent(flags, depth + 1, 1, dump, data)) {
                            jsonp_free(keys);
                            return -1;
                        }
                    } else {
                        if (dump_indent(flags, depth, 0, dump, data)) {
                            jsonp_free(keys);
                            return -1;
                        }
                    }
                }

                jsonp_free(keys);
            } else {
                /* Don't sort keys */

                while (iter) {
                    void *next = json_object_iter_next((json_t *)json, iter);
                    const char *key = json_object_iter_key(iter);
                    const size_t key_len = json_object_iter_key_len(iter);
		    struct attribute_flag_entry*atent= attr_flag_find(key);
                    dump_string(key, key_len, dump, data, flags);
		    if (atent)
		      flags |= atent->attr_flags;
                    if (dump(separator, separator_length, data) ||
                        do_dump(json_object_iter_value(iter), flags, depth + 1, parents,
                                dump, data))
                        return -1;

                    if (next) {
                        if (dump(",", 1, data) ||
                            dump_indent(flags, depth + 1, 1, dump, data))
                            return -1;
                    } else {
                        if (dump_indent(flags, depth, 0, dump, data))
                            return -1;
                    }

                    iter = next;
                }
            }

            hashtable_del(parents, loop_key, loop_key_len);
            return embed ? 0 : dump("}", 1, data);
        }

        default:
            /* not reached */
            return -1;
    }
}

char *json_dumps(const json_t *json, size_t flags) {
    strbuffer_t strbuff;
    char *result;

    if (strbuffer_init(&strbuff))
        return NULL;

    if (json_dump_callback(json, dump_to_strbuffer, (void *)&strbuff, flags))
        result = NULL;
    else
        result = jsonp_strdup(strbuffer_value(&strbuff));

    strbuffer_close(&strbuff);
    return result;
}

size_t json_dumpb(const json_t *json, char *buffer, size_t size, size_t flags) {
    struct buffer buf = {size, 0, buffer};

    if (json_dump_callback(json, dump_to_buffer, (void *)&buf, flags))
        return 0;

    return buf.used;
}

int json_dumpf(const json_t *json, FILE *output, size_t flags) {
    return json_dump_callback(json, dump_to_file, (void *)output, flags);
}

int json_dumpfd(const json_t *json, int output, size_t flags) {
    return json_dump_callback(json, dump_to_fd, (void *)&output, flags);
}

int json_dump_file(const json_t *json, const char *path, size_t flags) {
    int result;

    FILE *output = fopen(path, "w");
    if (!output)
        return -1;

    result = json_dumpf(json, output, flags);

    if (fclose(output) != 0)
        return -1;

    return result;
}

int json_dump_callback(const json_t *json, json_dump_callback_t callback, void *data,
                       size_t flags) {
    int res;
    hashtable_t parents_set;

    if (!(flags & JSON_ENCODE_ANY)) {
        if (!json_is_array(json) && !json_is_object(json))
            return -1;
    }

    if (hashtable_init(&parents_set))
        return -1;
    res = do_dump(json, flags, 0, &parents_set, callback, data);
    hashtable_close(&parents_set);

    return res;
}
