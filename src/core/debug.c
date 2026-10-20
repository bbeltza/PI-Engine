#include <piengine/pi_debug.h>
#include <piengine/pi_io.h>

#include <SDL3/SDL_stdinc.h>

    /* Logging API */

size_t PI_LOG_STR(PI_LOGCATEGORY category, const char* str)
{
    PI_LOG_NEW(category);

    size_t count = 0;
    const char* writeptr = str;
    const char* ptr = str;
    while (*ptr) {
        char chr = *ptr;
        if (chr == '\n') {
            PI_LOG_BYTES(ptr - writeptr, writeptr);
            PI_LOG_NEW(category);
            writeptr = ++ptr;
            continue;
        }

        ptr++;
        count++;
    }

    PI_LOG_BYTES(ptr - writeptr, writeptr);
    return count;
}

#include <SDL3/SDL_thread.h>

size_t PI_LOG_FMTVA(PI_LOGCATEGORY category, const char* fmt, va_list va)
{
    struct TLSData {
        size_t size;
        char buf[];
    };

    static SDL_TLSID buf_tls;
    struct TLSData* data = SDL_GetTLS(&buf_tls);
    if (!data) {
        const size_t START_SIZE = 512;
        bool res = SDL_SetTLS(&buf_tls, PI_ALLOC(sizeof(struct TLSData) + START_SIZE, 0), PI_FREE);
        PI_ASSERT(res == true);

        data = SDL_GetTLS(&buf_tls);
        PI_ASSERT(data != NULL);

        data->size = START_SIZE;
    }

    size_t ret;
    va_list vac;
    va_copy(vac, va);
    int count = SDL_vsnprintf(data->buf, data->size, fmt, vac);
    if (count <= 0) {
        return 0;
    }

    if (count >= data->size) {
        size_t newsize = data->size + count;
        PI_FREE(data);

        bool res = SDL_SetTLS(&buf_tls, PI_ALLOC(sizeof(struct TLSData) + newsize, 0), PI_FREE);
        PI_ASSERT(res == true);

        data = SDL_GetTLS(&buf_tls);
        PI_ASSERT(data != NULL);

        data->size = newsize;
        count = SDL_vsnprintf(data->buf, newsize, fmt, va);
        PI_ASSERT(count < newsize);
    }
    
    PI_ASSERT(data->buf[count] == '\0');
    ret = PI_LOG_STR(category, data->buf);
    return ret;
}

    /* Lower level logging API (Shouldn't be used directly) */

#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_timer.h>

#include <stdlib.h>

typedef SDL_Mutex* PI_HMUTEX;
typedef SDL_Thread* PI_HTHREAD;
typedef SDL_SpinLock PI_SPINLOCK;

static PI_HMUTEX s_logmutex;
static PI_HTHREAD s_logthread;
static PI_LOGCATEGORY s_queued_category = -1;
static size_t s_queued_category_charswritten = 0;

static char s_logbuf[BUFSIZ];
static size_t s_logpos = 0;

static PI_HSYSIO s_loghio = NULL;
static int s_esccols_supported;
static int s_delaytime = 10;

static int s_loginit = 0;

#define PREFIX_STR(x) { x, sizeof(x)-1 }
static const char INFO_STR[] = "[ INFO ]: ";
static const char WARN_STR[] = "[ WARN ]: ";
static const char ERROR_STR[] = "[ ERROR ]: ";
static const char DEBUG_STR[] = "[ DEBUG ]: ";
static const struct { const char* data; size_t size; } PREFIX_STRS[] = {
    PREFIX_STR(INFO_STR),
    PREFIX_STR(WARN_STR),
    PREFIX_STR(ERROR_STR),
    PREFIX_STR(DEBUG_STR),
};

static const char RETURN_ESCCOL[] = "\033[0m";

static const char INFO_ESCCOL[] = "\033[36;1m";
static const char WARN_ESCCOL[] = "\033[33;1m";
static const char ERROR_ESCCOL[] = "\033[32;1m";
static const char DEBUG_ESCCOL[] = "\033[39;1m";

static const struct { const char* data; size_t size; } PREFIX_ESCCOLS[] = {
    PREFIX_STR(INFO_ESCCOL),
    PREFIX_STR(WARN_ESCCOL),
    PREFIX_STR(ERROR_ESCCOL),
    PREFIX_STR(DEBUG_ESCCOL),
};

static bool log_flush_nolock();

static int log_threadentry(void* spinlock) {
    SDL_LockSpinlock((SDL_SpinLock*)spinlock);
    SDL_UnlockSpinlock((SDL_SpinLock*)spinlock);
    
    while (s_loginit) {
        PI_LOG_FLUSH();
        SDL_Delay(s_delaytime);
    }

    return 0;
}

