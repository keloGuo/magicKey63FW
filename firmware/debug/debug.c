#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include "tusb.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <reent.h>
#include <string.h>
#include "pico/stdio/driver.h"
#include "pico/sync.h"
#include <stddef.h>
#include "../magic63_config.h"

#if MAGIC63_ENABLE_USB_CDC_DEBUG
#define CDC_DEBUG_TX_BUF_SIZE 2048u
#endif

#if MAGIC63_ENABLE_USB_CDC_DEBUG
static unsigned char cdc_debug_enabled = 1;
static critical_section_t cdc_debug_lock;
static char cdc_debug_tx_buf[CDC_DEBUG_TX_BUF_SIZE];
static unsigned int cdc_debug_tx_head = 0;
static unsigned int cdc_debug_tx_tail = 0;
static unsigned char cdc_debug_inited = 0;
#endif
static volatile unsigned int debug_core0_heartbeat = 0;
static volatile unsigned int debug_core1_heartbeat = 0;
static volatile unsigned int debug_core0_stage = 0;
static volatile unsigned int debug_core1_stage = 0;
static unsigned int debug_last_heap_free = 0;
static unsigned int debug_min_heap_free = 0xffffffffu;
static volatile unsigned int debug_alloc_current = 0;
static volatile unsigned int debug_alloc_peak = 0;
static volatile unsigned int debug_alloc_fail = 0;
static volatile unsigned int debug_mallinfo_arena = 0;
static volatile unsigned int debug_mallinfo_ordblks = 0;
static volatile unsigned int debug_mallinfo_uordblks = 0;
static volatile unsigned int debug_mallinfo_fordblks = 0;
static volatile unsigned int debug_mallinfo_keepcost = 0;
static char *debug_heap_start = NULL;
#if MAGIC63_ENABLE_DEBUG_OUTPUT
static uint32_t debug_last_probe_ms = 0;
#endif
#if MAGIC63_ENABLE_USB_CDC_DEBUG
static void cdc_debug_out_chars(const char *buf, int len);
static void cdc_debug_out_flush(void);
static int cdc_debug_in_chars(char *buf, int len);
#endif

extern char __StackLimit;
extern void *sbrk(ptrdiff_t incr);
void *__real__malloc_r(struct _reent *r, size_t size);
void *__real__calloc_r(struct _reent *r, size_t nmemb, size_t size);
void *__real__realloc_r(struct _reent *r, void *ptr, size_t size);
void __real__free_r(struct _reent *r, void *ptr);

#define DEBUG_ALLOC_MAGIC 0x4d41474bu

typedef struct {
	unsigned int magic;
	unsigned int size;
} debug_alloc_header_t;

void *__wrap__malloc_r(struct _reent *r, size_t size);
void *__wrap__calloc_r(struct _reent *r, size_t nmemb, size_t size);
void *__wrap__realloc_r(struct _reent *r, void *ptr, size_t size);
void __wrap__free_r(struct _reent *r, void *ptr);

static void debug_alloc_add(unsigned int size)
{
	debug_alloc_current += size;
	if(debug_alloc_current > debug_alloc_peak) debug_alloc_peak = debug_alloc_current;
}

static void debug_alloc_remove(unsigned int size)
{
	if(debug_alloc_current >= size) debug_alloc_current -= size;
	else debug_alloc_current = 0;
}

void *__wrap__malloc_r(struct _reent *r, size_t size)
{
	if(debug_heap_start == NULL)
	{
		char *heap_end = (char *)sbrk(0);
		if(heap_end != (char *)-1) debug_heap_start = heap_end;
	}
	size_t total = size + sizeof(debug_alloc_header_t);
	debug_alloc_header_t *header = (debug_alloc_header_t *)__real__malloc_r(r, total);
	if(header == NULL)
	{
		debug_alloc_fail++;
		return NULL;
	}
	header->magic = DEBUG_ALLOC_MAGIC;
	header->size = (unsigned int)size;
	debug_alloc_add((unsigned int)size);
	return (void *)(header + 1);
}

void *__wrap__calloc_r(struct _reent *r, size_t nmemb, size_t size)
{
	if(size != 0 && nmemb > ((size_t)-1) / size)
	{
		debug_alloc_fail++;
		return NULL;
	}
	size_t total = nmemb * size;
	void *ptr = __wrap__malloc_r(r, total);
	if(ptr != NULL) memset(ptr, 0, total);
	return ptr;
}

