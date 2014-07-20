#include <stdio.h>

#include "vtimer.h"
#include "openwsn.h"

int main(void) {
//   board_init();
//   scheduler_init();
    openwsn_start_thread();
    unsigned i = 0;
    while (1) {
        printf("RIOT -> %u\n", i++);
        thread_print_all();
        vtimer_usleep(2000 * 1000);
    }
   // openwsn_init();
   // puts("and now?!");
   // scheduler_start();
   while (1); // this line should never be reached
}
