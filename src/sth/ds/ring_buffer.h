#ifndef _STH_DS_RING_BUFFER_H_
#define _STH_DS_RING_BUFFER_H_

// https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/

#define sth_ds_ring_buffer_mask(cap, v) ((v) & ((cap) - 1))

#define sth_ds_ring_buffer_init(rb, backing_buffer, capacity) \
    do { \
        size_t _cap_ = (capacity); \
        STH_BASE_ASSERT((_cap_ & (_cap_-1)) == 0 && "ring buffer capacity must be a power of 2"); \
        (rb)->items = STH_BASE_DECLTYPE((rb)->items) (backing_buffer); \
        (rb)->read = 0; \
        (rb)->write = 0; \
        (rb)->cap = _cap_; \
    } while (0)

#define sth_ds_ring_buffer_is_empty(rb) \
    ((rb)->read == (rb)->write)

#define sth_ds_ring_buffer_size(rb) \
    ((rb)->write - (rb)->read)

#define sth_ds_ring_buffer_is_full(rb) \
    (sth_ds_ring_buffer_size((rb)) == (rb)->cap)

#define sth_ds_ring_buffer_push(rb, item) \
    ((!sth_ds_ring_buffer_is_full((rb))) \
     ? ((rb)->write += 1, (rb)->items[sth_ds_ring_buffer_mask((rb)->cap, (rb)->write)] = (item), STH_OK) \
     : STH_FAILED)

#define sth_ds_ring_buffer_shift(rb) \
    ((!sth_ds_ring_buffer_is_empty((rb))) \
     ? ((rb)->read += 1, (rb)->items[sth_ds_ring_buffer_mask((rb)->cap, (rb)->read)], STH_OK) \
     : STH_FAILED)

#define sth_ds_ring_buffer_shift_out(rb, out) \
    ((!sth_ds_ring_buffer_is_empty((rb))) \
     ? ((rb)->read += 1, (out) = (rb)->items[sth_ds_ring_buffer_mask((rb)->cap, (rb)->read)], STH_OK) \
     : STH_FAILED)

#endif // _STH_DS_RING_BUFFER_H_
