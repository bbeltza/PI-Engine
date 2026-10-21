#include "app.h"

#include <piengine/pi_main.h>

#define PI_APPALLOCDATA_SIMPLE(fn) static bool fn(size_t* psize) { \
                                        *psize = sizeof(struct PI_GAME);  \
                                        return true;               \
                                   }

PI_APPALLOCDATA_SIMPLE(appallocdata)

int main(int argc, char* argv[])
{
    PI_APPINFO appinfo = {
        "2D OBBs test",
        &(PI_APPCALLBACKS){
            appallocdata,
            app_start,
            app_close,
            app_event,
            app_update
        }
    };
    return pi_run_ex(&appinfo, argc, argv);
}