static void log_cleanup(void) {
    SDL_LockMutex(s_logmutex);
        s_loginit = 0;
    SDL_UnlockMutex(s_logmutex);

    SDL_WaitThread(s_logthread, NULL);
    log_flush_nolock();
#ifdef __unix__
    char buf = '\n';
    pi_io_write(s_loghio, sizeof(buf), &buf);
#endif
    SDL_DestroyMutex(s_logmutex);
    s_logmutex = NULL;
}

static void log_init()
{
    if (s_loginit) {
        return;
    }
    static PI_SPINLOCK spinlock;
    SDL_LockSpinlock(&spinlock);
        if (s_loginit) {
            goto INIT_END;
        }

        s_logmutex = SDL_CreateMutex();
        s_loghio = pi_io_getconsole_handle(PI_IO_STDOUT);
        s_esccols_supported = pi_io_esccols_supported();
        
        s_logthread = SDL_CreateThread(log_threadentry, "PI Logging Thread", &spinlock);
        atexit(log_cleanup);

    INIT_END:
        s_loginit = 1;
    SDL_UnlockSpinlock(&spinlock);
}

static bool log_flush_nolock() {
    bool succeed = true;
    if (!s_logpos)
        return true;

    if (pi_io_write(s_loghio, s_logpos, s_logbuf) < (PI_SSIZE)s_logpos)
        succeed = false;
    s_logpos = 0;

    return succeed;
}

static size_t log_write(size_t count, const char* data)
{
    PI_ASSERT(s_logpos < BUFSIZ);
    size_t i = count;
    const char* ptr = data;
    while (i) {
        s_logbuf[s_logpos++] = *ptr++;
        i--;

        if (s_logpos >= BUFSIZ) {
            if (!log_flush_nolock()) {
                return count - i;
            }
        }
    }

    return count;
}

size_t PI_LOG_BYTES(size_t datasize, const void* p_data)
{
    if (!s_loginit) {
        return 0;
    }

    if (!datasize)
        return 0;
    
    SDL_LockMutex(s_logmutex);
        if (s_queued_category == -2) {
            s_queued_category = -1;
        }
        else if (s_queued_category != -1) {
            PI_ASSERT(s_queued_category < PI_ARRAYSIZE(PREFIX_STRS));

            if (s_loginit != 1) {
                char newline = '\n';
                log_write(sizeof(newline), &newline);
            }
            else {
                s_loginit = 2;
            }
            if (s_esccols_supported) {
                log_write(
                        PREFIX_ESCCOLS[s_queued_category].size,
                        PREFIX_ESCCOLS[s_queued_category].data
                    );
            }
                log_write(
                        PREFIX_STRS[s_queued_category].size,
                        PREFIX_STRS[s_queued_category].data
                    );
            if (s_esccols_supported) {
                log_write(
                        sizeof(RETURN_ESCCOL)-1,
                        RETURN_ESCCOL
                    );
            }

            s_queued_category_charswritten = 0;
            s_queued_category = -1;
        }
        size_t written = log_write(datasize, p_data);
        s_queued_category_charswritten += written;
    SDL_UnlockMutex(s_logmutex);
    return written;
}

size_t PI_LOG_NEW(PI_LOGCATEGORY category)
{
    switch (category) {
        case PI_LOG_INFO:
        case PI_LOG_WARN:
        case PI_LOG_ERROR:
        case PI_LOG_DEBUG:
            break;
        default:
            return 0;
    }

    log_init();
    if (s_queued_category == -2)
        return s_queued_category_charswritten;

    return SDL_SetAtomicInt((SDL_AtomicInt*)&s_queued_category, category) == -1 ?
                            s_queued_category_charswritten : 0;
}

bool PI_LOG_SAMELINE(void)
{
    if (!s_loginit) {
        return false;
    }

    return SDL_SetAtomicInt((SDL_AtomicInt*)&s_queued_category, -2) <= -1;
}
bool PI_LOG_FLUSH(void)
{
    if (!s_loginit) {
        return false;
    }

    bool succeed;
    SDL_LockMutex(s_logmutex);
        succeed = log_flush_nolock();
    SDL_UnlockMutex(s_logmutex);
    return succeed;
}

    /* Error API */

typedef struct PI_ERRORSLOT {
    PI_HGAME hgame;
    PI_ERRORTYPE type;
    char buf[4096];
} PI_ERRORSLOT;

typedef struct PI_ERRORTLS
{
    size_t slotcapacity;
    PI_ERRORSLOT* slots;
    PI_ERRORSLOT sys_slot;
} PI_ERRORTLS;

void PI_ERRORTLS_DESTRUCTOR(void* data) {
    PI_ERRORTLS* tls = (PI_ERRORTLS*)data;
    PI_FREE(tls->slots);
    PI_FREE(data);
}

static SDL_TLSID s_errortls;

#define SLOT_HASH(hgame, cap) ( ((size_t)hgame / 8) % cap )

