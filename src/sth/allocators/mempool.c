#ifdef __cplusplus
extern "C" {
#endif

void sth_mempool_init(sth_mempool_t *self, sth_arena_t *arena, size_t size) {
    size = (size >= sizeof(union __sth_mempool_node)) ? size : sizeof(union __sth_mempool_node);
    *self = (sth_mempool_t){
        .arena = arena,
        .list = NULL,
        .size = size
    };
}

void *sth_mempool_get(sth_mempool_t *self) {
    union __sth_mempool_node *c;
    if (!self->list)
        return sth_arena_alloc(self->arena, self->size);
    c = self->list;
    self->list = c->next;
    return c;
}

void sth_mempool_put(sth_mempool_t *self, void *v) {
    union __sth_mempool_node *c = (union __sth_mempool_node*) v;
    c->next = self->list;
    self->list = c;
}

#ifdef __cplusplus
}
#endif