void *__wrap__realloc_r(struct _reent *r, void *ptr, size_t size)
{
	if(ptr == NULL) return __wrap__malloc_r(r, size);
	if(size == 0)
	{
		__wrap__free_r(r, ptr);
		return NULL;
	}

	debug_alloc_header_t *old_header = ((debug_alloc_header_t *)ptr) - 1;
	if(old_header->magic != DEBUG_ALLOC_MAGIC) return __real__realloc_r(r, ptr, size);

	void *new_ptr = __wrap__malloc_r(r, size);
	if(new_ptr == NULL) return NULL;
	size_t copy_size = old_header->size < size ? old_header->size : size;
	memcpy(new_ptr, ptr, copy_size);
	__wrap__free_r(r, ptr);
	return new_ptr;
}

void __wrap__free_r(struct _reent *r, void *ptr)
{
	if(ptr == NULL) return;
	debug_alloc_header_t *header = ((debug_alloc_header_t *)ptr) - 1;
	if(header->magic != DEBUG_ALLOC_MAGIC)
	{
		__real__free_r(r, ptr);
		return;
	}
	header->magic = 0;
	debug_alloc_remove(header->size);
	__real__free_r(r, header);
}

#if MAGIC63_ENABLE_USB_CDC_DEBUG
static stdio_driver_t cdc_debug_stdio = {
	.out_chars = cdc_debug_out_chars,
	.out_flush = cdc_debug_out_flush,
	.in_chars = cdc_debug_in_chars,
};

static unsigned char cdc_debug_ready(void)
{
	return cdc_debug_enabled && tud_ready() && tud_cdc_n_connected(0);
}

static unsigned int cdc_debug_next_pos(unsigned int pos)
{
	return (pos + 1u) % CDC_DEBUG_TX_BUF_SIZE;
}

static void cdc_debug_queue(const char *buf, unsigned int len)
{
	if(!cdc_debug_enabled || buf == NULL || len == 0) return;
	if(!cdc_debug_inited) return;

	critical_section_enter_blocking(&cdc_debug_lock);
	for(unsigned int i = 0;i < len;i++)
	{
		unsigned int next = cdc_debug_next_pos(cdc_debug_tx_head);
		if(next == cdc_debug_tx_tail) break;
		cdc_debug_tx_buf[cdc_debug_tx_head] = buf[i];
		cdc_debug_tx_head = next;
	}
	critical_section_exit(&cdc_debug_lock);
}

static void cdc_debug_queue_line(char *line, unsigned int size, int len)
{
	if(line == NULL || size < 3) return;
	if(len < 0) return;
	if((unsigned int)len >= size)
	{
		line[size - 3] = '\r';
		line[size - 2] = '\n';
		line[size - 1] = '\0';
		cdc_debug_queue(line, size - 1);
		return;
	}
	cdc_debug_queue(line, (unsigned int)len);
}

static unsigned int cdc_debug_dequeue(char *buf, unsigned int maxLen)
{
	unsigned int count = 0;
	if(!cdc_debug_inited || buf == NULL || maxLen == 0) return 0;

	critical_section_enter_blocking(&cdc_debug_lock);
	while(cdc_debug_tx_tail != cdc_debug_tx_head && count < maxLen)
	{
		buf[count++] = cdc_debug_tx_buf[cdc_debug_tx_tail];
		cdc_debug_tx_tail = cdc_debug_next_pos(cdc_debug_tx_tail);
	}
	critical_section_exit(&cdc_debug_lock);
	return count;
}
#else
static void cdc_debug_queue_line(char *line, unsigned int size, int len)
{
	(void)line;
	(void)size;
	(void)len;
}
#endif

unsigned int debugHeapFreeBytes(void)
{
	char *heap_end = (char *)sbrk(0);
	char *stack_limit = &__StackLimit;
	if(heap_end == (char *)-1 || heap_end >= stack_limit) return 0;
	if(debug_heap_start == NULL) debug_heap_start = heap_end;
	return (unsigned int)(stack_limit - heap_end);
}

unsigned int debugHeapApproxFreeBytes(void)
{
	if(debug_heap_start == NULL) debugHeapFreeBytes();
	if(debug_heap_start == NULL) return 0;
	char *stack_limit = &__StackLimit;
	if(debug_heap_start >= stack_limit) return 0;
	unsigned int capacity = (unsigned int)(stack_limit - debug_heap_start);
	if(capacity <= debug_alloc_current) return 0;
	return capacity - debug_alloc_current;
}

