#ifndef NATIVE_H
#define NATIVE_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct PVm PVm;
void RegisterNatives(PVm *vm);

#ifdef __cplusplus
}
#endif

#endif
