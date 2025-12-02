#ifndef ENV_H
#define ENV_H

#ifdef __cplusplus
extern "C" {
#endif
#include "object.h"
#include "ptypes.h"
#include <stddef.h>
#include <stdint.h>

#define NAME   EnvTable
#define KEY_TY u64
#define VAL_TY PValue
#define HEADER_MODE
#include "../external/verstable/verstable.h"

typedef struct Pgc Pgc;

// Environment Structure
typedef struct PEnv {
    // Variable Table
    EnvTable table;
    // Count of pairs in table
    u64 count;
    // Parent Environment
    struct PEnv *enclosing;
} PEnv;

// Typecast `void *` to `PEnv *`
#define GetEnv(ptr) ((PEnv *)ptr)

// Create New Environment;
// `enclosing` = Optional Parent Environment. Can be NULL
PEnv *NewEnv(Pgc *gc, PEnv *enclosing);

// Free Environment with freeing all elements
void ReallyFreeEnv(PEnv *e);

// Recycle Environment adding to GC's free list for reuse
void RecycleEnv(Pgc *gc, PEnv *e);

// Print all pairs inside the Environment;
// Does not print parent(s)
void DebugEnv(PEnv *e);

// Mark Environment Roots for GC
void MarkEnvGC(Pgc *gc, PEnv *e);

// Capture Upvalues Naively from parentEnv to clsEnv
void EnvCaptureUpvalues(Pgc *gc, PEnv *parentEnv, PEnv *clsEnv);

// Add Key/Value pair to the `e`'s table
void EnvTableAddValue(PEnv *e, u64 hash, PValue value);
// Check if `e`'s table has hash
bool EnvHasKey(PEnv *e, u64 hash);
// Get how many pairs env `e` has
u64 EnvGetCount(const PEnv *e);

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