static const char *debug_stage_name(unsigned int stage)
{
	switch(stage)
	{
		case 0: return "idle";
		case 1: return "lvgl";
		case 2: return "http";
		case 3: return "core0_loop";
		case 10: return "tud";
		case 11: return "cdc";
		case 12: return "core1_loop";
		case 20: return "page_change";
		case 21: return "snapshot";
		case 22: return "lfs";
		case 23: return "malloc";
		default: return "unknown";
	}
}

void debugStage(unsigned char core, unsigned int stage)
{
	if(core == 0) debug_core0_stage = stage;
	else debug_core1_stage = stage;
}

void debugHeartbeat(unsigned char core)
{
	if(core == 0) debug_core0_heartbeat++;
	else debug_core1_heartbeat++;
}

void debugMallinfoSample(unsigned int arena, unsigned int ordblks, unsigned int uordblks, unsigned int fordblks, unsigned int keepcost)
{
	debug_mallinfo_arena = arena;
	debug_mallinfo_ordblks = ordblks;
	debug_mallinfo_uordblks = uordblks;
	debug_mallinfo_fordblks = fordblks;
	debug_mallinfo_keepcost = keepcost;
}

void debugHeapSample(void)
{
	debug_last_heap_free = debugHeapFreeBytes();
	if(debug_last_heap_free < debug_min_heap_free) debug_min_heap_free = debug_last_heap_free;
}

void debugEvent(const char *tag, int value)
{
#if MAGIC63_ENABLE_DEBUG_OUTPUT
	char line[144];
	int len = snprintf(line, sizeof(line),
	                   "[e] %s v=%d bf=%u bm=%u af=%u a=%u p=%u f=%u\r\n",
	                   tag == NULL ? "null" : tag, value, debug_last_heap_free, debug_min_heap_free,
	                   debugHeapApproxFreeBytes(), debug_alloc_current, debug_alloc_peak, debug_alloc_fail);
	cdc_debug_queue_line(line, sizeof(line), len);

	len = snprintf(line, sizeof(line),
	               "[m] ar=%u ord=%u u=%u f=%u k=%u\r\n",
	               debug_mallinfo_arena, debug_mallinfo_ordblks,
	               debug_mallinfo_uordblks, debug_mallinfo_fordblks,
	               debug_mallinfo_keepcost);
	cdc_debug_queue_line(line, sizeof(line), len);
#else
	(void)tag;
	(void)value;
#endif
}

void debugEventText(const char *tag, const char *text, int value)
{
#if MAGIC63_ENABLE_DEBUG_OUTPUT
	char line[160];
	int len = snprintf(line, sizeof(line),
	                   "[e] %s v=%d t=%s bf=%u bm=%u af=%u a=%u p=%u f=%u\r\n",
	                   tag == NULL ? "null" : tag, value, text == NULL ? "null" : text,
	                   debug_last_heap_free, debug_min_heap_free,
	                   debugHeapApproxFreeBytes(), debug_alloc_current, debug_alloc_peak, debug_alloc_fail);
	cdc_debug_queue_line(line, sizeof(line), len);
#else
	(void)tag;
	(void)text;
	(void)value;
#endif
}

void debugProbeTask(void)
{
#if MAGIC63_ENABLE_DEBUG_OUTPUT
	uint32_t now = to_ms_since_boot(get_absolute_time());
	if((uint32_t)(now - debug_last_probe_ms) < 1000u) return;
	debug_last_probe_ms = now;
	debugHeapSample();

	char line[192];
	int len = snprintf(line, sizeof(line),
	                   "[h] h0=%u h1=%u s0=%u:%s s1=%u:%s bf=%u bm=%u af=%u a=%u p=%u f=%u\r\n",
	                   debug_core0_heartbeat, debug_core1_heartbeat,
	                   debug_core0_stage, debug_stage_name(debug_core0_stage),
	                   debug_core1_stage, debug_stage_name(debug_core1_stage),
	                   debug_last_heap_free, debug_min_heap_free,
	                   debugHeapApproxFreeBytes(), debug_alloc_current, debug_alloc_peak, debug_alloc_fail);
	cdc_debug_queue_line(line, sizeof(line), len);
#endif
}

