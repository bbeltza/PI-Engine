#include <Windows.h>

#include <piengine/pi_io.h>
#include <piengine/pi_debug.h>

#define CAST_HSYSIO(hfile) ( (PI_HSYSIO)(hfile) )

PI_EXTERN_C PI_HSYSIO pi_io_getconsole_handle(PI_IO_CONSOLE console)
{
    DWORD stdhandle ;
    switch (console) {
        case PI_IO_STDIN:
            stdhandle = STD_INPUT_HANDLE;
            break;
        case PI_IO_STDOUT:
            stdhandle = STD_OUTPUT_HANDLE;
            break;
        case PI_IO_STDERR:
            stdhandle = STD_ERROR_HANDLE;
            break;
        default:
            return PI_INVALID_HIO;
    }

    return CAST_HSYSIO( GetStdHandle(stdhandle) );
}

PI_SSIZE pi_io_read(PI_HSYSIO hio, size_t bufsize, void* buf)
{
    DWORD bytesread;
    HANDLE hfile = PI_IOCAST(hio, HANDLE);
    if (!ReadFile(hfile, buf, (DWORD)bufsize, &bytesread, NULL)) {
        return -1;
    }

    return bytesread;
}

PI_SSIZE pi_io_write(PI_HSYSIO hio, size_t bufsize, const void* buf)
{
    DWORD byteswritten;
    HANDLE hfile = PI_IOCAST(hio, HANDLE);
    if (!WriteFile(hfile, buf, (DWORD)bufsize, &byteswritten, NULL)) {
        return -1;
    }

    return byteswritten;
}

PI_HSYSIO pi_io_open(const char* path, PI_IOMODE mode, PI_IOACCESS accessflags)
{
    PI_ERROR(NULL, PI_UNSUPPORTED, "Opening IO files is unimplemented");
    return PI_INVALID_HIO;
}

bool pi_io_close(PI_HSYSIO hio) {
    return CloseHandle(PI_IOCAST(hio, HANDLE));
}


PI_EXTERN_C bool pi_io_esccols_supported(void)
{
    #ifdef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        static HANDLE hstdout;
        if (!hstdout)
            hstdout = GetStdHandle(STD_OUTPUT_HANDLE);

        DWORD mode;
        BOOL res = GetConsoleMode(hstdout, &mode);
        PI_ASSERT(res == TRUE);
        if (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)
            return true;
    #endif
        return false;
}
