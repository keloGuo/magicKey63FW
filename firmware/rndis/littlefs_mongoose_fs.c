#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "lfs.h"
#include "mongoose.h"

lfs_t *fsInit(void);
void debugStage(unsigned char core, unsigned int stage);
void debugHeapSample(void);
void debugEvent(const char *tag, int value);
void debugEventText(const char *tag, const char *text, int value);

static const char *lfs_path(const char *path)
{
  while (path != NULL && *path == '/') path++;
  return path == NULL ? "" : path;
}

static int lfs_mg_stat(const char *path, size_t *size, time_t *mtime)
{
  struct lfs_info info;
  const char *name = lfs_path(path);

  if (mtime != NULL) *mtime = 0;
  if (*name == '\0') {
    if (size != NULL) *size = 0;
    return MG_FS_DIR;
  }

  if (lfs_stat(fsInit(), name, &info) < 0) return 0;
  if (size != NULL) *size = info.size;
  return info.type == LFS_TYPE_DIR ? MG_FS_DIR : MG_FS_READ;
}

static void lfs_mg_list(const char *path, void (*fn)(const char *, void *),
                        void *userdata)
{
  lfs_dir_t dir;
  struct lfs_info info;
  const char *name = lfs_path(path);

  if (lfs_dir_open(fsInit(), &dir, *name == '\0' ? "/" : name) < 0) return;
  while (lfs_dir_read(fsInit(), &dir, &info) > 0) {
    if (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0) continue;
    fn(info.name, userdata);
  }
  lfs_dir_close(fsInit(), &dir);
}

static void *lfs_mg_open(const char *path, int flags)
{
  int lfs_flags = (flags & MG_FS_WRITE) ? (LFS_O_RDWR | LFS_O_CREAT) : LFS_O_RDONLY;
  debugStage(0, 22);
  debugHeapSample();
  lfs_file_t *file = calloc(1, sizeof(*file));

  if (file == NULL) {
    debugEvent("lfs_calloc_fail", 0);
    return NULL;
  }
  int err = lfs_file_open(fsInit(), file, lfs_path(path), lfs_flags);
  if (err < 0) {
    debugEventText("lfs_open_fail", lfs_path(path), err);
    free(file);
    return NULL;
  }
  return file;
}

static void lfs_mg_close(void *fd)
{
  if (fd == NULL) return;
  lfs_file_close(fsInit(), fd);
  free(fd);
}

static size_t lfs_mg_read(void *fd, void *buf, size_t len)
{
  debugStage(0, 22);
  lfs_ssize_t n = lfs_file_read(fsInit(), fd, buf, len);
  if (n < 0) debugEvent("lfs_read_fail", (int)n);
  return n < 0 ? 0 : (size_t)n;
}

static size_t lfs_mg_write(void *fd, const void *buf, size_t len)
{
  debugStage(0, 22);
  lfs_ssize_t n = lfs_file_write(fsInit(), fd, buf, len);
  if (n < 0) debugEvent("lfs_write_fail", (int)n);
  return n < 0 ? 0 : (size_t)n;
}

static size_t lfs_mg_seek(void *fd, size_t offset)
{
  debugStage(0, 22);
  lfs_soff_t pos = lfs_file_seek(fsInit(), fd, offset, LFS_SEEK_SET);
  if (pos < 0) debugEvent("lfs_seek_fail", (int)pos);
  return pos < 0 ? 0 : (size_t)pos;
}

static bool lfs_mg_rename(const char *from, const char *to)
{
  return lfs_rename(fsInit(), lfs_path(from), lfs_path(to)) == 0;
}

static bool lfs_mg_remove(const char *path)
{
  return lfs_remove(fsInit(), lfs_path(path)) == 0;
}

static bool lfs_mg_mkdir(const char *path)
{
  return lfs_mkdir(fsInit(), lfs_path(path)) == 0;
}

struct mg_fs mg_fs_littlefs = {
    lfs_mg_stat,  lfs_mg_list, lfs_mg_open,   lfs_mg_close,  lfs_mg_read,
    lfs_mg_write, lfs_mg_seek, lfs_mg_rename, lfs_mg_remove, lfs_mg_mkdir};
