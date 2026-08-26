#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifndef  NOBBER_HASH_SET_DEFAULT_BUCKET_COUNT
#define NOBBER_HASH_SET_DEFAULT_BUCKET_COUNT 17
#endif

#define NOBBER_ERROR_OUT_OF_MEMORY(PTR)                                                          \
    if (PTR == NULL) {                                                                                  \
        fprintf (stderr, "[ERROR] Could not allocate memory: %s.\nExiting.\n", strerror(errno)); \
        exit (errno);                                                                            \
    }

#define str_hash djb2

typedef size_t (*NobberHashFunc)(void * data);
typedef size_t (*NobberEqualFunc)(void * l, void * r);
typedef bool (*NobberForeachFunc)(void * data, void * user_data);

typedef struct NobberHashSetItem NobberHashSetItem;
struct NobberHashSetItem {
    void * data;
    size_t hash;
    NobberHashSetItem * next;
    // Since we are not ordering the lists, we don't need this.
    //NobberHashSetItem * prev;
};

typedef struct {
    NobberHashSetItem * head;
    NobberHashSetItem * tail;
    //NobberHashSetItem * cur;
    size_t count;
} NobberHashSetBucket;

typedef struct {
    NobberHashSetBucket * buckets;
    NobberHashFunc hashfunc;
    NobberEqualFunc equalfunc;
    size_t bucket_count; // This must not change after a new NobberHashSet has been created.
} NobberHashSet;


/* Create a new hash set.
 *
 * @param hashfunc            A function to hash data with.
 * @param equalfunc           A Function to check if two items are equal.
 * @param number_of_buckets   The number of buckets to use. if zero, #NOBBER_HASH_SET_DEFAULT_BUCKET_COUNT will bu used.
 *
 * @Note   The nuber of buckets should be a prime number. But it is not required.
 */
NobberHashSet * nobber_hash_set_new (NobberHashFunc hashfunc, NobberEqualFunc equalfunc, size_t number_of_buckets);

void nobber_hash_set_add (NobberHashSet * set, void * data);

size_t nobber_hash_set_get_count (NobberHashSet * set);

void ** nobber_hash_set_get_data (NobberHashSet * set);

void nobber_hash_set_foreach (NobberHashSet * set, NobberForeachFunc foreach_func, void * user_data);


//Hashing algos taken from: http://www.cse.yorku.ca/~oz/hash.html

// djb2 hashing algo
static unsigned long djb2 (unsigned char *str);

// a public-domain reimplementation of ndbm.
static unsigned long sdbm(unsigned char * str);

// K&R hashing algo
static unsigned long knr_hash (unsigned char *str);


#ifdef NOBBER_HASH_SET_IMPREMENTATION
#undef NOBBER_HASH_SET_IMPREMENTATION

static unsigned long
djb2 (unsigned char * str) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static unsigned long
sdbm(unsigned char * str) {
    unsigned long hash = 0;
    int c;

    while (c = *str++)
        hash = c + (hash << 6) + (hash << 16) - hash;

    return hash;
}

static unsigned long
knr_hash (unsigned char * str) {
    unsigned int hash = 0;
    int c;

    while (c = *str++)
        hash += c;

    return hash;
}

NobberHashSet * nobber_hash_set_new (NobberHashFunc hashfunc, NobberEqualFunc equalfunc, size_t bucket_count) {
    NobberHashSet * set = (NobberHashSet*) malloc(sizeof (NobberHashSet));
    set = (NobberHashSet*) memset (set, 0, sizeof (NobberHashSet));

    set->hashfunc = hashfunc;
    set->equalfunc = equalfunc;
    set->bucket_count = bucket_count;

    NOBBER_ERROR_OUT_OF_MEMORY(set);
    if (bucket_count == 0) {
        set->bucket_count = NOBBER_HASH_SET_DEFAULT_BUCKET_COUNT;
    } else {
        set->bucket_count = bucket_count;
    }
    set->buckets = (NobberHashSetBucket*) malloc  (sizeof (NobberHashSetBucket) * set->bucket_count + 1);
    set->buckets = (NobberHashSetBucket*) memset (set->buckets, 0, sizeof (NobberHashSetBucket) * set->bucket_count + 1);


    return set;
}

void
nobber_hash_set_add (NobberHashSet * set, void * data) {
    if (data == NULL) {
        fprintf (stdout, "[WARNING] Trying to add null-data to hash set. Returning.\n");
        return;
    }
    NobberHashFunc hashfunc = set->hashfunc;
    NobberEqualFunc equalfunc = set->equalfunc;

    // Find a bucket to put data into.
    size_t hash = hashfunc(data);
    //modulus to get the correct bucket.
    size_t bucket_nr = hash % set->bucket_count;
    NobberHashSetBucket * bucket = &set->buckets[bucket_nr];

    if (bucket->head == NULL) {
        // Special case when the list is empty
        NobberHashSetItem * item = (NobberHashSetItem*) malloc (sizeof (NobberHashSetItem));
        bucket->head = item;
        bucket->tail = item;
        item->data = data;
        item->hash = hash;
        item->next = NULL;

        return;
        //goto finish;
    }

    NobberHashSetItem * item = bucket->head;
    NobberHashSetItem * end = bucket->tail;


    // See if the bucket has our data in it. (Well, according to the hash function).
    do {
        if (hash == item->hash) {
            // We found the something with the same hash.
            if (equalfunc (item->data, data) == 0) {
                //it has the same data too. Nothing to do.
                return;
            }
        }
        // Next!
        item = item->next;
    } while (item);
    // Same data not found.

    NobberHashSetItem * new_item = (NobberHashSetItem*) malloc (sizeof (NobberHashSetItem));
    end->next = new_item;
finish:
    new_item->data = data;
    new_item->hash = hash;
    bucket->tail = new_item;
    bucket->count += 1;
}

size_t
nobber_hash_set_get_count (NobberHashSet * set) {
    size_t count = 0;
    for (size_t i; i <= set->bucket_count; i++) {
        count += set->buckets[i].count;
    }
    return count;
}


void nobber_hash_set_foreach (NobberHashSet * set, NobberForeachFunc foreach_func, void * user_data) {
    for (size_t i = 0; i <= set->bucket_count; i++) {
        NobberHashSetBucket * bucket = &set->buckets[i];
        if (bucket == NULL) continue;
        NobberHashSetItem * item = bucket->head;
        NobberHashSetItem * end = bucket->tail;
        if (item == NULL) continue;
        do {
            foreach_func (item->data, user_data);
            item = item->next;
        } while (item);
    }
}


struct Nobber__GetDataForeachFuncData {
    void ** arr;
    size_t index;
    size_t max;
};

bool nobber__foreach_get_data (void * data, struct Nobber__GetDataForeachFuncData * user_data ) {
    assert (user_data->index <= user_data->max);
    user_data->arr[user_data->index] = data;
    user_data += 1;

    return true;
}

void ** nobber_hash_set_get_data (NobberHashSet * set) {
    size_t count = nobber_hash_set_get_count (set);
    void ** arr = malloc (sizeof (void *) * count);
    struct Nobber__GetDataForeachFuncData func_data = {
        .arr = arr,
        .index = 0,
        .max = count,
    };

    nobber_hash_set_foreach (set, (NobberForeachFunc) nobber__foreach_get_data, &func_data);

    return arr;
}


#endif
#undef NOBBER_ERROR_OUT_OF_MEMORY
