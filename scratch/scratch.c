#include "scratch.h"

#include <string.h>
#include "pico/sync.h"

static unsigned char scratch_buf[SCRATCH_SIZE];
static critical_section_t scratch_lock;
static unsigned char scratch_inited = 0;
static unsigned char scratch_busy = 0;
static size_t scratch_used = 0;
static const char *scratch_owner = NULL;

void scratchInit(void)
{
	if(scratch_inited) return;
	critical_section_init(&scratch_lock);
	scratch_inited = 1;
}

void *scratchAcquire(size_t size, const char *owner)
{
	if(!scratch_inited) scratchInit();
	if(size == 0 || size > SCRATCH_SIZE) return NULL;

	void *ptr = NULL;
	critical_section_enter_blocking(&scratch_lock);
	if(!scratch_busy)
	{
		scratch_busy = 1;
		scratch_used = size;
		scratch_owner = owner;
		ptr = scratch_buf;
	}
	critical_section_exit(&scratch_lock);

	if(ptr != NULL) memset(ptr, 0, size);
	return ptr;
}

void scratchRelease(void *ptr, const char *owner)
{
	(void)owner;
	if(ptr == NULL || !scratchIsPtr(ptr)) return;
	if(!scratch_inited) return;

	critical_section_enter_blocking(&scratch_lock);
	scratch_busy = 0;
	scratch_used = 0;
	scratch_owner = NULL;
	critical_section_exit(&scratch_lock);
}

unsigned char scratchIsBusy(void)
{
	if(!scratch_inited) return 0;
	return scratch_busy;
}

unsigned char scratchIsPtr(const void *ptr)
{
	return ptr == (const void *)scratch_buf;
}

size_t scratchSize(void)
{
	return SCRATCH_SIZE;
}

size_t scratchUsedSize(void)
{
	return scratch_used;
}

const char *scratchOwner(void)
{
	return scratch_owner;
}
