#include <piengine/pi_main.h>
#include <piengine/pi_debug.h>

int main()
{
    PI_LOG_STR(0, "LOG STR from main!");
    PI_LOG_FMT(0, "LOG FMT from main! Example string: \"%s\"", "Hello World!");

}