static void printchar(char **str, int c) {
	if (str) {
		**str = c;
		++(*str);
	} else
		(void) putchar(c);
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad) {
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr)
			++len;
		if (len >= width)
			width = 0;
		else
			width -= len;
		if (pad & PAD_ZERO)
			padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for (; width > 0; --width) {
			printchar(out, padchar);
			++pc;
		}
	}
	for (; *string; ++string) {
		printchar(out, *string);
		++pc;
	}
	for (; width > 0; --width) {
		printchar(out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad,
		int letbase) {
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN - 1;
	*s = '\0';

	while (u) {
		t = u % b;
		if (t >= 10)
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if (width && (pad & PAD_ZERO)) {
			printchar(out, '-');
			++pc;
			--width;
		} else {
			*--s = '-';
		}
	}

	return pc + prints(out, s, width, pad);
}

static int print(char **out, const char *format, va_list args) {
	register int width, pad;
	register int pc = 0;
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0')
				break;
			if (*format == '%')
				goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for (; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if (*format == 's') {
				register char *s = (char *) va_arg( args, int );
				pc += prints(out, s ? s : "(null)", width, pad);
				continue;
			}
			if (*format == 'd') {
				pc += printi(out, va_arg( args, int ), 10, 1, width, pad, 'a');
				continue;
			}
			if (*format == 'x') {
				pc += printi(out, va_arg( args, int ), 16, 0, width, pad, 'a');
				continue;
			}
			if (*format == 'X') {
				pc += printi(out, va_arg( args, int ), 16, 0, width, pad, 'A');
				continue;
			}
			if (*format == 'u') {
				pc += printi(out, va_arg( args, int ), 10, 0, width, pad, 'a');
				continue;
			}
			if (*format == 'c') {
				/* char are converted to int then pushed on the stack */
				scr[0] = (char) va_arg( args, int );
				scr[1] = '\0';
				pc += prints(out, scr, width, pad);
				continue;
			}
		} else {
			out: printchar(out, *format);
			++pc;
		}
	}
	if (out)
		**out = '\0';
	return pc;
}


void cdc_task(void)
{
	debugProbeTask();
#if MAGIC63_ENABLE_USB_CDC_DEBUG
	static unsigned char last_ready = 0;
	unsigned char ready = cdc_debug_ready();
	if(ready != last_ready)
	{
		last_ready = ready;
#if MAGIC63_ENABLE_DEBUG_OUTPUT
		char line[48];
		int len = snprintf(line, sizeof(line), "[e] cdc_ready v=%u\r\n", ready);
		cdc_debug_queue_line(line, sizeof(line), len);
#endif
	}
	if ( tud_cdc_n_available(0) )
	{
		uint8_t buf[64];
		tud_cdc_n_read(0, buf, sizeof(buf));
	}
	if(ready)
	{
		char out[64];
		unsigned int len = cdc_debug_dequeue(out, sizeof(out));
		if(len > 0)
		{
			tud_cdc_n_write(0, out, len);
			tud_cdc_n_write_flush(0);
		}
	}
#endif
}
  

void echo_serial_port(uint8_t itf, uint8_t buf[], uint32_t count)
{
#if MAGIC63_ENABLE_USB_CDC_DEBUG
	if(itf != 0) return;
	cdc_debug_queue((const char *)buf, count);
#else
	(void)itf;
	(void)buf;
	(void)count;
#endif
}

#if MAGIC63_ENABLE_USB_CDC_DEBUG
static void cdc_debug_out_chars(const char *buf, int len)
{
	if(len <= 0) return;
	cdc_debug_queue(buf, (unsigned int)len);
}

static void cdc_debug_out_flush(void)
{
}

static int cdc_debug_in_chars(char *buf, int len)
{
	if(!tud_ready() || len <= 0 || tud_cdc_n_available(0) == 0) return PICO_ERROR_NO_DATA;
	return (int)tud_cdc_n_read(0, buf, (uint32_t)len);
}

void cdcDebugInit(void)
{
	critical_section_init(&cdc_debug_lock);
	cdc_debug_inited = 1;
	stdio_set_driver_enabled(&cdc_debug_stdio, true);
	stdio_set_translate_crlf(&cdc_debug_stdio, true);
#if MAGIC63_ENABLE_DEBUG_OUTPUT
	char line[48];
	int len = snprintf(line, sizeof(line), "[e] cdc_init v=1\r\n");
	cdc_debug_queue_line(line, sizeof(line), len);
#endif
}
#else
void cdcDebugInit(void)
{
}
#endif

static int cdc_vprintf(const char* format, va_list va)
{
#if MAGIC63_ENABLE_USB_CDC_DEBUG
    char temp[256] = {0};
    char *p = temp;
    print(&p,format,va);
    unsigned int len = strlen(temp);
    echo_serial_port(0,(unsigned char *)temp,len);
	return (int)len;
#else
	(void)format;
	(void)va;
	return 0;
#endif
}

void userPrintf(const char* format, ...)
{
    va_list va;
    va_start(va, format);
    cdc_vprintf(format, va);
    va_end(va);
}
