#ifndef ENV_H
#define ENV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "object.h"
#include <stdint.h>

// Key+Value Pair for storing variables in Environment
// `key` = Hash of the variable
// `value` = Pankti Object
typedef struct EnvPair {
    // Hash of the variable name
    uint32_t key;
    // Object value
    PObj *value;
} EnvPair;

// Environment Structure
typedef struct PEnv {
    // Variable Table
    // We are using Array for `now`. Note: Switch to Hash table
    EnvPair **table;
    // Count of pairs in table
    int count;
    // Parent Environment
    struct PEnv *enclosing;
} PEnv;

// Typecast `void *` to `PEnv *`
#define GetEnv(ptr) ((PEnv *)ptr)

// Create New Environment;
// `enclosing` = Optional Parent Environment. Can be NULL
PEnv *NewEnv(PEnv *enclosing);

// Create Key/Value Pair (malloc'd)
// Should not use outside of Environment related functions
EnvPair *NewPair(uint32_t key, PObj *value);

// Free Environment with freeing all elements
void FreeEnv(PEnv *e);

// Print all pairs inside the Environment;
// Does not print parent(s)
void DebugEnv(PEnv *e);

// Create a Pair and push to the Environment table
void EnvPutValue(PEnv *e, uint32_t hash, PObj *value);

// Fetch value from table corresponding the provided hash
// Returns `NULL` if not found.
PObj *EnvGetValue(PEnv *e, uint32_t hash);

// Set value to existing variable in table;
// Returns true if exists and was successful in updating value;
// Returns false if key doesn't exist, or failed to update.
bool EnvSetValue(PEnv *e, uint32_t hash, PObj *value);

#ifdef __cplusplus
}
#endif

#endif
