#ifndef PI_MAIN_H
#define PI_MAIN_H

#include <piengine/pi_common.h>

#define PI_DEFAULT_STRUCT_ALIGNMENT (sizeof(void*) << 1)

// Forward struct declarations that are defined in <pi_core.h>

struct PI_EVENT;

//

typedef bool (*PI_PFN_APPALLOCDATA)(size_t* size); // The allocated struct will be aligned to 8 bytes on 32-bit platforms, or 16 bytes on 64-bit platforms (see PI_DEFAULT_STRUCT_ALIGNMENT)
typedef bool (*PI_PFN_APPSTART)(PI_HGAME hgame, va_list args);
typedef void (*PI_PFN_APPCLOSE)(PI_HGAME hgame);

typedef bool (*PI_PFN_APPEVENT)(PI_HGAME hgame, struct PI_EVENT* event);
typedef bool (*PI_PFN_APPUPDATE)(PI_HGAME hgame);

typedef struct PI_APPCALLBACKS
{
    PI_PFN_APPALLOCDATA appallocdata;
    PI_PFN_APPSTART appstart;
    PI_PFN_APPCLOSE appclose;

    PI_PFN_APPEVENT appevent;
    PI_PFN_APPUPDATE appupdate;
} PI_APPCALLBACKS;

typedef struct PI_APPINFO
{
    const char* name;
    const PI_APPCALLBACKS* callbacks;
} PI_APPINFO;

PI_EXTERN_C int pi_run_exv(const PI_APPINFO* appinfo, va_list args);

static inline int pi_run_ex(const PI_APPINFO* appinfo, ...) {
    int res;
    va_list va;
    va_start(va, appinfo);
        res = pi_run_exv(appinfo, va);
    va_end(va);
    return res;
}
PI_INLINE_C int pi_run(const PI_APPINFO* appinfo) {
    return pi_run_ex(appinfo, 0);
}

#endif
