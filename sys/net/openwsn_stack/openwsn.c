/**
\brief General OpenWSN definitions

\author Thomas Watteyne <watteyne@eecs.berkeley.edu>, September 2012
*/

#include "openwsn.h"
#include "scheduler.h"
#include "thread.h"
#include "board_ow.h"
#include "leds.h"
//===== drivers
#include "openserial.h"
//===== stack
//-- cross-layer
#include "idmanager.h"
#include "openqueue.h"
#include "openrandom.h"
#include "opentimers.h"
//-- 02a-TSCH
#include "adaptive_sync.h"
#include "IEEE802154E.h"
//-- 02b-RES
#include "schedule.h"
#include "sixtop.h"
#include "neighbors.h"
//-- 03a-IPHC
#include "openbridge.h"
#include "iphc.h"
//-- 03b-IPv6
#include "forwarding.h"
#include "icmpv6.h"
#include "icmpv6echo.h"
#include "icmpv6rpl.h"
//-- 04-TRAN
#include "opentcp.h"
#include "openudp.h"
#include "opencoap.h"
//===== applications
//+++++ TCP
//- debug
#include "tcpecho.h"
#include "tcpinject.h"
#include "tcpprint.h"
//- common
#include "ohlone.h"
//- board-specific
//++++ UDP
//- debug
#include "udpecho.h"
#include "udpinject.h"
#include "udpprint.h"
//- common
//#include "udprand.h"
//#include "udplatency.h"
#include "udpstorm.h"
//- board-specific
//#include "imu.h"
//+++++ CoAP
//- debug
//- common
#include "rinfo.h"
#include "rleds.h"
#include "rwellknown.h"
#include "r6t.h"
//#include "rrt.h"
//#include "rex.h"
//#include "rrube.h"
//#include "layerdebug.h"
//- board-specific
//#include "rheli.h"
//#include "rt.h"
//#include "rxl1.h"
//#include "heli.h"
//#include "imu.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

//=========================== variables =======================================

static char openwsn_stack[KERNEL_CONF_STACKSIZE_MAIN];

//=========================== prototypes ======================================

void openwsn_init(void);
void* openwsn_start(void *arg);

//=========================== public ==========================================

void openwsn_start_thread(void) {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    thread_create(openwsn_stack, KERNEL_CONF_STACKSIZE_MAIN,
                   PRIORITY_OPENWSN+1, CREATE_STACKTEST,
                   openwsn_start, NULL, "openwsn thread");
}

void* openwsn_start(void *arg) {
    DEBUG("%s\n",__PRETTY_FUNCTION__);
    (void)arg;
    leds_all_off();
    board_init_ow();
    scheduler_init();
    openwsn_init();
    puts("OpenWSN thread started.");
    scheduler_start();
    return NULL;
}

//=========================== private =========================================

void openwsn_init(void) {
    DEBUG("%s\n",__PRETTY_FUNCTION__);

   //===== drivers
   //openserial_init();

   //===== stack
   //-- cross-layer
   idmanager_init();    // call first since initializes EUI64 and isDAGroot
   openqueue_init();
   openrandom_init();
   opentimers_init();
   //-- 02a-TSCH
   adaptive_sync_init();
   ieee154e_init();
   //-- 02b-RES
   schedule_init();
   sixtop_init();
   neighbors_init();
   //-- 03a-IPHC
   openbridge_init();
   iphc_init();
   //-- 03b-IPv6
   forwarding_init();
   icmpv6_init();
   icmpv6echo_init();
   icmpv6rpl_init();
   //-- 04-TRAN
   opentcp_init();
   openudp_init();
   opencoap_init();    // initialize before any of the CoAP applications
   //===== applications
   //+++++ TCP
   //- debug
   tcpecho_init();
   tcpinject_init();
   tcpprint_init();
   //- common
   ohlone_init();
   //- board-specific
   //+++++ UDP
   //- debug
   udpecho_init();
   udpinject_init();
   udpprint_init();
   //- common
   //udprand_init();
   //udplatency_init();
   // udpstorm_init();
   //- board-specific
   //imu_init();
   //+++++ CoAP
   //- debug
   //- core
   //- common
   // rinfo_init();
   //rrt_init();
   // rleds__init();
   // rwellknown_init();
   r6t_init();
   //rreg_init();
   //rex_init();
   //rrube_init();
   //layerdebug_init();
   //- board-specific
   //rheli_init();
   //rt_init();
   //rxl1_init();

   openserial_printInfo(COMPONENT_OPENWSN,ERR_BOOTED,
                            (errorparameter_t)0,
                            (errorparameter_t)0);
    // leds_all_off();
}
