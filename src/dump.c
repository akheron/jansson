#include <jansson.h>

char *json_dumps(const json_t *json, uint32_t flags)
{
    (void)flags;

    switch(json_typeof(json)) {
        case JSON_NULL:
            printf("null");
            break;

        case JSON_TRUE:
            printf("true");
            break;

        case JSON_FALSE:
            printf("false");
            break;

        case JSON_NUMBER:
            printf("%f", json_number_value(json));
            break;

        case JSON_STRING:
            printf("%s", json_string_value(json));
            break;

        case JSON_ARRAY: {
            int i, n = json_array_size(json);
            printf("[");
            for(i = 0; i < n; ++i) {
                json_dumps(json_array_get(json, i), 0);
                if(i < n - 1)
                    printf(", ");
            }
            printf("]");
            break;
        }

        default:
            printf("<object>");
            break;
    }
    return NULL;
}

