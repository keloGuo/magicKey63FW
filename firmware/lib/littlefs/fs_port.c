#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "pico/multicore.h"

#include "lfs.h"

// 1.5M 后的 512K 做 littlefs，合并原 256K HTML 区和 256K littlefs 区。
#define LITTLE_FS_FLASH_BASE (1024 + 512)*1024
#define LITTLE_FS_FLASH_BASE_XIP (XIP_BASE + LITTLE_FS_FLASH_BASE)
#define GIF_FS_FLASH_BASE ((1024)*1024)
#define GIF_FS_FLASH_SIZE (512 * 1024)
#define HTML_FILE_NAME "index.html"
/**
 * lfs与底层flash读数据接口
 * @param  c
 * @param  block  块编号
 * @param  off    块内偏移地址
 * @param  buffer 用于存储读取到的数据
 * @param  size   要读取的字节数
 * @return
 */
static int lfs_deskio_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
	//W25QXX_Read((uint8_t *)buffer, c->block_size * block + off, size);
	unsigned char *p = (unsigned char *)(LITTLE_FS_FLASH_BASE_XIP + c->block_size * block + off);
	memcpy(buffer,p,size);
	return LFS_ERR_OK;
}

/**
 * lfs与底层flash写数据接口
 * @param  c
 * @param  block  块编号
 * @param  off    块内偏移地址
 * @param  buffer 待写入的数据
 * @param  size   待写入数据的大小
 * @return
 */
static int lfs_deskio_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
	//W25QXX_Write_NoCheck((uint8_t *)buffer, c->block_size * block + off, size);

    multicore_lockout_start_blocking();
    uint32_t status = save_and_disable_interrupts();
    flash_range_program(LITTLE_FS_FLASH_BASE + c->block_size * block + off, buffer, size);
    restore_interrupts(status);
    multicore_lockout_end_blocking();

	return LFS_ERR_OK;
}

/**
 * lfs与底层flash擦除接口
 * @param  c
 * @param  block 块编号
 * @return
 */
static int lfs_deskio_erase(const struct lfs_config *c, lfs_block_t block)
{
	//W25QXX_Erase_Sector(block);
	multicore_lockout_start_blocking();
    uint32_t status = save_and_disable_interrupts();
    flash_range_erase(LITTLE_FS_FLASH_BASE + c->block_size * block, FLASH_SECTOR_SIZE);
    restore_interrupts(status);
    multicore_lockout_end_blocking();
	return LFS_ERR_OK;
}

static int lfs_deskio_sync(const struct lfs_config *c)
{
	return LFS_ERR_OK;
}

// lfs句柄
lfs_t lfs_rp2040;

lfs_file_t lfs_file_rp2040;

const struct lfs_config cfg =
{
	// block device operations
	.read  = lfs_deskio_read,
	.prog  = lfs_deskio_prog,
	.erase = lfs_deskio_erase,
	.sync  = lfs_deskio_sync,

	// block device configuration
	.read_size = 16,
	.prog_size = 256,
	.block_size = 4096,
	.block_count = 128,
	.cache_size = 256,
	.lookahead_size = 16,
	.block_cycles = 500,
};

// entry point
lfs_t *lfsInit(void)
{
	static unsigned char mounted = 0;
	if (mounted) return &lfs_rp2040;

	// mount the filesystem
	int err = lfs_mount(&lfs_rp2040, &cfg);

	// reformat if we can't mount the filesystem
	// this should only happen on the first boot
	if (err)
	{
		lfs_format(&lfs_rp2040, &cfg);
		err = lfs_mount(&lfs_rp2040, &cfg);
	}
	if (!err) mounted = 1;

	// // read current count
	// uint32_t boot_count = 0;
	// lfs_file_open(&lfs_rp2040, &lfs_file_rp2040, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
	// lfs_file_read(&lfs_rp2040, &lfs_file_rp2040, &boot_count, sizeof(boot_count));

	// // update boot count
	// boot_count += 1;
	// lfs_file_rewind(&lfs_rp2040, &lfs_file_rp2040);
	// lfs_file_write(&lfs_rp2040, &lfs_file_rp2040, &boot_count, sizeof(boot_count));

	// // remember the storage is not updated until the file is closed successfully
	// lfs_file_close(&lfs_rp2040, &lfs_file_rp2040);

	// // release any resources we were using
	// lfs_unmount(&lfs_rp2040);

	// // print the boot count
	// printf("boot_count: %d\n", boot_count);
	return &lfs_rp2040;
}


lfs_t * fsInit(void)
{
	return lfsInit();
}

