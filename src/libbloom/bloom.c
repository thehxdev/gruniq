/*
 * This a forked version of libbloom with modifications to make the library work
 * on different platforms and simplified source code. Now this library can be used
 * as a self-contained C library, embeddable in other projects.
 */

/*
 *  Copyright (c) 2012-2022, Jyri J. Virkki
 *  All rights reserved.
 *
 *  This file is under BSD license. See LICENSE file.
 */

/*
 * Refer to bloom.h for documentation on the public interfaces.
 */

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bloom.h"

#define __BLOOM_MAGIC "libbloom3"

#define __bloom_concat_(A,B) A##B
#define __bloom_concat(A,B) __bloom_concat_(A,B)
#define __bloom_static_assert(condition, id) \
    extern char __bloom_concat(id, __LINE__)[ ((condition)) ? 1 : -1 ]

//-----------------------------------------------------------------------------
// __bloom_murmurhash2, by Austin Appleby

// Note - This code makes a few assumptions about how your machine behaves -

// 1. We can read a 4-byte value from any address without crashing
// 2. sizeof(int) == 4
__bloom_static_assert(sizeof(int) == 4, sizeof_int_must_be_4_bytes);

// And it has a few limitations -

// 1. It will not work incrementally.
// 2. It will not produce the same results on little-endian and big-endian
//    machines.

