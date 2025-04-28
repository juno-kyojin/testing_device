/**
 * @file action_registry.h
 * @brief Header file for action dispatch registry
 *
 * This header file defines the interface for the action dispatch registry, which maps
 * service names to their respective handler functions. It provides functions to register
 * services and initialize the dispatch system.
 *
 * @author [junokyojin]
 * @date 2025-04-23
 */
#ifndef ACTION_REGISTRY_H
#define ACTION_REGISTRY_H

/**
 * @brief Initialize the action dispatch system
 *
 * This function initializes the action dispatch system by setting up the dispatch table.
 * It should be called before any services are registered or actions are executed.
 *
 * @return None
 */
void init_action_dispatch();

/**
 * @brief Register a service with its handler
 *
 * This function registers a service with its corresponding handler function in the
 * dispatch table.
 *
 * @param service Name of the service (e.g., "ping").
 * @param handler Pointer to the handler function for the service.
 * @return None
 */
void register_service(const char *service, void (*handler)());

/* Other declarations as needed */
#endif /* ACTION_REGISTRY_H */