#ifndef _STH_ALLOCATORS_MEMPOOL_H_
#define _STH_ALLOCATORS_MEMPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

union __sth_mempool_node {
    union __sth_mempool_node *next;
};

typedef struct {
    sth_arena_t *arena;
    union __sth_mempool_node *list;
    size_t size;
} sth_mempool_t;

void sth_mempool_init(sth_mempool_t *self, sth_arena_t *arena, size_t size);

void *sth_mempool_get(sth_mempool_t *self);

void sth_mempool_put(sth_mempool_t *self, void *v);

#ifdef __cplusplus
}
#endif

#endif // _STH_ALLOCATORS_MEMPOOL_H_
