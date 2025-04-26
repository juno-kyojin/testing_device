/**
 * @file action_registry.c
 * @brief Implementation of the action registry system
 *
 * This file contains the implementation of the action registry mechanism.
 * It registers all supported test services with the action dispatch table,
 * ensuring that the system can route test case requests to the appropriate
 * handler functions. Services like "ping" and "speedtest" are registered during
 * initialization.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see action_registry.h
 * @see action.h
 * @see ping.h
 * @see speedtest.h
 */
#include "action.h"
#include "ping.h"
#include "speedtest.h"

void init_action_dispatch(void) {
    register_service("ping", execute_ping);
    // register_service("speedtest", execute_speedtest);
}