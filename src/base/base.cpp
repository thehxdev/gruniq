#define GRUNIQ_UNUSED(v) ((void)(v))

#define GRUNIQ_TIME_SPENT(__block, __elapsed) \
    do { \
        clock_t __clock_start, __clock_end; \
        __clock_start = clock(); \
        do { __block; } while (0); \
        __clock_end = clock(); \
        __elapsed = ((double)(__clock_end - __clock_start)) * 1000.0 / CLOCKS_PER_SEC; \
    } while (0)


// Branch prediction hints
#if defined(__GNUC__) || defined(__clang__)
    #define GRUNIQ_EXPECT(expr, val) __builtin_expect((expr), (val))
#else
    #define GRUNIQ_EXPECT(expr, val) (expr)
#endif
#define GRUNIQ_LIKELY(expr)    GRUNIQ_EXPECT(expr, 1)
#define GRUNIQ_UNLIKELY(expr)  GRUNIQ_EXPECT(expr, 0)


#if defined(__GNUC__) || defined(__clang__)
    #define GRUNIQ_TRAP __builtin_trap()
    #define GRUNIQ_PACKED(__decl__) __decl__ __attribute__((packed))
#elif defined (_MSC_VER)
    #define GRUNIQ_TRAP __debugbreak()
    #define GRUNIQ_PACKED(__decl__) __pragma(pack(push, 1)) __decl__ __pragma(pack(pop))
#else
    #define GRUNIQ_TRAP (*(volatile char*)0)
#endif


#ifdef NDEBUG
    #define gruniq_trap_assert(cond)
#else
    #define gruniq_trap_assert(cond) \
        do { \
            if (!(cond)) { \
                fprintf(stderr, "%s(%d): trap_assert(" #cond ")", __FILE__, __LINE__); \
                GRUNIQ_TRAP; \
            } \
        } while (0)
#endif


#define GRUNIQ_LOGI(format, ...)                                \
    do {                                                        \
        fprintf(stderr, "[ INFO ] " format, ## __VA_ARGS__);    \
        fflush(stderr);                                         \
    } while (0)


#define GRUNIQ_LOGE(format, ...)                                \
    do {                                                        \
        fprintf(stderr, "[ ERROR ] " format, ## __VA_ARGS__);   \
        fflush(stderr);                                         \
    } while (0)


#include "string.cpp"
