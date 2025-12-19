#define __gruniq_base_aligned_copy_info(__buffer, __n)                        \
    __word_t *__buffer##_word = (__word_t*) __buffer;                         \
    __word_t *__buffer##_word_end = __buffer##_word + ((__n) >> SHIFT_N_BY);  \
    uint8_t *__buffer##_byte = (uint8_t*) __buffer##_word_end;                \
    uint8_t *__buffer##_byte_end = __buffer##_byte + ((__n) & LAST_BYTES_ADDR_MASK);


namespace gruniq::base {

typedef unsigned long long __word_t;

static constexpr size_t WORD_BIT_SIZE        = sizeof(__word_t) << 3U;
static constexpr size_t SHIFT_N_BY           = (WORD_BIT_SIZE >> 5U) + 1U;
static constexpr size_t LAST_BYTES_ADDR_MASK = (WORD_BIT_SIZE >> 3U) - 1U;

// Copy data in word-size chunks.
void *memcpy_fast(void *dest, const void *src, size_t n) {
    __gruniq_base_aligned_copy_info(src, n);
    __gruniq_base_aligned_copy_info(dest, n);
    GRUNIQ_UNUSED(dest_byte_end);

    while (src_word != src_word_end) {
        *dest_word = *src_word;
        dest_word++;
        src_word++;
    }
    while (src_byte != src_byte_end) {
        *dest_byte = *src_byte;
        dest_byte++;
        src_byte++;
    }

    return dest;
}

void *memmove_fast(void *dest, const void *src, size_t n) {
    if (dest < src)
        return memcpy_fast(dest, src, n);

    __gruniq_base_aligned_copy_info(src, n);
    __gruniq_base_aligned_copy_info(dest, n);

    dest_byte_end--;
    dest_word_end--;
    src_byte_end--;
    src_word_end--;

    while (src_byte < src_byte_end) {
        *dest_byte_end = *src_byte_end;
        dest_byte_end--;
        src_byte_end--;
    }
    while (src_word <= src_word_end) {
        *dest_word_end = *src_word_end;
        dest_word_end--;
        src_word_end--;
    }

    return dest;
}

} // end namespace gruniq::base
