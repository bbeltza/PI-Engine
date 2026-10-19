#include <piengine/pi_debug.h>

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

size_t PI_LOG_FMTVA(PI_LOGCATEGORY category, const char* fmt, va_list va)
{
    size_t ret;
    int count = SDL_vsnprintf(NULL, 0, fmt, va);
    if (count <= 0) {
        return 0;
    }
    
    count++;
    char* buf = SDL_stack_alloc(char, count);
    SDL_vsnprintf(buf, count, fmt, va);
        PI_ASSERT(buf[count-1] == '\0');
        ret = PI_LOG_STR(category, buf);
    SDL_stack_free(buf);
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
static FILE* s_logstream = NULL;
static int s_delaytime = 10;

static int s_loginit = 0;

#define PREFIX_STR(x) { x, sizeof(x)-1 }
static const char INFO_STR[] = "[ INFO ]: ";
static const char WARN_STR[] = "[ WARN ]: ";
static const char DEBUG_STR[] = "[ DEBUG ]: ";
static const struct { const char* data; size_t size; } PREFIX_STRS[] = {
    PREFIX_STR(INFO_STR),
    PREFIX_STR(WARN_STR),
    PREFIX_STR(DEBUG_STR),
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
    SDL_DestroyMutex(s_logmutex);
    s_logmutex = NULL;
}

static void log_init()
{
    if (s_loginit) {
        return;
    }
    static PI_SPINLOCK spinlock;
    int pres;
    SDL_LockSpinlock(&spinlock);
        if (s_loginit) {
            goto INIT_END;
        }

        pres = setvbuf(stdout, NULL, _IONBF, 0);
        PI_ASSERT(pres == 0);

        s_logmutex = SDL_CreateMutex();
        s_logstream = stdout;
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

    if (fwrite(s_logbuf, 1, s_logpos, s_logstream) < s_logpos)
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
            log_write(
                    PREFIX_STRS[s_queued_category].size,
                    PREFIX_STRS[s_queued_category].data
                );
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


