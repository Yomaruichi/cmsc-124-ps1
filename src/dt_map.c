/*
 * dt_map.c: Associative arrays for Unit 5, Section E.
 *
 * An array does not store its indices. This map stores its keys.
 * An array calculates a position with one subtraction.
 * The map calculates a hash and then compares keys in one bucket.
 *
 * Hashing turns the key into a bucket number. Compare all keys in that bucket
 * because two keys can select it. Use a linked list for each bucket. Start the
 * unsigned accumulator at 14695981039346656037ULL. For each unsigned byte,
 * exclusive-or the byte into it and multiply by 1099511628211ULL.
 *
 * A separate list stores insertion order for stable output. dt_map_key_at
 * reads this list. Updating a key preserves its position. Removing and
 * reinserting a key moves it to the end.
 */

#include "dt.h"

#include <stdlib.h>
#include <string.h>

#define DT_MAP_BUCKETS 16

typedef struct dt_map_entry {
    char *key;
    dt_value value;
    struct dt_map_entry *next;
} dt_map_entry;

struct dt_map {
    dt_map_entry *buckets[DT_MAP_BUCKETS];
    dt_map_entry **order;
    size_t len;
    size_t order_capacity;
};

size_t dt_map_hash(const char *key)
{
    unsigned long long hash = 14695981039346656037ULL;

    while (*key != '\0') {
        hash ^= (unsigned char)*key;
        hash *= 1099511628211ULL;
        key++;
    }

    return (size_t)hash;
}

/*
 * dt_map_new builds an empty map. It returns NULL after an allocation failure.
 */
dt_map *dt_map_new(void)
{
    /* TODO: Return an allocated empty map. Return NULL after an allocation failure.
       dt_map_new()  -> a map whose dt_map_len is 0
       cases/normal/map_basics.case */

    dt_map *map = malloc(sizeof(dt_map));


    if (map == NULL) return NULL;

    for (size_t i = 0; i < DT_MAP_BUCKETS; i++){
      map->buckets[i] = NULL;
    }
    map->order = NULL;
    map->len = 0;
    map->order_capacity = 0;


    return map;
}

/*
 * dt_map_free releases each entry, copied key, order array, and map.
 * It accepts NULL. The environment owns the values.
 */
void dt_map_free(dt_map *m)
{
    /* TODO: Release each entry, copied key, order array, and map.
       Preserve the values. The environment owns them.
       a map holding a string value  -> the nodes and keys go, the string stays
       dt_map_free(NULL)             -> returns, having done nothing
       cases/cleanup/map_churn.case */
    
       if (m == NULL) return;

       for (size_t i = 0; i < DT_MAP_BUCKETS; i++){
        
        dt_map_entry *entry = m->buckets[i];
        while (entry != NULL) {
            dt_map_entry *next = entry->next;

            free(entry->key);
            free(entry);

            entry = next;
        }
       }

       free(m->order);
       free(m);

}

/*
 * dt_map_len returns the number of keys in constant time.
 */
size_t dt_map_len(const dt_map *m)
{
    /* TODO: Return the current key count.
       Replacing a value does not change this count.
       after put alpha, beta, gamma:  dt_map_len(m) -> 3
       after put beta again:          dt_map_len(m) -> 3, still
       after del alpha:               dt_map_len(m) -> 2
       cases/normal/map_basics.case */
    return m->len;
}

/*
 * dt_map_put binds v to key.
 * An existing key keeps its insertion position. A new key becomes the last key.
 * Copy each new key because the caller owns the source buffer.
 * Return DT_ERR_CAPACITY after an allocation failure.
 */
