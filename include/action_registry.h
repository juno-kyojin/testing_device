/**
 * @file action_registry.h
 * @brief Header file for the action registry system
 *
 * This header file defines the interface for the action registry mechanism,
 * which is responsible for initializing the action dispatch system by registering
 * all available service handlers (e.g., "ping", "speedtest"). It ensures that the
 * system is ready to execute test cases by mapping service names to their respective
 * handler functions.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 * @see action_registry.c
 * @see action.h
 */
#ifndef ACTION_REGISTRY_H
#define ACTION_REGISTRY_H

/**
 * @brief Initialize the action dispatch system
 *
 * This function registers all available service handlers with the action dispatch
 * mechanism. Each service (e.g., "ping", "speedtest") is associated with its
 * corresponding handler function (e.g., `execute_ping`, `execute_speedtest`).
 * This function must be called before any test case actions can be executed.
 *
 * @return None
 */
void init_action_dispatch(void);

#endif