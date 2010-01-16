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
#	include <jansson.h>

	class Iterator;
	class Value;

	namespace _private {
		class ElementProxy;
		class PropertyProxy;

		// base class for JSON value interface
		template <typename _Base>
		class ValueBase : public _Base {
		public:
			// empty constructor
			ValueBase() : _Base() {}

			// copy constructor
			ValueBase(const _Base& base) : _Base(base) {}

			// create reference to value
			ValueBase(json_t* json) : _Base(json) {}

			// assignment operator
			ValueBase& operator=(const Value& value) { _Base::operator=(value); return *this; }

			// check value type
			bool is_undefined() const { return _Base::as_json() == 0; }
			bool is_object() const { return json_is_object(_Base::as_json()); }
			bool is_array() const { return json_is_array(_Base::as_json()); }
			bool is_string() const { return json_is_string(_Base::as_json()); }
			bool is_integer() const { return json_is_integer(_Base::as_json()); }
			bool is_real() const { return json_is_real(_Base::as_json()); }
			bool is_number() const { return json_is_number(_Base::as_json()); }
			bool is_true() const { return json_is_true(_Base::as_json()); }
			bool is_false() const { return json_is_false(_Base::as_json()); }
			bool is_boolean() const { return json_is_boolean(_Base::as_json()); }
			bool is_null() const { return json_is_null(_Base::as_json()); }

			// get size of array or object
			inline unsigned int size() const;

			// get value at array index (const version)
			inline const Value at(unsigned int index) const;

			inline const Value operator[](signed int index) const;
			inline const Value operator[](unsigned int index) const;
			inline const Value operator[](signed short index) const;
			inline const Value operator[](unsigned short index) const;
			inline const Value operator[](signed long index) const;
			inline const Value operator[](unsigned long index) const;

			// get value at array index (non-const version)
			inline ValueBase<ElementProxy> at(unsigned int index);

			inline ValueBase<ElementProxy> operator[](signed int index);
			inline ValueBase<ElementProxy> operator[](unsigned int index);
			inline ValueBase<ElementProxy> operator[](signed short index);
			inline ValueBase<ElementProxy> operator[](unsigned short index);
			inline ValueBase<ElementProxy> operator[](signed long index);
			inline ValueBase<ElementProxy> operator[](unsigned long index);

			// get object property (const version)
			inline const Value get(const char* key) const;

			inline const Value get(const std::string& key) const;
			inline const Value operator[](const char* key) const;
			inline const Value operator[](const std::string& key) const;

			// get object property (non-const version)
			inline ValueBase<PropertyProxy> get(const char* key);

			inline ValueBase<PropertyProxy> get(const std::string& key);
			inline ValueBase<PropertyProxy> operator[](const char* key);
			inline ValueBase<PropertyProxy> operator[](const std::string& key);

			// clear all array/object values
			inline void clear();

			// get value cast to specified type
			inline const char* as_cstring() const;
			inline std::string as_string() const;
			inline int as_integer() const;
			inline double as_real() const;
			inline double as_number() const;
			inline bool as_boolean() const;

			// set an object property (converts value to object is not one already)
			inline _Base& set_key(const char* key, const Value& value);

			inline _Base& set_key(const std::string& key, const Value& value);

			// set an array index (converts value to object is not one already)
			inline _Base& set_at(unsigned int index, const Value& value);

			// delete an object key
			inline _Base& del_key(const char* key);

			inline _Base& del_key(const std::string& key);

			// delete an item from an array by index
			inline _Base& del_at(unsigned int index);

			// insert an item into an array at a given index
			inline _Base& insert_at(unsigned int index, const Value& value);
		};

		// represents any JSON value, private base
		class Basic {
		public:
			// construct new Value with an undefined value
			Basic() : _value(0) {}

			// copy constructor
			Basic(const Basic& value) : _value(json_incref(value._value)) {}

			// make a reference to an existing json_t value
			explicit Basic(json_t* value) : _value(json_incref(value)) {}

			// free Value resources
			~Basic() { json_decref(_value); }

			// copy an existing Value
			Basic& operator=(const Basic& e) {
				if (&e != this) {
					json_decref(_value);
					_value = json_incref(e._value);
				}
				return *this;
			}

			// get the underlying json_t
			json_t* as_json() const { return _value; }

		protected:
			// take ownership of a json_t (does not increase reference count)
			static Basic _take(json_t* json) {
				Basic v;
				v._value = json;
				return v;
			}

		private:
			// internal value pointer
			json_t* _value;
		};

		// proxies an array element
		class ElementProxy {
		public:
			// constructor
			ElementProxy(json_t* array, unsigned int index) : _array(array), _index(index) {}

			// assign to the proxied element
			inline ElementProxy& operator=(const Value& value);

			// get the proxied element
			json_t* as_json() const { return json_array_get(_array, _index); }

		private:
			// array object we wrap
			json_t* _array;

			// index of property
			unsigned int _index;
		};

		// proxies an object property
		class PropertyProxy {
		public:
			// constructor
			PropertyProxy(json_t* array, const char* key) : _object(array), _key(key) {}

			// assign to the proxied element
			inline PropertyProxy& operator=(const Value& value);

			// get the proxied element
			json_t* as_json() const { return json_object_get(_object, _key); }

		private:
			// array object we wrap
			json_t* _object;

			// key of property
			const char* _key;
		};

	} // namespace jansson::_private

	// represents any JSON value
	class Value : public _private::ValueBase<_private::Basic> {
	public:
		// empty constructor
		Value() : _private::ValueBase<_private::Basic>() {}

		// copy constructor for base
		Value(const _private::Basic& value) : _private::ValueBase<_private::Basic>(value) {}
	
		// copy constructor for base
		Value(const _private::ValueBase<_private::Basic>& value) : _private::ValueBase<_private::Basic>(value) {}

		// copy constructor
		Value(const Value& value) : _private::ValueBase<_private::Basic>(value) {}

		// create reference to value
		explicit Value(json_t* json) : _private::ValueBase<_private::Basic>(json) {}

		// construct Value from input
		static inline Value from(const char* value) { return Value::_take(json_string(value)); }
		static inline Value from(const std::string& value) { return from(value.c_str()); }
		static inline Value from(bool value) { return Value::_take(value ? json_true() : json_false()); }
		static inline Value from(signed int value) { return Value::_take(json_integer(value)); }
		static inline Value from(unsigned int value) { return Value::_take(json_integer(value)); }
		static inline Value from(signed short value) { return Value::_take(json_integer(value)); }
		static inline Value from(unsigned short value) { return Value::_take(json_integer(value)); }
		static inline Value from(signed long value) { return Value::_take(json_integer(value)); }
		static inline Value from(unsigned long value) { return Value::_take(json_integer(value)); }
		static inline Value from(float value) { return Value::_take(json_real(value)); }
		static inline Value from(double value) { return Value::_take(json_real(value)); }

		// create a new empty object
		static inline Value object() { return Value::_take(json_object()); }

		// create a new empty array
		static inline Value array() { return Value::_take(json_array()); }

		// create a new null value
		static inline Value null() { return Value::_take(json_null()); }

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
			return json_dump_file(as_json(), path, flags);
		}

		// write the value to a string (caller must deallocate with free()!)
		char* save_string(int flags = 0) const {
			return json_dumps(as_json(), flags);
		}
	};

	// iterators over a JSON object
	class Iterator {
	public:
		// construct a new iterator for a given object
		Iterator(const Value& value) : _object(value), _iter(0) {
			_iter = json_object_iter(_object.as_json());
		}

		// construct a new iterator for a given object
		Iterator(const _private::ValueBase<_private::PropertyProxy>& value) : _object(value.as_json()), _iter(0) {
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

#include "janssonxx.tcc"

#endif
