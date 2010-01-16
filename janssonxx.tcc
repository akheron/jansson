// get size of array or object
template <typename _Base>
unsigned int jansson::_ValueBase<_Base>::size() const {
	if (is_object())
		return json_object_size(_Base::as_json());
	else
		return json_array_size(_Base::as_json());
}

// get value at array index (const version)
template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::at(unsigned int index) const {
	return jansson::Value(json_array_get(_Base::as_json(), index));
}

template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::operator[](signed int index) const { return at(index); }
template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::operator[](unsigned int index) const { return at(index); }
template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::operator[](signed short index) const { return at(index); }
template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::operator[](unsigned short index) const { return at(index); }
template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::operator[](signed long index) const { return at(index); }
template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::operator[](unsigned long index) const { return at(index); }

// get value at array index (non-const version)
template <typename _Base>
jansson::_ValueBase<jansson::_ArrayProxy> jansson::_ValueBase<_Base>::at(unsigned int index) {
	return _ArrayProxy(_Base::as_json(), index);
}

template <typename _Base>
jansson::_ValueBase<jansson::_ArrayProxy> jansson::_ValueBase<_Base>::operator[](signed int index) { return at(index); }
template <typename _Base>
jansson::_ValueBase<jansson::_ArrayProxy> jansson::_ValueBase<_Base>::operator[](unsigned int index) { return at(index); }
template <typename _Base>
jansson::_ValueBase<jansson::_ArrayProxy> jansson::_ValueBase<_Base>::operator[](signed short index) { return at(index); }
template <typename _Base>
jansson::_ValueBase<jansson::_ArrayProxy> jansson::_ValueBase<_Base>::operator[](unsigned short index) { return at(index); }
template <typename _Base>
jansson::_ValueBase<jansson::_ArrayProxy> jansson::_ValueBase<_Base>::operator[](signed long index) { return at(index); }
template <typename _Base>
jansson::_ValueBase<jansson::_ArrayProxy> jansson::_ValueBase<_Base>::operator[](unsigned long index) { return at(index); }

// get object property (const version)
template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::get(const char* key) const {
	return jansson::Value(json_object_get(_Base::as_json(), key));
}

template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::get(const std::string& key) const { return get(key.c_str()); }
template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::operator[](const char* key) const { return get(key); }
template <typename _Base>
const jansson::Value jansson::_ValueBase<_Base>::operator[](const std::string& key) const { return get(key.c_str()); }

// get object property (non-const version)
template <typename _Base>
jansson::_ValueBase<jansson::_ObjectProxy> jansson::_ValueBase<_Base>::get(const char* key) {
	return _ObjectProxy(_Base::as_json(), key);
}

template <typename _Base>
jansson::_ValueBase<jansson::_ObjectProxy> jansson::_ValueBase<_Base>::get(const std::string& key) { return get(key.c_str()); }
template <typename _Base>
jansson::_ValueBase<jansson::_ObjectProxy> jansson::_ValueBase<_Base>::operator[](const char* key) { return get(key); }
template <typename _Base>
jansson::_ValueBase<jansson::_ObjectProxy> jansson::_ValueBase<_Base>::operator[](const std::string& key) { return get(key.c_str()); }

// clear all array/object values
template <typename _Base>
void jansson::_ValueBase<_Base>::clear() {
	if (is_object())
		json_object_clear(_Base::as_json());
	else
		json_array_clear(_Base::as_json());
}

// get value cast to specified type
template <typename _Base>
const char* jansson::_ValueBase<_Base>::as_cstring() const { return json_string_value(_Base::as_json()); }
template <typename _Base>
std::string jansson::_ValueBase<_Base>::as_string() const {
	const char* tmp = as_cstring();
	return tmp == 0 ? "" : tmp;
}
template <typename _Base>
int jansson::_ValueBase<_Base>::as_integer() const { return json_integer_value(_Base::as_json()); }
template <typename _Base>
double jansson::_ValueBase<_Base>::as_real() const { return json_real_value(_Base::as_json()); }
template <typename _Base>
double jansson::_ValueBase<_Base>::as_number() const { return json_number_value(_Base::as_json()); }
template <typename _Base>
bool jansson::_ValueBase<_Base>::as_boolean() const { return is_true(); }

// set an object property (converts value to object is not one already)
template <typename _Base>
 _Base& jansson::_ValueBase<_Base>::set_key(const char* key, const jansson::Value& value) {
	json_object_set(_Base::as_json(), key, value._Base::as_json());
	return *this;
}

template <typename _Base>
 _Base& jansson::_ValueBase<_Base>::set_key(const std::string& key, const jansson::Value& value) {
	return set_key(key.c_str(), value);
}

// set an array index (converts value to object is not one already)
template <typename _Base>
 _Base& jansson::_ValueBase<_Base>::set_at(unsigned int index, const jansson::Value& value) {
	if (index == size())
		json_array_append(_Base::as_json(), value._Base::as_json());
	else
		json_array_set(_Base::as_json(), index, value._Base::as_json());
	return *this;
}

// delete an object key
template <typename _Base>
 _Base& jansson::_ValueBase<_Base>::del_key(const char* key) {
	json_object_del(_Base::as_json(), key);
	return *this;
}

template <typename _Base>
 _Base& jansson::_ValueBase<_Base>::del_key(const std::string& key) {
	return del_key(key.c_str());
}

// delete an item from an array by index
template <typename _Base>
 _Base& jansson::_ValueBase<_Base>::del_at(unsigned int index) {
	json_array_remove(_Base::as_json(), index);
	return *this;
}

// insert an item into an array at a given index
template <typename _Base>
 _Base& jansson::_ValueBase<_Base>::insert_at(unsigned int index, const jansson::Value& value) {
	json_array_insert(_Base::as_json(), index, value._Base::as_json());
	return *this;
}

// assign value to proxied array element
jansson::_ArrayProxy& jansson::_ArrayProxy::operator=(const Value& value) {
	json_array_set(_array, _index, value.as_json());
	return *this;
}

// assign value to proxied object property
jansson::_ObjectProxy& jansson::_ObjectProxy::operator=(const Value& value) {
	json_object_set(_object, _key, value.as_json());
	return *this;
}
