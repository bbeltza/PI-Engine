#ifndef PI_DEBUG_H
#define PI_DEBUG_H

#include <piengine/pi_common.h>
#include <stdio.h>

#if _MSC_VER >= 1400
    #include <sal.h>
    #if _MSC_VER > 1400
        #define _PI_PRINTF_FMTSTR _Printf_format_string_
    #else
        #define _PI_PRINTF_FMTSTR __format_string
    #endif
#else
    #define _PI_PRINTF_FMTSTR
#endif

#if _MSC_VER
#else
#endif

#if __GNUC__
    #define _PI_PRINTF_ATTR __attribute__((format (printf, 2, 3)))
#else
    #define _PI_PRINTF_ATTR
#endif

/* Logging API */
    /* Logging category */
typedef int PI_LOGCATEGORY;

#define PI_LOG_INFO 0
#define PI_LOG_WARN 1
#define PI_LOG_DEBUG 2

PI_EXTERN_C size_t PI_LOG_STR(PI_LOGCATEGORY category, const char* str);
PI_EXTERN_C size_t PI_LOG_FMTVA(PI_LOGCATEGORY category, const char* fmt, va_list va);
PI_INLINE_C size_t PI_LOG_FMT(PI_LOGCATEGORY category, _PI_PRINTF_FMTSTR const char* fmt, ...) _PI_PRINTF_ATTR
{
    va_list va;
    size_t ret;
    va_start(va, fmt);
        ret = PI_LOG_FMTVA(category, fmt, va);
    va_end(va);
    return ret;
}

#define PI_INFO_FMTVA(fmt, va) PI_LOG_FMTVA(PI_LOG_INFO, fmt, va)
#define PI_INFO_FMT(...) PI_LOG_FMT(PI_LOG_INFO, __VA_ARGS__)
#define PI_INFO_STR(str) PI_LOG_STR(PI_LOG_INFO, str)

#define PI_WARN_FMTVA(fmt, va) PI_LOG_FMTVA(PI_LOG_WARN, fmt, va)
#define PI_WARN_FMT(...) PI_LOG_FMT(PI_LOG_WARN, __VA_ARGS__)
#define PI_WARN_STR(str) PI_LOG_STR(PI_LOG_WARN, str)

#ifdef NDEBUG
    #define PI_DEBUG_FMTVA(fmt, va)
    #define PI_DEBUG_FMT(...)
    #define PI_DEBUG_STR(str)
#else
    #define PI_DEBUG_FMTVA(fmt, va) PI_LOG_FMTVA(PI_LOG_DEBUG, fmt, va)
    #define PI_DEBUG_FMT(...) PI_LOG_FMT(PI_LOG_DEBUG, __VA_ARGS__)
    #define PI_DEBUG_STR(str) PI_LOG_STR(PI_LOG_DEBUG, str)
#endif


PI_EXTERN_C size_t PI_LOG_BYTES(size_t datasize, const void* p_data);
PI_EXTERN_C size_t PI_LOG_NEW(PI_LOGCATEGORY category);

PI_EXTERN_C bool PI_LOG_SAMELINE(void);
PI_EXTERN_C bool PI_LOG_FLUSH(void);

/* Error API */

    /* Generic errors */

#define PI_NOERROR     0
#define PI_INVALIDARG -1
#define PI_FAIL       -2

    /* Users can make their own error types using this enum, to avoid collision (as long as the user doesn't choose the same `id`s for different error types) */
#define PI_MAKE_ERROR(id) ( 0x80000000 | (id) )

    /* For API headers to use */
#define PI_MAKE_API_ERROR(id) ( 0xFFFFFF80 | (id) )

    /* Error type definition */
typedef int PI_ERRORTYPE;

PI_EXTERN_C long PI_PUSH_ERROR(PI_HGAME hgame, PI_ERRORTYPE type, const char* msg);
PI_EXTERN_C PI_ERRORTYPE PI_POP_ERROR(const char** pp_msg, PI_HGAME* p_hgame);
PI_EXTERN_C PI_ERRORTYPE PI_GET_LASTERROR(const char** pp_msg, PI_HGAME* p_hgame);

/**/

#endif
