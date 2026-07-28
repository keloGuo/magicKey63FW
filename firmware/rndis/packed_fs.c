#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "packed_update_page.h"

void debugPrintf(const char* format, ...);

static struct packed_file {
  const char *name;
  const unsigned char *data;
  size_t size;
  time_t mtime;
} packed_files[3] = {
  {"/web_root/update/index.html", magic63_update_page_html, MAGIC63_UPDATE_PAGE_HTML_SIZE, 1685125117},
  {NULL, NULL, 0, 0},
  {NULL, NULL, 0, 0}
};

const unsigned char path[] = "/web_root/index/index.html";

unsigned char packedFilesWrite(unsigned char *p)
{
  debugPrintf("strlen((char *)p) = %d \n", strlen((char *)p));
  if(*p != 0xff)
  {
    packed_files[1].name = (const char *)path;
    packed_files[1].data = p;
    packed_files[1].size = strlen((char *)p);
    packed_files[1].mtime = 1685125117;
  }
  return 0;
}

unsigned char packedFilesDelete(void)
{
  packed_files[1].name = NULL;
  packed_files[1].data = NULL;
  packed_files[1].size = 0;
  packed_files[1].mtime = 0;

  return 0;
}

static int scmp(const char *a, const char *b)
{
  while (*a && (*a == *b)) a++, b++;
  return *(const unsigned char *)a - *(const unsigned char *)b;
}

const char *mg_unlist(size_t no);
const char *mg_unlist(size_t no)
{
  return packed_files[no].name;
}

const char *mg_unpack(const char *path, size_t *size, time_t *mtime);
const char *mg_unpack(const char *name, size_t *size, time_t *mtime)
{
  const struct packed_file *p;
  for (p = packed_files; p->name != NULL; p++) {
    if (scmp(p->name, name) != 0) continue;
    if (size != NULL) *size = p->size - 1;
    if (mtime != NULL) *mtime = p->mtime;
    return (const char *)p->data;
  }
  return NULL;
}

unsigned char packedFsInit(unsigned char *p)
{
  if(*p != 0xff)
  {
    packedFilesWrite(p);
  }
  return 0;
}
