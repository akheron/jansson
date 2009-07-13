#ifndef JANSSON_PRIVATE_H
#define JANSSON_PRIVATE_H

int json_object_set_nocheck(json_t *json, const char *key, json_t *value);
json_t *json_string_nocheck(const char *value);


#endif
