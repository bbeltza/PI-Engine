#include "../pi_internal.h"

#if !defined(_WIN32) 

#include <stdlib.h>

void* PI_ALLOC(size_t n, uint32_t flags)
{
    return (flags & PI_ALLOC_ZERO) ? calloc(1, n)
                                   : malloc(n);
}

void PI_FREE(void* ptr)
{
    if (!ptr) {
        return;
    }

    free(ptr);
}

void* PI_REALLOC(void* ptr, size_t n, uint32_t flags)
{
    if (!ptr && (flags & PI_ALLOC_ZERO))
        return calloc(1, n);

    return realloc(ptr, n);
}

#else

#include <Windows.h>

HANDLE PI_WIN32_HEAP;

static inline DWORD get_dwflags(uint32_t flags) {
    DWORD dwflags = 0;
    if (flags & PI_ALLOC_ZERO)
        dwflags |= HEAP_ZERO_MEMORY;
    return dwflags;
}

void* PI_ALLOC(size_t n, uint32_t flags)
{
    if (!PI_WIN32_HEAP) {
        // TODO: This is thread-unsafe, so make it thread safe in the future
            // (Ensure there is not a race condition on initializing the heap so two or more threads at the same initialize the "PI_WIN32_HEAP" global...)
            // "HEAP_NO_SERIALIZE" is being used for now.
        PI_WIN32_HEAP = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
        PI_ASSERT(PI_WIN32_HEAP != NULL);
        if (PI_WIN32_HEAP == NULL) {
            return NULL;
        }
    }

    DWORD dwflags = get_dwflags(flags);
    return HeapAlloc(PI_WIN32_HEAP, dwflags, n);
}

void PI_FREE(void* ptr)
{
    if (!ptr) {
        return;
    }

    HeapFree(PI_WIN32_HEAP, 0, ptr);
}

void* PI_REALLOC(void* ptr, size_t n, uint32_t flags)
{
    if (!ptr) {
        return PI_ALLOC(n, flags);
    }

    DWORD dwflags = get_dwflags(flags);
    return HeapReAlloc(PI_WIN32_HEAP, dwflags, ptr, n);
}


#endif
