#include <stdint.h>
#include <sys/types.h>

#include "jansson.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  json_error_t error;

  if (size < sizeof(size_t) + sizeof(size_t))
  {
    return 0;
  }

  // Use the first sizeof(size_t) bytes as load flags.
  size_t load_flags = *(const size_t*)data;
  data += sizeof(size_t);
  size -= sizeof(size_t);

  // Use the next sizeof(size_t) bytes as dump flags.
  size_t dump_flags = *(const size_t*)data;
  data += sizeof(size_t);
  size -= sizeof(size_t);

  // Attempt to load the remainder of the data with the given load flags.
  const char* text = reinterpret_cast<const char *>(data);
  json_t* jobj = json_loadb(text, size, load_flags, &error);

  if (jobj == NULL)
  {
    return 0;
  }

  // Attempt to dump the loaded json object with the given dump flags.
  char* out = json_dumps(jobj, dump_flags);
  if (out)
  {
    free(out);
  }

  if (jobj)
  {
    json_decref(jobj);
  }

  return 0;
}