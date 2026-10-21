#ifndef PI_IO_H
#define PI_IO_H

#include <piengine/pi_common.h>

    /* System IO handle. It doesn't wrap absolutely anything, for example, let's say:
     *  On Windows: This would give you a valid pointer to a HANDLE.
     *  On Unix-like systems: This would give you a valid file descriptor. */
typedef struct PI_SYSIO *PI_HSYSIO;

#ifdef PI_CPP
    #define PI_IOCAST(hio, t) reinterpret_cast<t>(const_cast<PI_SYSIO*>(hio))
#else
    #define PI_IOCAST(hio, t) ((t)(hio))
#endif

#define PI_IO_STDIN  0
#define PI_IO_STDOUT 1
#define PI_IO_STDERR 2

#define PI_IOACCESS_READ  0x01
#define PI_IOACCESS_WRITE 0x02
#define PI_IOACCESS_READWRITE ( PI_IOACCESS_READ | PI_IOACCESS_WRITE ) 

#define PI_IOMODE_CREATE 1
#define PI_IOMODE_APPEND 2
#define PI_IOMODE_OPEN 3

typedef int PI_IO_CONSOLE;

typedef int PI_IOMODE;
typedef int PI_IOACCESS;

#ifndef __unix__
    PI_EXTERN_C PI_HSYSIO pi_io_getconsole_handle(PI_IO_CONSOLE console);
#else
    #define pi_io_getconsole_handle(console) ( ((unsigned)(console)) > PI_IO_STDERR ? PI_INVALID_HIO : (PI_HSYSIO)(console) )
#endif

#if defined(__unix__) || defined(_WIN32)
    #define PI_INVALID_HIO ((PI_HSYSIO)-1)
#else
    extern PI_HSYSIO const PI_INVALID_HIO;
#endif

PI_EXTERN_C PI_SSIZE pi_io_read(PI_HSYSIO hio, size_t bufsize, void* buf);
PI_EXTERN_C PI_SSIZE pi_io_write(PI_HSYSIO hio, size_t bufsize, const void* buf);
PI_EXTERN_C PI_HSYSIO pi_io_open(const char* path, PI_IOMODE mode, PI_IOACCESS accessflags);
PI_EXTERN_C bool      pi_io_close(PI_HSYSIO hio);


PI_EXTERN_C bool pi_io_esccols_supported(void);

#endif