static unsigned int __bloom_murmurhash2(const void *key, int len, const unsigned int seed)
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.

    const unsigned int m = 0x5bd1e995;
    const int r = 24;

    // Initialize the hash to a 'random' value

    unsigned int h = seed ^ len;

    // Mix 4 bytes at a time into the hash

    const unsigned char *data = (const unsigned char *)key;

    while(len >= 4) {
        unsigned int k = *(unsigned int *)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    // Handle the last few bytes of the input array

    switch(len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
            h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}
// End __bloom_murmurhash2
//-----------------------------------------------------------------------------


inline static int __bloom_test_bit_set_bit(unsigned char *buf,
                                           unsigned long bit,
                                           int set_bit)
{
    unsigned long byte = bit >> 3;
    unsigned char c = buf[byte];
    unsigned char mask = 1 << (bit % 8ul);
    if (c & mask) {
        return 1;
    } else {
        if (set_bit) {
            buf[byte] = c | mask;
        }
        return 0;
    }
}


static int __bloom_check_add(struct bloom *bloom,
                             const void *buffer,
                             int len,
                             int add)
{
    assert(bloom->ready != 0);

    unsigned char hits = 0;
    unsigned int a = __bloom_murmurhash2(buffer, len, 0x9747b28c);
    unsigned int b = __bloom_murmurhash2(buffer, len, a);
    unsigned long int x;
    unsigned long int i;

    for (i = 0; i < bloom->hashes; i++) {
        x = (a + b*i) % bloom->bits;
        if (__bloom_test_bit_set_bit(bloom->bf, x, add))
            hits++;
        else if (!add)
            // Don't care about the presence of all the bits. Just our own.
            return 0;
    }

    if (hits == bloom->hashes)
        return 1;

    return 0;
}


int bloom_init(struct bloom *bloom, unsigned int entries, double error)
{
    __bloom_static_assert(sizeof(unsigned long long) == 8, test_sizeof_unsigned_long_long);

    memset(bloom, 0, sizeof(struct bloom));
    if (entries < 1000 || error <= 0 || error >= 1)
        return 1;

    bloom->entries = entries;
    bloom->error = error;

    const double num = -log(bloom->error);
    const double denom = 0.480453013918201; // ln(2)^2
    bloom->bpe = (num / denom);

    const long double dentries = (long double)entries;
    const long double allbits = dentries * bloom->bpe;
    bloom->bits = (unsigned long int)allbits;

    if (bloom->bits % 8)
        bloom->bytes = (bloom->bits / 8) + 1;
    else
        bloom->bytes = bloom->bits / 8;

    bloom->hashes = (unsigned char)ceil(0.693147180559945 * bloom->bpe); // ln(2)

    bloom->bf = (unsigned char *)calloc(bloom->bytes, sizeof(unsigned char));
    if (bloom->bf == NULL)
        return 1;

    bloom->ready = 1;

    bloom->major = BLOOM_VERSION_MAJOR;
    bloom->minor = BLOOM_VERSION_MINOR;

    return 0;
}


int bloom_check(struct bloom *bloom, const void *buffer, int len)
{
    return __bloom_check_add(bloom, buffer, len, 0);
}


int bloom_add(struct bloom *bloom, const void *buffer, int len)
{
    return __bloom_check_add(bloom, buffer, len, 1);
}


void bloom_print(struct bloom *bloom)
{
    printf("bloom at %p\n", (void *)bloom);
    if (!bloom->ready)
        printf(" *** NOT READY ***\n");
    printf(" ->version = %d.%d\n", bloom->major, bloom->minor);
    printf(" ->entries = %u\n", bloom->entries);
    printf(" ->error = %f\n", bloom->error);
    printf(" ->bits = %lu\n", bloom->bits);
    printf(" ->bits per elem = %f\n", bloom->bpe);
    printf(" ->bytes = %lu", bloom->bytes);
    unsigned int KB = bloom->bytes / 1024;
    unsigned int MB = KB / 1024;
    printf(" (%u KB, %u MB)\n", KB, MB);
    printf(" ->hash functions = %d\n", bloom->hashes);
}


void bloom_free(struct bloom *bloom)
{
    if (bloom->ready)
        free(bloom->bf);
    bloom->ready = 0;
}


int bloom_reset(struct bloom *bloom)
{
    if (!bloom->ready)
        return 1;
    memset(bloom->bf, 0, bloom->bytes);
    return 0;
}


int bloom_save(struct bloom *bloom, char *filename)
{
    if (filename == NULL || filename[0] == 0)
        return 1;

    FILE *fp = fopen(filename, "wb");
    if (!fp)
        return 1;

    uint16_t size;
    int err = 0;
    size_t bloom_magic_length = strlen(__BLOOM_MAGIC);
    size_t out = fwrite(__BLOOM_MAGIC, sizeof(char), bloom_magic_length, fp);
    if (out != bloom_magic_length) {
        err = 1;
        goto save_error;
    }

    size = sizeof(struct bloom);
    out = fwrite(&size, sizeof(char), sizeof(uint16_t), fp);
    if (out != sizeof(uint16_t)) {
        err = 1;
        goto save_error;
    }

    out = fwrite(bloom, sizeof(char), sizeof(struct bloom), fp);
    if (out != sizeof(struct bloom)) {
        err = 1;
        goto save_error;
    }

    out = fwrite(bloom->bf, sizeof(char), bloom->bytes, fp);
    if (out != bloom->bytes)
        err = 1;

save_error:
    fclose(fp);
    return err;
}


int bloom_load(struct bloom *bloom, char *filename)
{
    int rv = 0;

    if (filename == NULL || filename[0] == 0)
        return 1;
    if (bloom == NULL)
        return 2;

    memset(bloom, 0, sizeof(struct bloom));

    FILE *fp = fopen(filename, "rb");
    if (!fp)
        return 3;

    char line[30];
    memset(line, 0, 30);
    const size_t bloom_magic_length = strlen(__BLOOM_MAGIC);
    size_t in = fread(line, sizeof(char), bloom_magic_length, fp);

    if (in != bloom_magic_length) {
        rv = 4;
        goto load_error;
    }

    if (strncmp(line, __BLOOM_MAGIC, bloom_magic_length)) {
        rv = 5;
        goto load_error;
    }

    uint16_t size;
    in = fread(&size, sizeof(char), sizeof(uint16_t), fp);
    if (in != sizeof(uint16_t)) {
        rv = 6;
        goto load_error;
    }

    if (size != sizeof(struct bloom)) {
        rv = 7;
        goto load_error;
    }

    in = fread(bloom, sizeof(char), sizeof(struct bloom), fp);
    if (in != sizeof(struct bloom)) {
        rv = 8;
        goto load_error;
    }

    bloom->bf = NULL;
    if (bloom->major != BLOOM_VERSION_MAJOR) {
        rv = 9;
        goto load_error;
    }

    bloom->bf = (unsigned char *)malloc(bloom->bytes);
    if (bloom->bf == NULL) {
        rv = 10;
        goto load_error;
    }

    in = fread(bloom->bf, sizeof(char), bloom->bytes, fp);
    if (in != bloom->bytes) {
        rv = 11;
        free(bloom->bf);
        bloom->bf = NULL;
        goto load_error;
    }

    fclose(fp);
    return rv;

load_error:
    fclose(fp);
    bloom->ready = 0;
    return rv;
}


int bloom_merge(struct bloom *bloom_dest, struct bloom *bloom_src)
{
    assert(bloom_dest->ready != 0);
    assert(bloom_src->ready != 0);

    if (bloom_dest->entries != bloom_src->entries)
        return 1;

    if (bloom_dest->error != bloom_src->error)
        return 1;

    if (bloom_dest->major != bloom_src->major)
        return 1;

    if (bloom_dest->minor != bloom_src->minor)
        return 1;

    // Not really possible if properly used but check anyway to avoid the
    // possibility of buffer overruns.
    if (bloom_dest->bytes != bloom_src->bytes)
        return 1;

    unsigned long int p;
    for (p = 0; p < bloom_dest->bytes; p++)
        bloom_dest->bf[p] |= bloom_src->bf[p];

    return 0;
}
