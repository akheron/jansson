/*
 * Copyright (c) 2009 Petri Lehtinen <petri@digip.org>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef struct hashtable hashtable_t;

typedef unsigned int (*key_hash_fn)(const void *key);
typedef int (*key_cmp_fn)(const void *key1, const void *key2);
typedef void (*free_fn)(void *key);

/**
 * hashtable_new - Create a hashtable object
 *
 * @hash_key: The key hashing function
 * @cmp_keys: The key compare function. Returns non-zero for equal and
 *     zero for unequal unequal keys
 * @free_key: If non-NULL, called for a key that is no longer referenced.
 * @free_value: If non-NULL, called for a value that is no longer referenced.
 *
 * Returns a new hashtable object that should be freed with
 * hashtable_free when it's no longer used.
 */
hashtable_t *hashtable_new(key_hash_fn hash_key, key_cmp_fn cmp_keys,
                           free_fn free_key, free_fn free_value);

/**
 * hashtable_free - Destroy a hashtable object
 *
 * @hashtable: The hashtable
 */
void hashtable_free(hashtable_t *hashtable);

/**
 * hashtable_set - Add/modify value in hashtable
 *
 * @hashtable: The hashtable object
 * @key: The key
 * @value: The value
 *
 * If a value with the given key already exists, its value is replaced
 * with the new value.
 *
 * Key and value are "stealed" in the sense that hashtable frees them
 * automatically when they are no longer used. The freeing is
 * accomplished by calling free_key and free_value functions that were
 * supplied to hashtable_new. In case one or both of the free
 * functions is NULL, the corresponding item is not "stealed".
 *
 * Returns 0 on success, -1 on failure (out of memory).
 */
int hashtable_set(hashtable_t *hashtable, void *key, void *value);

/**
 * hashtable_get - Get a value associated with a key
 *
 * @hashtable: The hashtable object
 * @key: The key
 *
 * Returns value if it is found, or NULL otherwise.
 */
void *hashtable_get(hashtable_t *hashtable, const void *key);

/**
 * hashtable_del - Remove a value from the hashtable
 *
 * @hashtable: The hashtable object
 * @key: The key
 *
 * Returns 0 on success, or -1 if the key was not found.
 */
int hashtable_del(hashtable_t *hashtable, const void *key);

/**
 * hashtable_iter - Iterate over hashtable
 *
 * @hashtable: The hashtable object
 *
 * Returns an opaque iterator to the first element in the hashtable.
 * The iterator should be passed to hashtable_iter_* functions.
 * The hashtable items are not iterated over in any particular order.
 *
 * There's no need to free the iterator in any way. The iterator is
 * valid as long as the item that is referenced by the iterator is not
 * deleted. Other values may be added or deleted. In particular,
 * hashtable_iter_next() may be called on an iterator, and after that
 * the key/value pair pointed by the old iterator may be deleted.
 */
void *hashtable_iter(hashtable_t *hashtable);

/**
 * hashtable_iter_next - Advance an iterator
 *
 * @hashtable: The hashtable object
 * @iter: The iterator
 *
 * Returns a new iterator pointing to the next element in the
 * hashtable or NULL if the whole hastable has been iterated over.
 */
void *hashtable_iter_next(hashtable_t *hashtable, void *iter);

/**
 * hashtable_iter_key - Retrieve the key pointed by an iterator
 *
 * @iter: The iterator
 */
void *hashtable_iter_key(void *iter);

/**
 * hashtable_iter_value - Retrieve the value pointed by an iterator
 *
 * @iter: The iterator
 */
void *hashtable_iter_value(void *iter);

#endif
