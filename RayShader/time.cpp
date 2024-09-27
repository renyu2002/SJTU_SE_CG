
/*******************************\
| A small unitlity to get both  |
| windows and linux times.      |
\*******************************/


#include <stdlib.h> // for NULL

#ifdef _WIN32
#   include <windows.h>
    static DWORD last_idle_time;
    static DWORD time_now;
#else
#   include <sys/time.h>
    static struct timeval last_idle_time;
    static struct timeval time_now;
#endif

// Figure out time elapsed since last call
float time_dt() {
  last_idle_time = time_now;

#ifdef _WIN32
    time_now = GetTickCount();
    return (float) (time_now - last_idle_time) / 1000.0;
#else
    gettimeofday(&time_now, NULL);
    return (
        (float)(time_now.tv_sec  - last_idle_time.tv_sec ) +
        1.0e-6*(time_now.tv_usec - last_idle_time.tv_usec)
    );
#endif

}

void time_init() {
#ifdef _WIN32
    last_idle_time = GetTickCount();
#else
    gettimeofday (&last_idle_time, NULL);
#endif
    time_dt();
}
