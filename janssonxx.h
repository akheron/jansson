// janssonxx - C++ wrapper for jansson
//
// author: Sean Middleditch <sean@middleditch.us>
//
// janssonxx is free software; you can redistribute it and/or modify
// it under the terms of the MIT license. See LICENSE for details.


#if !defined(JANSSONXX_H)
#define JANSSONXX_H 1

#include <string>
#include <ostream>
#include <istream>
#include <sstream>
#include <cstdlib>

namespace jansson {

// include in the jansson namespace
#include <jansson.h>

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
		if (&e != this) {
			json_decref(_value);
			_value = json_incref(e._value);
		}
		return *this;
	}

	// load a file as a JSON value
	static Value load_file(const char* path, json_error_t* error = 0) {
		return Value::_take(json_load_file(path, error));
	}

	// load a string as a JSON value
	static Value load_string(const char* string, json_error_t* error = 0) {
		return Value::_take(json_loads(string, error));
	}

	// write the value to a file
	int save_file(const char* path, int flags = 0) const {
		return json_dump_file(_value, path, flags);
	}

	// write the value to a string (caller must deallocate with free()!)
	char* save_string(int flags = 0) const {
		return json_dumps(_value, flags);
	}

	// construct Value from input
	static Value from(const char* value) { return Value::_take(json_string(value)); }
	static Value from(const std::string& value) { return from(value.c_str()); }
	static Value from(bool value) { return Value::_take(value ? json_true() : json_false()); }
	static Value from(signed int value) { return Value::_take(json_integer(value)); }
	static Value from(unsigned int value) { return Value::_take(json_integer(value)); }
	static Value from(signed short value) { return Value::_take(json_integer(value)); }
	static Value from(unsigned short value) { return Value::_take(json_integer(value)); }
	static Value from(signed long value) { return Value::_take(json_integer(value)); }
	static Value from(unsigned long value) { return Value::_take(json_integer(value)); }
	static Value from(float value) { return Value::_take(json_real(value)); }
	static Value from(double value) { return Value::_take(json_real(value)); }

	// create a new empty object
	static Value object() { return Value::_take(json_object()); }

	// create a new empty array
	static Value array() { return Value::_take(json_array()); }

	// create a new null value
	static Value null() { return Value::_take(json_null()); }

	// get the underlying json_t
	json_t* as_json() const { return _value; }

	// check value type
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
		else
			return json_array_size(_value);
	}

	// get value at array index (const version)
	const Value at(unsigned int index) const {
		return Value(json_array_get(_value, index));
	}

	const Value operator[](signed int index) const { return at(index); }
	const Value operator[](unsigned int index) const { return at(index); }
	const Value operator[](signed short index) const { return at(index); }
	const Value operator[](unsigned short index) const { return at(index); }
	const Value operator[](signed long index) const { return at(index); }
	const Value operator[](unsigned long index) const { return at(index); }

	// get value at array index (non-const version)
	Value at(unsigned int index) {
		return Value(json_array_get(_value, index));
	}

	Value operator[](signed int index) { return at(index); }
	Value operator[](unsigned int index) { return at(index); }
	Value operator[](signed short index) { return at(index); }
	Value operator[](unsigned short index) { return at(index); }
	Value operator[](signed long index) { return at(index); }
	Value operator[](unsigned long index) { return at(index); }

	// get object property
	const Value get(const char* key) const {
		return Value(json_object_get(_value, key));
	}

	const Value get(const std::string& key) const { return get(key.c_str()); }
	const Value operator[](const char* key) const { return get(key); }
	const Value operator[](const std::string& key) const { return get(key.c_str()); }

	// clear all array/object values
	void clear() {
		if (is_object())
			json_object_clear(_value);
		else
			json_array_clear(_value);
	}

	// get value cast to specified type
	const char* as_cstring() const { return json_string_value(_value); }
	std::string as_string() const {
		const char* tmp = as_cstring();
		return tmp == 0 ? "" : tmp;
	}
	int as_integer() const { return json_integer_value(_value); }
	double as_real() const { return json_real_value(_value); }
	double as_number() const { return json_number_value(_value); }
	bool as_boolean() const { return is_true(); }

	// set an object property (converts value to object is not one already)
	Value& set_key(const char* key, const Value& value) {
		json_object_set(_value, key, value.as_json());
		return *this;
	}

	Value& set_key(const std::string& key, const Value& value) {
		return set_key(key.c_str(), value);
	}

	// set an array index (converts value to object is not one already)
	Value& set_at(unsigned int index, const Value& value) {
		if (index == size())
			json_array_append(_value, value.as_json());
		else
			json_array_set(_value, index, value.as_json());
		return *this;
	}

	// delete an object key
	Value& del_key(const char* key) {
		json_object_del(_value, key);
		return *this;
	}

	Value& del_key(const std::string& key) {
		return del_key(key.c_str());
	}

	// delete an item from an array by index
	Value& del_at(unsigned int index) {
		json_array_remove(_value, index);
		return *this;
	}

	// insert an item into an array at a given index
	Value& insert_at(unsigned int index, const Value& value) {
		json_array_insert(_value, index, value.as_json());
		return *this;
	}

private:
	// take ownership of a json_t (does not increase reference count)
	static Value _take(json_t* json) {
		Value v;
		v._value = json;
		return v;
	}

	// internal value pointer
	json_t* _value;
};

// iterators over a JSON object
class Iterator {
public:
	// construct a new iterator for a given object
	Iterator(const Value& value) : _object(value), _iter(0) {
		_iter = json_object_iter(_object.as_json());
	}

	// increment iterator
	void next() {
		_iter = json_object_iter_next(_object.as_json(), _iter);
	}

	Iterator& operator++() { next(); return *this; }

	// test if iterator is still valid
	bool valid() const { return _iter != 0; }
	operator bool() const { return valid(); }

	// get key
	const char* ckey() const {
		return json_object_iter_key(_iter);
	}

	std::string key() const { return ckey(); }

	// get value
	const Value value() const {
		return Value(json_object_iter_value(_iter));
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

// stream JSON value out
std::ostream& operator<<(std::ostream& os, const jansson::Value& value) {
	char* tmp = value.save_string();
	if (tmp != 0) {
		os << tmp;
		free(tmp);
	}
	return os;
}

// read JSON value
std::istream& operator>>(std::istream& is, jansson::Value& value) {
	std::stringstream tmp;
	while (is)
		tmp << static_cast<char>(is.get());
	value = jansson::Value::load_string(tmp.str().c_str());
	return is;
}

#endif
