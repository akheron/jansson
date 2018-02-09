#include <string.h>
#include <jansson.h>
#include "util.h"


static void test_sprintf() {
    json_t *s = json_sprintf("foo bar %d", 42);
    if (!s)
        fail("json_sprintf returned NULL");
    if (!json_is_string(s))
        fail("json_sprintf didn't return a JSON string");
    if (strcmp(json_string_value(s), "foo bar 42"))
        fail("json_sprintf generated an unexpected string");

    json_decref(s);
}


static void run_tests()
{
    test_sprintf();
}
