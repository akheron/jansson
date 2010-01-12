#if !defined(JANSSONXX_H)
#define JANSSONXX_H 1

#include <string>
#include <jansson.h>

namespace jansson {

class Iterator;

// represents any JSON value
class Value {
public:
	// construct new Value with an undefined value
	Value() : _value(0) {}

	// free Value resources
	~Value() { json_decref(_value); }

	// copy an existing Value
	Value(const Value& e) : _value(json_incref(e._value)) {}

	// make a reference to an existing json_t value
	explicit Value(json_t* value) : _value(json_incref(value)) {}

	// copy an existing Value
	Value& operator=(const Value& e) {
		json_decref(_value);
		_value = json_incref(e._value);
		return *this;
	}

	// take ownership of a json_t (does not increase reference count)
	Value& take_ownership(json_t* value) {
		json_decref(_value);
		_value = value;
		return *this;
	}

	// load a file as a JSON value
	static Value load_file(const char* path, json_error_t* error = 0) {
		return Value().take_ownership(json_load_file(path, error));
	}

	// load a string as a JSON value
	static Value load_string(const char* string, json_error_t* error = 0) {
		return Value().take_ownership(json_loads(string, error));
	}

	// get the underlying json_t
	json_t* as_json_t() const { return _value; }

	// check Value value type
	bool is_undefined() const { return _value == 0; }
	bool is_object() const { return json_is_object(_value); }
	bool is_array() const { return json_is_array(_value); }
	bool is_string() const { return json_is_string(_value); }
	bool is_integer() const { return json_is_integer(_value); }
	bool is_real() const { return json_is_real(_value); }
	bool is_number() const { return json_is_number(_value); }
	bool is_true() const { return json_is_true(_value); }
	bool is_false() const { return json_is_false(_value); }
	bool is_boolean() const { return json_is_boolean(_value); }
	bool is_null() const { return json_is_null(_value); }

	// get size of array or object
	unsigned int size() const {
		if (is_object())
			return json_object_size(_value);
		else if (is_array())
			return json_array_size(_value);
		else
			return 0;
	}

	// get value at array index
	const Value at(unsigned int index) const {
		if (is_array())
			return Value(json_array_get(_value, index));
		else
			return Value();
	}

	const Value operator[](signed int index) const { return at(index); }
	const Value operator[](unsigned int index) const { return at(index); }
	const Value operator[](signed short index) const { return at(index); }
	const Value operator[](unsigned short index) const { return at(index); }
	const Value operator[](signed long index) const { return at(index); }
	const Value operator[](unsigned long index) const { return at(index); }

	// get object property
	const Value get(const char* key) const {
		if (is_object())
			return Value(json_object_get(_value, key));
		else
			return Value();
	}

	const Value get(const std::string& key) const { return get(key.c_str()); }
	const Value operator[](const char* key) const { return get(key); }
	const Value operator[](const std::string& key) const { return get(key.c_str()); }

	// clear all array/object values
	void clear() {
		if (is_object())
			json_object_clear(_value);
		else if (is_array())
			json_array_clear(_value);
	}

	// get value cast to specified type
	const char* as_cstring() const { return json_string_value(_value); }
	std::string as_string() const { return as_cstring(); }
	int as_integer() const { return json_integer_value(_value); }
	double as_real() const { return json_real_value(_value); }
	double as_number() const { return json_number_value(_value); }
	bool as_boolean() const { return is_true(); }

	// assign new string value
	Value& operator=(const char* value) {
		json_decref(_value);
		_value = json_string(value);
		return *this;
	}

	// assign new integer value
	Value& operator=(int value) {
		json_decref(_value);
		_value = json_integer(value);
		return *this;
	}

	// assign new real/double/number value
	Value& operator=(double value) {
		json_decref(_value);
		_value = json_real(value);
		return *this;
	}

	// assign new boolean value
	Value& operator=(bool value) {
		json_decref(_value);
		_value = value ? json_true() : json_false();
		return *this;
	}

private:
	// internal value pointer
	json_t* _value;
};

// iterators over a JSON object
class Iterator {
public:
	// construct a new iterator for a given object
	Iterator(const Value& value) : _object(value), _iter(0) {
		_iter = json_object_iter(_object.as_json_t());
	}

	// increment iterator
	void next() {
		if (_iter != 0)
			_iter = json_object_iter_next(_object.as_json_t(), _iter);
	}

	Iterator& operator++() { next(); return *this; }

	// test if iterator is still valid
	bool valid() const { return _iter != 0; }
	operator bool() const { return valid(); }

	// get key
	const char* ckey() const {
		if (_iter != 0)
			return json_object_iter_key(_iter);
		else
			return "";
	}

	std::string key() const { return ckey(); }

	// get value
	const Value value() const {
		if (_iter != 0)
			return Value(json_object_iter_value(_iter));
		else
			return Value();
	}

	// dereference value
	const Value operator*() const { return value(); }

private:
	// disallow copying
	Iterator(const Iterator&);
	Iterator& operator=(const Iterator&);

	// object being iterated over
	Value _object;

	// iterator value
	void* _iter;
};

} // namespace jansson

#endif
