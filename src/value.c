#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include <jansson.h>
#include "hashtable.h"

#define max(a, b)  ((a) > (b) ? (a) : (b))

#define container_of(ptr_, type_, member_)  \
    ((type_ *)((char *)ptr_ - (size_t)&((type_ *)0)->member_))

typedef struct {
    json_t json;
    hashtable_t *hashtable;
} json_object_t;

typedef struct {
    json_t json;
    unsigned int size;
    unsigned int entries;
    json_t **table;
} json_array_t;

typedef struct {
    json_t json;
    char *value;
} json_string_t;

typedef struct {
    json_t json;
    double value;
} json_number_t;

#define json_to_object(json_)  container_of(json_, json_object_t, json)
#define json_to_array(json_)   container_of(json_, json_array_t, json)
#define json_to_string(json_)  container_of(json_, json_string_t, json)
#define json_to_number(json_)  container_of(json_, json_number_t, json)

static inline void json_init(json_t *json, json_type type)
{
    json->type = type;
    json->refcount = 1;
}


/*** object ***/

static unsigned int hash_string(const void *key)
{
    const char *str = (const char *)key;
    unsigned int hash = 5381;
    unsigned int c;

    while((c = (unsigned int)*str))
    {
        hash = ((hash << 5) + hash) + c;
        str++;
    }

    return hash;
}

static int string_equal(const void *key1, const void *key2)
{
    return strcmp((const char *)key1, (const char *)key2) == 0;
}

static void value_decref(void *value)
{
    json_decref((json_t *)value);
}

json_t *json_object(void)
{
    json_object_t *object = malloc(sizeof(json_object_t));
    if(!object)
        return NULL;
    json_init(&object->json, JSON_OBJECT);

    object->hashtable =
      hashtable_new(hash_string, string_equal, free, value_decref);
    if(!object->hashtable)
    {
        free(object);
        return NULL;
    }
    return &object->json;
}

static void json_delete_object(json_object_t *object)
{
    hashtable_free(object->hashtable);
    free(object);
}

json_t *json_object_get(const json_t *json, const char *key)
{
    json_object_t *object;

    if(!json_is_object(json))
        return NULL;

    return hashtable_get(object->hashtable, key);
}

int json_object_del(json_t *json, const char *key)
{
    json_object_t *object;

    if(!json_is_object(json))
        return -1;

    object = json_to_object(json);
    return hashtable_del(object->hashtable, key);
}

int json_object_set(json_t *json, const char *key, json_t *value)
{
    json_object_t *object;

    if(!json_is_object(json))
        return -1;

    object = json_to_object(json);
    return hashtable_set(object->hashtable, strdup(key), json_incref(value));
}


/*** array ***/

json_t *json_array(void)
{
    json_array_t *array = malloc(sizeof(json_array_t));
    if(!array)
      return NULL;
    json_init(&array->json, JSON_ARRAY);

    array->entries = 0;
    array->size = 0;
    array->table = NULL;

    return &array->json;
}

static void json_delete_array(json_array_t *array)
{
    unsigned int i;

    for(i = 0; i < array->entries; i++)
        json_decref(array->table[i]);

    free(array->table);
    free(array);
}

unsigned int json_array_size(const json_t *json)
{
    if(!json_is_array(json))
        return 0;

    return json_to_array(json)->entries;
}

json_t *json_array_get(const json_t *json, unsigned int index)
{
    json_array_t *array;
    if(!json_is_array(json))
        return NULL;
    array = json_to_array(json);

    if(index >= array->size)
        return NULL;

    return array->table[index];
}

int json_array_set(json_t *json, unsigned int index, json_t *value)
{
    json_array_t *array;
    if(!json_is_array(json))
        return -1;
    array = json_to_array(json);

    if(index >= array->size)
        return -1;

    array->table[index] = json_incref(value);
    return 0;
}

int json_array_append(json_t *json, json_t *value)
{
    json_array_t *array;
    if(!json_is_array(json))
        return -1;
    array = json_to_array(json);

    if(array->entries == array->size) {
        array->size = max(8, array->size * 2);
        array->table = realloc(array->table, array->size * sizeof(json_t *));
        if(!array->table)
            return -1;
    }

    array->table[array->entries] = json_incref(value);
    array->entries++;

    return 0;
}


/*** string ***/

json_t *json_string(const char *value)
{
    json_string_t *string = malloc(sizeof(json_string_t));
    if(!string)
       return NULL;
    json_init(&string->json, JSON_STRING);

    string->value = strdup(value);
    return &string->json;
}

const char *json_string_value(const json_t *json)
{
    if(!json_is_string(json))
        return NULL;

    return json_to_string(json)->value;
}

static void json_delete_string(json_string_t *string)
{
    free(string->value);
    free(string);
}

json_t *json_number(double value)
{
    json_number_t *number = malloc(sizeof(json_number_t));
    if(!number)
       return NULL;
    json_init(&number->json, JSON_NUMBER);

    number->value = value;
    return &number->json;
}


/*** number ***/

double json_number_value(const json_t *json)
{
    if(!json_is_number(json))
        return 0.0;

    return json_to_number(json)->value;
}

static void json_delete_number(json_number_t *number)
{
    free(number);
}


/*** simple values ***/

json_t *json_true(void)
{
    static json_t the_true = {
        .type = JSON_TRUE,
        .refcount = 1
    };
    return json_incref(&the_true);
}


json_t *json_false(void)
{
    static json_t the_false = {
        .type = JSON_FALSE,
        .refcount = 1
    };
    return json_incref(&the_false);
}


json_t *json_null(void)
{
    static json_t the_null = {
        .type = JSON_NULL,
        .refcount = 1
    };
    return json_incref(&the_null);
}


/*** deletion ***/

void json_delete(json_t *json)
{
    if(json_is_object(json))
        json_delete_object(json_to_object(json));

    else if(json_is_array(json))
        json_delete_array(json_to_array(json));

    else if(json_is_string(json))
        json_delete_string(json_to_string(json));

    else if(json_is_number(json))
        json_delete_number(json_to_number(json));

    /* json_delete is not called for true, false or null */
}
