#include "util.h"
#include <jansson.h>
#include <string.h>

static void run_tests() {
    json_error_t *error;

    json_t *json = json_pack(
        "{s:{s:[{}[]sifbbn],s:s,s:i,s:f,s:b,s:b,s:n,s:{s:{s:{}}}}}",
        "base",
        "array",
        "string 1",
        1,
        2.2,
        1,
        0,
        "string2",
        "string3",
        "integer",
        3,
        "real",
        4.4,
        "true",
        1,
        "false",
        0,
        "null",
        "nest 1",
        "nest 2",
        "nest 3"
    );

    // printf("%s\n", json_dumps(json,0));

    json_t* value = json_object_get_path(json, 1, JSON_OBJECT, "A");
    if(value) fail("should not have found object");
    
    value = json_object_get_path(json, 1, JSON_ARRAY, "A");
    if(value) fail("should not have found array");
    
    value = json_object_get_path(json, 1, JSON_STRING, "A");
    if(value) fail("should not have found string");
    
    value = json_object_get_path(json, 1, JSON_INTEGER, "A");
    if(value) fail("should not have found integer");
    
    value = json_object_get_path(json, 1, JSON_TRUE, "A");
    if(value) fail("should not have found true");
    
    value = json_object_get_path(json, 1, JSON_FALSE, "A");
    if(value) fail("should not have found false");
    
    value = json_object_get_path(json, 1, JSON_NULL, "A");
    if(value) fail("should not have found null");

    value = json_object_get_path(json, 1, JSON_OBJECT, "base");
    if(!value) fail("did not find object");

    value = json_object_get_path(json, 2, JSON_OBJECT, "base", JSON_ARRAY, "array");
    if(!value) fail("did not find array");

    value = json_object_get_path(json, 3, JSON_OBJECT, "base", JSON_ARRAY, "array", (size_t) 0);
    if(!value) fail("did not find array element");
    if(value->type != JSON_OBJECT) fail("did not find object in array");

    value = json_object_get_path(json, 3, JSON_OBJECT, "base", JSON_ARRAY, "array", (size_t) 1);
    if(!value) fail("did not find array element");
    if(value->type != JSON_ARRAY) fail("did not find array in array");

    value = json_object_get_path(json, 3, JSON_OBJECT, "base", JSON_ARRAY, "array", (size_t) 2);
    if(!value) fail("did not find array element");
    if(value->type != JSON_STRING) fail("did not find string in array");

    value = json_object_get_path(json, 3, JSON_OBJECT, "base", JSON_ARRAY, "array", (size_t) 3);
    if(!value) fail("did not find array element");
    if(value->type != JSON_INTEGER) fail("did not find integer in array");

    value = json_object_get_path(json, 3, JSON_OBJECT, "base", JSON_ARRAY, "array", (size_t) 4);
    if(!value) fail("did not find array element");
    if(value->type != JSON_REAL) fail("did not find real in array");

    value = json_object_get_path(json, 3, JSON_OBJECT, "base", JSON_ARRAY, "array", (size_t) 5);
    if(!value) fail("did not find array element");
    if(value->type != JSON_TRUE) fail("did not find real in array");

    value = json_object_get_path(json, 3, JSON_OBJECT, "base", JSON_ARRAY, "array", (size_t) 6);
    if(!value) fail("did not find array element");
    if(value->type != JSON_FALSE) fail("did not find real in array");

    value = json_object_get_path(json, 3, JSON_OBJECT, "base", JSON_ARRAY, "array", (size_t) 7);
    if(!value) fail("did not find array element");
    if(value->type != JSON_NULL) fail("did not find real in array");

    value = json_object_get_path(json, 3, JSON_OBJECT, "base", JSON_ARRAY, "array", (size_t) 8);
    if(value) fail("should not have found array element");

    value = json_object_get_path(json, 2, JSON_OBJECT, "base", JSON_STRING, "string2");
    if(!value) fail("did not find string in object");
    if(value->type != JSON_STRING) fail("did not find string in object");
    if(strcmp(json_string_value(value), "string3")) fail("did not get appropriate string value");

    value = json_object_get_path(json, 2, JSON_OBJECT, "base", JSON_INTEGER, "integer");
    if(!value) fail("did not find integer in object");
    if(value->type != JSON_INTEGER) fail("did not find integer in object");
    if(json_integer_value(value) != 3) fail("did not get appropriate integer value");

    value = json_object_get_path(json, 2, JSON_OBJECT, "base", JSON_REAL, "real");
    if(!value) fail("did not find real in object");
    if(value->type != JSON_REAL) fail("did not find real in object");
    if(json_real_value(value)!= 4.4) fail("did not get appropriate real value");

    value = json_object_get_path(json, 2, JSON_OBJECT, "base", JSON_TRUE, "true");
    if(!value) fail("did not find true in object");
    if(value->type != JSON_TRUE) fail("did not find true in object");
    if(!json_is_true(value)) fail("did not get appropriate true value");

    value = json_object_get_path(json, 2, JSON_OBJECT, "base", JSON_FALSE, "false");
    if(!value) fail("did not find false in object");
    if(value->type != JSON_FALSE) fail("did not find false in object");
    if(!json_is_false(value)) fail("did not get appropriate false value");

    value = json_object_get_path(json, 2, JSON_OBJECT, "base", JSON_NULL, "null");
    if(!value) fail("did not find null in object");
    if(value->type != JSON_NULL) fail("did not find null in object");

    value = json_object_get_path(json, 2, JSON_OBJECT, "base", JSON_NULL, "null", JSON_OBJECT, "nest 1", JSON_OBJECT, "nest 2", JSON_OBJECT, "nest 3");
    if(!value) fail("did not ignore garbage arguments");
    
    value = json_object_get_path(json, 4, JSON_OBJECT, "base", JSON_OBJECT, "nest 1", JSON_OBJECT, "nest 2", JSON_OBJECT, "nest 3");
    if(!value) fail("did not find nested object");

    json_decref(json);

    json = json_pack(
        "[sss]",
        "base",
        "array",
        "string 1"
    );

    value = json_object_get_path(json, 1, (size_t) 0);
    if(!value) fail("did not find object in list");
    if(strcmp(json_string_value(value), "base")) fail("did not get appropriate string value");

    value = json_object_get_path(json, 1, (size_t) 1);
    if(!value) fail("did not find object in list");
    if(strcmp(json_string_value(value), "array")) fail("did not get appropriate string value");

    value = json_object_get_path(json, 1, (size_t) 2);
    if(!value) fail("did not find object in list");
    if(strcmp(json_string_value(value), "string 1")) fail("did not get appropriate string value");

    json_decref(json);
}