dt_status dt_map_put(dt_map *m, const char *key, dt_value v)
{
    /* TODO: Replace the value for an existing key.
       Add a new entry for a new key. Copy each new key.
       Hash the key. Select its bucket. Search the bucket chain.
       Add a new entry to the chain and insertion list.
       put "beta" -> 2 on an empty map    -> DT_OK, "beta" is last in order
       put "beta" -> 22 on that map       -> DT_OK, same position, new value
       an allocation failure              -> DT_ERR_CAPACITY, map unchanged
       cases/normal/map_basics.case */
 size_t bucket = dt_map_hash(key) % DT_MAP_BUCKETS;

    /* Search for an existing key. */
    dt_map_entry *entry = m->buckets[bucket];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = v;
            return DT_OK;
        }

        entry = entry->next;
    }

    /* Make sure the order array has room. */
    if (m->len == m->order_capacity) {
        size_t new_capacity;

        if (m->order_capacity == 0) {
            new_capacity = 4;
        } else {
            new_capacity = m->order_capacity * 2;
        }

        dt_map_entry **new_order =
            realloc(m->order, new_capacity * sizeof(*new_order));

        if (new_order == NULL) {
            return DT_ERR_CAPACITY;
        }

        m->order = new_order;
        m->order_capacity = new_capacity;
    }

    /* Allocate the new entry. */
    dt_map_entry *new_entry = malloc(sizeof(*new_entry));

    if (new_entry == NULL) {
        return DT_ERR_CAPACITY;
    }

    /* Copy the key. */
    new_entry->key = malloc(strlen(key) + 1);

    if (new_entry->key == NULL) {
        free(new_entry);
        return DT_ERR_CAPACITY;
    }

    strcpy(new_entry->key, key);
    new_entry->value = v;

    /* Add to bucket chain. */
    new_entry->next = m->buckets[bucket];
    m->buckets[bucket] = new_entry;

    /* Store pointer in insertion order. */
    m->order[m->len] = new_entry;
    m->len++;

    return DT_OK;
}

/*
 * dt_map_get writes the value for key to *out.
 * It returns DT_ERR_KEY and does not change *out when the key is absent.
 * An absent key differs from a nil value.
 */
dt_status dt_map_get(const dt_map *m, const char *key, dt_value *out)
{
    /* TODO: Return DT_ERR_KEY when the key is absent.
       Preserve *out after this error. A nil value can be present.
       after put "beta" -> 22:
         dt_map_get(m, "beta", &out)   -> DT_OK, *out is the integer 22
         dt_map_get(m, "ghost", &out)  -> DT_ERR_KEY, *out untouched
       cases/normal/map_basics.case, cases/boundary/map_missing_key.case */
    size_t bucket = dt_map_hash(key) % DT_MAP_BUCKETS;

    dt_map_entry *entry = m->buckets[bucket];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            *out = entry->value;
            return DT_OK;
        }

        entry = entry->next;
    }

    return DT_ERR_KEY;
}

/*
 * dt_map_remove removes key from its bucket and insertion position.
 * It releases the copied key. It returns DT_ERR_KEY when the key is absent.
 */
dt_status dt_map_remove(dt_map *m, const char *key)
{
    size_t bucket = dt_map_hash(key) % DT_MAP_BUCKETS;

    /* Find the entry in the bucket chain. */
    dt_map_entry *entry = m->buckets[bucket];
    dt_map_entry *prev = NULL;

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            break;
        }

        prev = entry;
        entry = entry->next;
    }

    if (entry == NULL) {
        return DT_ERR_KEY;
    }

    /* Remove from bucket chain. */
    if (prev == NULL) {
        m->buckets[bucket] = entry->next;
    } else {
        prev->next = entry->next;
    }

    /* Find the same entry in insertion order. */
    size_t index = 0;

    while (index < m->len && m->order[index] != entry) {
        index++;
    }

    /* Shift pointers left. */
    for (size_t i = index; i + 1 < m->len; i++) {
        m->order[i] = m->order[i + 1];
    }

    m->len--;

    /* Free the actual entry. */
    free(entry->key);
    free(entry);

    return DT_OK;
}

/*
 * dt_map_key_at writes the key at insertion position index to *out.
 * It returns DT_ERR_RANGE and does not change *out for an invalid index.
 */
dt_status dt_map_key_at(const dt_map *m, size_t index, const char **out)
{
    /* TODO: Write the key at the specified insertion position to *out.
       Return DT_ERR_RANGE for an invalid position. Preserve *out after this error.
       The printer uses this order.
       a map holding alpha, beta, gamma:
         dt_map_key_at(m, 0, &out)  -> DT_OK, *out = "alpha"
         dt_map_key_at(m, 3, &out)  -> DT_ERR_RANGE, *out untouched
       cases/normal/map_basics.case */
    if (index >= m->len) {
        return DT_ERR_RANGE;
    }

    *out = m->order[index]->key;

    return DT_OK;
}