long PI_ERROR(PI_HGAME hgame, PI_ERRORTYPE type, const char* msg)
{
    PI_ERRORTLS* tls = SDL_GetTLS(&s_errortls);
    if (!tls) {
        SDL_SetTLS(&s_errortls, PI_ALLOC(sizeof(PI_ERRORTLS), 0), PI_ERRORTLS_DESTRUCTOR);

        tls = SDL_GetTLS(&s_errortls);
        PI_ASSERT(tls != NULL);

        tls->slotcapacity = 4;
        tls->slots = PI_ALLOC(sizeof(PI_ERRORSLOT), 0);
        for (size_t i = 0; i < tls->slotcapacity; i++) {
            tls->slots[i].hgame = NULL;
        }
        tls->sys_slot.hgame = NULL;
    }

    PI_ERRORSLOT* slot;
    if (!hgame) {
        slot = &tls->sys_slot;
        goto UPDATE_SLOT;
    }

    size_t slothash = SLOT_HASH(hgame, tls->slotcapacity);
    for (size_t s = slothash;;) {
        slot = &tls->slots[s];
        if (!slot->hgame) {
            slot->hgame = hgame;
            slot->buf[sizeof(slot->buf)-1] = '\0';
            break;
        }

        if (slot->hgame == hgame) {
            break;
        }

        s++;
        s %= tls->slotcapacity;
        if (s == slothash) {
            slot = NULL;
            break;
        }
    }

    if (!slot) {
        size_t newcap = tls->slotcapacity * 2;
        PI_ERRORSLOT* newslots = PI_ALLOC(sizeof(PI_ERRORSLOT) * newcap, 0);
        for (size_t i = 0; i < newcap; i++) {
            newslots[i].hgame = NULL;
        }

        for (size_t i = 0; i <= tls->slotcapacity; i++) {
            PI_HGAME thishgame = i == tls->slotcapacity ? hgame
                                        : tls->slots[i].hgame;
            if (!thishgame)
               continue; 

            size_t hash = SLOT_HASH(tls->slots[i].hgame, tls->slotcapacity);
            for (size_t s = hash;;) {
                slot = &tls->slots[s];
                if (!slot->hgame) {
                    if (i == tls->slotcapacity) {
                        slot->hgame = thishgame;
                        slot->buf[sizeof(slot->buf)-1] = '\0';
                    }
                    else {
                        *slot = tls->slots[i];
                    }
                    break;
                }

                s++;
                s %= newcap;
                PI_ASSERT(slot->hgame != thishgame);
                PI_ASSERT(s != hash);
            }
        }

        PI_FREE(tls->slots);
        tls->slotcapacity = newcap;
        tls->slots = newslots;
    }
UPDATE_SLOT:
    PI_ASSERT(slot && slot->hgame == hgame);

    slot->type = type;
    SDL_strlcpy(slot->buf, msg, sizeof(slot->buf));
    PI_ASSERT(slot->buf[sizeof(slot->buf)-1] == '\0');

    PI_LOG_FMT(PI_LOG_ERROR, "%s\n", slot->buf);
    return 0;
}


static PI_ERRORTYPE get_error_ex(PI_HGAME hgame, const char** pp_msg, bool clear)
{
    const char* dummy_pmsg;
    if (!pp_msg) {
        pp_msg = &dummy_pmsg;
    }

    PI_ERRORTLS* tls = SDL_GetTLS(&s_errortls);
    if (!tls)
        goto RET_NOERROR;

    PI_ERRORSLOT* slot;
    if (!hgame) {
        slot = &tls->sys_slot;
        if (slot->type == PI_NOERROR)
            goto RET_NOERROR;
        *pp_msg = slot->buf;
        if (clear)
            slot->type = PI_NOERROR;
        return slot->type;
    }

    size_t slothash = SLOT_HASH(hgame, tls->slotcapacity);
    for (size_t s = slothash;;) {
        slot = &tls->slots[s];
        if (slot->hgame == hgame) {
            PI_ERRORTYPE type = slot->type;
            if (type == PI_NOERROR)
                goto RET_NOERROR;

            *pp_msg = slot->buf;
            if (clear)
                slot->type = PI_NOERROR;
            return type;
        }

        if (!slot->hgame)
            goto RET_NOERROR;
    
        s++;
        s %= tls->slotcapacity;
    
    }

RET_NOERROR:
    *pp_msg = "No error.";
    return PI_NOERROR;
}

PI_ERRORTYPE PI_CLEAR_ERROR(PI_HGAME hgame, const char** pp_msg) {
    return get_error_ex(hgame, pp_msg, true);
}

PI_ERRORTYPE PI_GET_LASTERROR(PI_HGAME hgame, const char** pp_msg)
{
    return get_error_ex(hgame, pp_msg, false);
}