int fsUsedBytes(void)
{
	lfs_ssize_t usedBlocks = lfs_fs_size(fsInit());
	if(usedBlocks < 0) return -1;
	return usedBlocks * cfg.block_size;
}

unsigned int fsTotalBytes(void)
{
	return cfg.block_size * cfg.block_count;
}

unsigned int fsBlockSize(void)
{
	return cfg.block_size;
}

unsigned int fsBlockCount(void)
{
	return cfg.block_count;
}


//这个函数调用的时候需要手动关中断，停核心
unsigned char flashErase(unsigned int addr,unsigned int len)
{
	int lenTemp = len;
    for(int i = 0;;i++)
    {
		if(lenTemp > 0)
		{
			flash_range_erase(addr + (i * 4096), FLASH_SECTOR_SIZE);
			//printf("flash_range_erase%08x \r\n",addr + (i * 4096));
			lenTemp -= 4096;
		}
		else
		{
			break;
		}
    }
	return 0;
}

unsigned char flashWrite(unsigned int addr,unsigned char *p,unsigned int len)
{
	int lenTemp = len;
	for(int i = 0;i<(lenTemp/256);i++)
	{
		flash_range_program(addr + i*256, (const uint8_t *)p + (i*256), 256);
	}
	flash_range_program(addr + (lenTemp/256) * 256 , (const uint8_t *)p + (lenTemp/256) * 256, lenTemp % 256);
	return 0;
}


unsigned char fsWriteHtmlFile(unsigned char *p,unsigned int len)
{
	p[len] = '\0';
	char *start = strstr((char *)p,"<!DOCTYPE html>");
	char *end = strstr((char *)p,"</html>");
	if(start == NULL || end == NULL) return 1;
	end += strlen("</html>");
	*end = '\0';
	//printf("start %08x, end = %08x len = %d\r\n",start,end,end - start);
	int lenTemp = end - start;

	lfs_file_t file;
	lfs_t *lfs = fsInit();
	if(lfs_file_open(lfs, &file, HTML_FILE_NAME, LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC) < 0) return 1;
	lfs_file_write(lfs, &file, start, lenTemp);
	lfs_file_close(lfs, &file);

	return 0;
}

//这里没有擦除
unsigned char fsEraseHtmlFile(void)
{
	lfs_remove(fsInit(), HTML_FILE_NAME);
	return 0;
}

//写之前注意，最后一包加上 '\0'
unsigned char fsWriteHtmlFilePakg(unsigned int offset,unsigned char *p,unsigned int len,unsigned char isOver)
{
	lfs_file_t file;
	lfs_t *lfs = fsInit();
	int flags = LFS_O_RDWR | LFS_O_CREAT;

	if(offset == 0) flags |= LFS_O_TRUNC;
	if(lfs_file_open(lfs, &file, HTML_FILE_NAME, flags) < 0) return 1;
	lfs_file_seek(lfs, &file, offset, LFS_SEEK_SET);
	lfs_file_write(lfs, &file, p, len);
	if(isOver) lfs_file_sync(lfs, &file);
	lfs_file_close(lfs, &file);

	return 0;
}

//写之前注意，最后一包加上 '\0'
unsigned char fsWriteGifFilePakg(unsigned int offset,unsigned char *p,unsigned int len,unsigned char isOver)
{
	if(offset > GIF_FS_FLASH_SIZE || len > GIF_FS_FLASH_SIZE || (offset + len) > GIF_FS_FLASH_SIZE) return 1;
	if(offset == 0)
	{
		if(len < 10) return 1;
		if(memcmp(p, "GIF", 3) != 0) return 1;
		unsigned int width = p[6] | (p[7] << 8);
		unsigned int height = p[8] | (p[9] << 8);
		if(width != 160 || height != 80) return 1;
	}

    multicore_lockout_start_blocking();                         //flash 写需要停掉另外一个核心
    uint32_t status = save_and_disable_interrupts();            //保存中断并停掉中断

	if((offset%4096) == 0) flashErase(GIF_FS_FLASH_BASE + offset,4096);
	flashWrite(GIF_FS_FLASH_BASE + offset,p,len);

    restore_interrupts(status);
    multicore_lockout_end_blocking();
	unsigned char gifReload(void);
	if(isOver) gifReload();

	return 0;
}

/* 文件系统操作：
	keymap/config 和网页 index.html 共用 512K littlefs。
	GIF 仍然放在独立 512K 固定 flash 区。
*/

/*
	keymap.bin 存储键值文件
	config.bin 存储配置，当前默认层，当前回报率，当前WEB SERVER是否开启,当前web server 是否默认开启
*/
