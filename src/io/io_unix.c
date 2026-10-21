#include <unistd.h>
#include <fcntl.h>

#include <piengine/pi_io.h>
#include <piengine/pi_debug.h>

#define CAST_HSYSIO(fd) ( (PI_HSYSIO)(fd) )
#define CAST_FD(hio) ( (int)PI_IOCAST(hio, intptr_t) )

PI_SSIZE pi_io_read(PI_HSYSIO hio, size_t bufsize, void* buf)
{
    int fd = CAST_FD(hio);
    return read(fd, buf, bufsize);
}

PI_SSIZE pi_io_write(PI_HSYSIO hio, size_t bufsize, const void* buf)
{
    int fd = CAST_FD(hio);
    return write(fd, buf, bufsize);
}

PI_HSYSIO pi_io_open(const char* path, PI_IOMODE mode, PI_IOACCESS accessflags)
{
    PI_ERROR(NULL, PI_UNSUPPORTED, "Opening IO files is unimplemented");
    return PI_INVALID_HIO;
}

bool pi_io_close(PI_HSYSIO hio) {
    int fd = CAST_FD(hio);
    return close(fd) == 0;
}


PI_EXTERN_C bool pi_io_esccols_supported(void) {
    return true;
}
