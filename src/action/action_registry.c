#include "action/action.h"
#include "test/ping.h"

void init_action_dispatch(void) {
    // register_action
    register_action("ping", execute_ping);
    //....
    // register_action("wan", execute_wan);
}