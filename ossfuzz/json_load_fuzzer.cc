#include <stdint.h>

#include "jansson.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  json_error_t error;
  auto jobj = json_loadb(reinterpret_cast<const char *>(data), size, 0, &error);
  if (jobj)
    json_decref(jobj);
  return 0;
}
