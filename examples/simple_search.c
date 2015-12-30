#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>

char * str_haystack = "{\
    \"haystack\": \"full of hay\",\
    \"keystr\": \"valuestr\",\
    \"keyint\": 1234,\
    \"keyreal\": 12.34,\
    \"keyobject\": {\
        \"name\": \"innerObject\",\
        \"otherKey\": \"otherValue\",\
        \"otherInt\": 778,\
        \"needle2\": \"waldo2\",\
        \"needle5\": [\
            \"waldo5\", \"waldo6\", \"waldo7\"\
        ]\
    },\
    \"keyarray\": [\
        {\
            \"name\": \"innerObjectInArray\",\
            \"otherKey\": \"otherValue\",\
            \"otherInt\": 778,\
            \"needle3\": 92536\
        },\
        543,\
        \"innerString\",\
        {\
            \"needle4\": \"waldo4\",\
            \"needle4key2\": 432\
        }\
    ],\
    \"needle1\": \"waldo1\"\
}";

int main() {
    json_t * j_haystack = json_loads(str_haystack, JSON_DECODE_ANY, NULL);
    json_t * j_needle1 = json_string("waldo1");
    json_t * j_needle2 = json_string("waldo2");
    json_t * j_needle3 = json_integer(92536);
    json_t * j_needle4 = json_pack("{sssi}", "needle4", "waldo4", "needle4key2", json_integer(432));
    json_t * j_needle5 = json_pack("[sss]", "waldo5", "waldo6", "waldo7");
    json_t * j_needle_none = json_pack("{ss}", "needle4", "waldo4");
    
    if (json_search(j_haystack, j_needle1) != NULL) {
        printf("needle %s found\n", json_dumps(j_needle1, JSON_ENCODE_ANY));
    } else {
        printf("needle %s not found\n", json_dumps(j_needle1, JSON_ENCODE_ANY));
    }
    
    if (json_search(j_haystack, j_needle2) != NULL) {
        printf("needle %s found\n", json_dumps(j_needle2, JSON_ENCODE_ANY));
    } else {
        printf("needle %s not found\n", json_dumps(j_needle2, JSON_ENCODE_ANY));
    }
    
    if (json_search(j_haystack, j_needle3) != NULL) {
        printf("needle %s found\n", json_dumps(j_needle3, JSON_ENCODE_ANY));
    } else {
        printf("needle %s not found\n", json_dumps(j_needle3, JSON_ENCODE_ANY));
    }
    
    if (json_search(j_haystack, j_needle4) != NULL) {
        printf("needle %s found\n", json_dumps(j_needle4, JSON_ENCODE_ANY));
    } else {
        printf("needle %s not found\n", json_dumps(j_needle4, JSON_ENCODE_ANY));
    }
    
    if (json_search(j_haystack, j_needle5) != NULL) {
        printf("needle %s found\n", json_dumps(j_needle5, JSON_ENCODE_ANY));
    } else {
        printf("needle %s not found\n", json_dumps(j_needle5, JSON_ENCODE_ANY));
    }
    
    if (json_search(j_haystack, j_needle_none) != NULL) {
        printf("needle %s found\n", json_dumps(j_needle_none, JSON_ENCODE_ANY));
    } else {
        printf("needle %s not found\n", json_dumps(j_needle_none, JSON_ENCODE_ANY));
    }
    
    return 0;
}
