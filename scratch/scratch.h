#ifndef SCRATCH_H
#define SCRATCH_H

#include <stddef.h>

#define SCRATCH_SIZE (32u * 1024u)
#define SCRATCH_MONGOOSE_THRESHOLD (8u * 1024u)

void scratchInit(void);
void *scratchAcquire(size_t size, const char *owner);
void scratchRelease(void *ptr, const char *owner);
unsigned char scratchIsBusy(void);
unsigned char scratchIsPtr(const void *ptr);
size_t scratchSize(void);
size_t scratchUsedSize(void);
const char *scratchOwner(void);

#endif
