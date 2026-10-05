#ifndef PI_COMMON_H
#define PI_COMMON_H

#ifdef __cplusplus
    #define PI_CPP __cplusplus
#endif

#if _MSC_VER
    #define PI_INLINE_C static __forceinline
    #define PI_ALIGN_AS(n) __declspec(align(n))
#elif __GNUC__
    #define PI_INLINE_C static inline __attribute__((always_inline))
    #define PI_ALIGN_AS(n) __attribute__((aligned(n)))
#else
    #define PI_INLINE_C static inline
    #define PI_ALIGN_AS(n)
#endif

#ifndef PI_CPP
    #include <stdint.h>
    #include <stdarg.h>
    #include <stdbool.h>
    #include <assert.h>

    #define PI_EXTERN_C extern
    #define PI_CONSTEXPR_F PI_INLINE_C /* PI_INLINE_C gonna be defined after */
    #define PI_CONSTEXPR_V static const 
    #define PI_MEMBER_DEFAULT(...)
#else
    #include <cstdint>
    #include <cstdarg>
    #include <new> /* <new> is included by default to allow the use for placement new on custom game classes without having to include this header */
    #include <cassert>

    #define PI_EXTERN_C extern "C"
    #define PI_CONSTEXPR_F constexpr
    #define PI_CONSTEXPR_V PI_CONSTEXPR_F

    #undef PI_ALIGN_AS
    #define PI_ALIGN_AS alignas

    #define PI_MEMBER_DEFAULT(...)  = __VA_ARGS__
#endif


/* Typedefs */

typedef struct PI_GAME *PI_HGAME;
typedef struct PI_SCENE_IMPL *PI_HSCENE;

/* Some helper functions */

#define PI_ASSERT assert

#define PI_ALIGN(n, a) ( ((n) + ((a) - 1)) / (a) * (a) )
#define PI_ARRAYSIZE(a) ( sizeof((a)) / sizeof(*(a)) )

#endif
