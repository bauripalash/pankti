#ifndef ENV_H
#define ENV_H


#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <stdint.h>
#include "object.h"
#include "ptypes.h"

// Environment Structure
typedef struct PEnv {
    // Variable Table
    struct {
        u64 key;
        PValue value;
    } *table;
    // Count of pairs in table
    u64 count;
    // Parent Environment
    struct PEnv *enclosing;
} PEnv;

// Typecast `void *` to `PEnv *`
#define GetEnv(ptr) ((PEnv *)ptr)

// Create New Environment;
// `enclosing` = Optional Parent Environment. Can be NULL
PEnv *NewEnv(PEnv *enclosing);

// Free Environment with freeing all elements
void FreeEnv(PEnv *e);

// Print all pairs inside the Environment;
// Does not print parent(s)
void DebugEnv(PEnv *e);

// Create a Pair and push to the Environment table
void EnvPutValue(PEnv *e, u64 hash, PValue value);

// Fetch value from table corresponding the provided hash
// Returns `NULL` if not found.
PValue EnvGetValue(PEnv *e, u64 hash, bool *found);

// Set value to existing variable in table;
// Returns true if exists and was successful in updating value;
// Returns false if key doesn't exist, or failed to update.
bool EnvSetValue(PEnv *e, u64 hash, PValue value);

#ifdef __cplusplus
}
#endif

#endif
