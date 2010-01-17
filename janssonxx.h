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
	// include Jansson C library in the jansson namespace
	#include <jansson.h>

	class Iterator;
	class Value;

	// implementation details; do not use directly
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
			inline ValueBase& operator=(const Value& value);

			// check value type
			inline bool is_undefined() const;
			inline bool is_object() const;
			inline bool is_array() const;
			inline bool is_string() const;
			inline bool is_integer() const;
			inline bool is_real() const;
			inline bool is_number() const;
			inline bool is_true() const;
			inline bool is_false() const;
			inline bool is_boolean() const;
			inline bool is_null() const;

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

			// write the value to a file
			inline int save_file(const char* path, int flags = 0) const;

			// write the value to a string (caller must deallocate with free()!)
			inline char* save_string(int flags = 0) const;
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
			inline ~Basic();

			// copy an existing Value
			inline Basic& operator=(const Basic& e);

			// get the underlying json_t
			inline json_t* as_json() const;

		protected:
			// take ownership of a json_t (does not increase reference count)
			static inline Basic _take(json_t* json);

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
			inline json_t* as_json() const;

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
			inline json_t* as_json() const;

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
		static inline Value from(const char* value);
		static inline Value from(const std::string& value);
		static inline Value from(bool value);
		static inline Value from(signed int value);
		static inline Value from(unsigned int value);
		static inline Value from(signed short value);
		static inline Value from(unsigned short value);
		static inline Value from(signed long value);
		static inline Value from(unsigned long value);
		static inline Value from(float value);
		static inline Value from(double value);

		// create a new empty object
		static inline Value object();

		// create a new empty array
		static inline Value array();

		// create a new null value
		static inline Value null();

		// load a file as a JSON value
		static inline Value load_file(const char* path, json_error_t* error = 0);

		// load a string as a JSON value
		static inline Value load_string(const char* string, json_error_t* error = 0);
	};

	// iterators over a JSON object
	class Iterator {
	public:
		// construct a new iterator for a given object
		inline Iterator(const Value& value);

		// construct a new iterator for a given object
		inline Iterator(const _private::ValueBase<_private::PropertyProxy>& value);

		// increment iterator
		inline void next();

		inline Iterator& operator++();

		// test if iterator is still valid
		inline bool valid() const;

		inline operator bool() const;

		// get key
		inline const char* ckey() const;

		inline std::string key() const;

		// get value
		inline const Value value() const;

		// dereference value
		inline const Value operator*() const;

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
inline std::ostream& operator<<(std::ostream& os, const jansson::Value& value);

// read JSON value
inline std::istream& operator>>(std::istream& is, jansson::Value& value);

#include "janssonxx.tcc"

#endif
