#include <stdio.h>
#include <stdlib.h>

/* == WATCHDOG CONFIG == */

#define WATCHDOG_MAX_TICKS  5

/* == INTERNAL STATE == */

static int watchdog_ticks  = 0;
static int watchdog_active = 0;

/* Start (enable) the watchdog */
void sim_watchdog_start(void)
{
    watchdog_ticks  = 0;
    watchdog_active = 1;
}
void sim_watchdog_kick(void)
{
    if (watchdog_active) {
        watchdog_ticks = 0;
    }
}
void sim_watchdog_tick(void)
{
    if (!watchdog_active) {
        return;
    }

    watchdog_ticks++;
    if (watchdog_ticks >= WATCHDOG_MAX_TICKS) {
        printf("🐶 WATCHDOG RESET (firmware unresponsive)\n");
        fflush(stdout);   
        exit(0);          
    }
}
void sim_watchdog_stop(void)
{
    watchdog_active = 0;
}
