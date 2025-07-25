[CCode (cheader_filename = "jansson.h", lower_case_cprefix = "json_")]
namespace Jansson {
  [CCode (cname = "JANSSON_MAJOR_VERSION")]
  public static int MAJOR_VERSION;

  [CCode (cname = "JANSSON_MINOR_VERSION")]
  public static int MINOR_VERSION;

  [CCode (cname = "JANSSON_MICRO_VERSION")]
  public static int MICRO_VERSION;

  [CCode (cname = "JANSSON_VERSION")]
  public static string VERSION;

  [CCode (cname = "JANSSON_VERSION_HEX")]
  public static int VERSION_HEX;

  [CCode (cprefix = "JSON_", has_type_id = false)]
  public enum ElementType {
    OBJECT,
    ARRAY,
    STRING,
    INTEGER,
    REAL,
    TRUE,
    FALSE,
    NULL
  }

  [Compact]
  [CCode (cname = "json_error_t", has_copy_function = false, has_type_id = false)]
  public struct Error {
    public string text;
    public string source;
    public int line;
    public int column;
    public size_t position;
  }

  [Flags]
  [CCode (cprefix = "JSON_", has_type_id = false)]
  public enum DecodeFlags {
    REJECT_DUPLICATES,
    DISABLE_EOF_CHECK,
    DECODE_ANY,
    DECODE_INT_AS_REAL,
    ALLOW_NUL
  }


  [Flags]
  [CCode (cprefix = "JSON_", has_type_id = false)]
  public enum EncodeFlags {
    MAX_INDENT,
    COMPACT,
    ENSURE_ASCII,
    SORT_KEYS,
    PRESERVE_ORDER,
    ENCODE_ANY,
    ESCAPE_SLASH;

    [CCode (cname = "JSON_INDENT")]
    public static int INDENT(int n);
    [CCode (cname = "JSON_REAL_PRECISION")]
    public static int REAL_PRECISION(int n);
  }


  [Flags]
  [CCode (cprefix = "JSON_", has_type_id = false)]
  public enum PackFlags {
    PLACEHOLDER // These flags are not used yet according to the docs
  }

  [Flags]
  [CCode (cprefix = "JSON_", has_type_id = false)]
  public enum UnpackFlags {
    STRICT,
    VALIDATE_ONLY
  }

  [SimpleType]
  [CCode (cname = "json_int_t", has_type_id = false)]
  public struct json_int_t : int64 {
  }


  // [CCode (cname = "json_load_callback_t")]
  // TODO public delegate LoadCb(void *buffer, size_t buflen, void *data);
  // [CCode (cname = "json_dump_callback_t")]
  // TODO public delegate DumpCb(void *buffer, size_t buflen, void *data);


  [Compact]
  [CCode (cname = "json_t", lower_case_cprefix = "json_", copy_function = "json_deep_copy", has_destroy_function = false, has_construct_function = false, has_new_function = false, ref_function = "json_incref", unref_function = "json_decref", free_function = "json_delete", has_type_id = false)]
  public class Element {

    /* types */

    public Element.object();
    public Element.array();
    public Element.string(string value);
    public Element.string_nocheck(string value);
    public Element.integer(json_int_t value);
    public Element.real(double value);
    public Element.true();
    public Element.false();
    public Element.boolean(bool value);
    public Element.null();

    [CCode (cname = "json_incref")]
    public Element @ref();

    [CCode (cname = "json_decref")]
    public Element unref();

    [CCode (cname = "json_typeof")]
    public ElementType element_type();

    public bool is_object();
    public bool is_array();
    public bool is_string();
    public bool is_integer();
    public bool is_real();
    public bool is_number();
    public bool is_true();
    public bool is_false();
    public bool is_boolean();
    public bool is_null();


    /* getters, setters, manipulation */

    public void object_seed(size_t seed);
    public size_t object_size();
    public unowned Element? object_get(string key);
    public int object_set_new(string key, Element value);
    public int object_set_new_nocheck(string key, Element value);
    public int object_del(string key);
    public int object_clear();
    public int object_update(Element other);
    public int object_update_existing(Element other);
    public int object_update_missing(Element other);
    public void *object_iter();
    public void *object_iter_at(string key);
    public void *object_key_to_iter(string key);
    public void *object_iter_next(void *iter);
    public unowned string object_iter_key(void *iter);
    public Element object_iter_value(void *iter);
    public int object_iter_set_new(void *iter, Element value);
    public int object_set(string key, Element value);
    public int object_set_nocheck(string key, Element value);

    /*
    #define json_object_foreach(object, key, value) \
        for(key = json_object_iter_key(json_object_iter(object)); \
            key && (value = json_object_iter_value(json_object_key_to_iter(key))); \
            key = json_object_iter_key(json_object_iter_next(object, json_object_key_to_iter(key))))

    #define json_object_foreach_safe(object, n, key, value)     \
        for(key = json_object_iter_key(json_object_iter(object)), \
                n = json_object_iter_next(object, json_object_key_to_iter(key)); \
            key && (value = json_object_iter_value(json_object_key_to_iter(key))); \
            key = json_object_iter_key(n), \
                n = json_object_iter_next(object, json_object_key_to_iter(key)))

    #define json_array_foreach(array, index, value) \
    	for(index = 0; \
    		index < json_array_size(array) && (value = json_array_get(array, index)); \
    		index++)*/

    /*static JSON_INLINE
    int json_object_iter_set(json_t *object, void *iter, json_t *value)
    {
        return json_object_iter_set_new(object, iter, json_incref(value));
    }*/


    public size_t array_size();
    public unowned Element? array_get(size_t index);
    public int array_set_new(size_t index, Element value);
    public int array_append_new(Element value);
    public int array_insert_new(size_t index, Element value);
    public int array_remove(size_t index);
    public int array_clear();
    public int array_extend(Element other);
    public int array_set(size_t ind, Element value);
    public int array_append(Element value);
    public int array_insert(size_t ind, Element value);

    public unowned string string_value();
    public size_t string_length();
    public json_int_t integer_value();
    public double real_value();
    public double number_value();

    public int string_set(Element string, string value);
    public int string_setn(Element string, string value, size_t len);
    public int string_set_nocheck(Element string, string value);
    public int string_setn_nocheck(Element string, string value, size_t len);
    public int integer_set(Element integer, json_int_t value);
    public int json_real_set(Element real, double value);


    /* pack, unpack */

    public Element? pack(string format, ...);
    public Element pack_ex(out Error error, PackFlags flags, string format, ...);
    public int unpack(string format, ...);
    public int unpac_ex(out Error error, UnpackFlags flags, string format, ...);


    /* equality */

    public bool equal(Element value2);


    /* copying */

    Element? copy();
    Element? deep_copy();


    /* decoding */

    public static Element? loads(string input, DecodeFlags flags, out Error error);
    // TODO public static Element loadb(string input, DecodeFlags flags, out Error error);
    public static Element loadf(GLib.FileStream input, DecodeFlags flags, out Error error);
    public static Element? load_file(string path, DecodeFlags flags, out Error error);
    // TODO public static Element load_callback(LoadCb callback, DecodeFlags flags, out Error error);


    /* encoding */

    public string? dumps(EncodeFlags flags);
    public int dumpf(GLib.FileStream output, EncodeFlags flags);
    public int dump_file(string path, EncodeFlags flags);
    /*FIXME int json_dump_callback(const json_t *json, json_dump_callback_t callback, void *data, size_t flags);*/
  }
}
