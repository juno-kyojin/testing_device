#include "action.h"
#include "ping.h"
#include "speedtest.h"

void init_action_dispatch(void) {
    register_service("ping", execute_ping);
    // register_service("speedtest", execute_speedtest);
}