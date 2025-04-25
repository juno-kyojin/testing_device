#ifndef TYPES_H
#define TYPES_H

#include "cjson/cJSON.h"

typedef struct {
    char service[50];  // Service (ping, speedtest, etc.)
    char action[50];   // Action (create, delete, etc.)
} TestCase;

// Function pointer type for action handling
typedef void (*ActionHandler)(TestCase *, const char *, int, cJSON *);

// Structure for the action dispatch table
typedef struct {
    char service[50];
    ActionHandler handler;
} ActionDispatch;

#endif