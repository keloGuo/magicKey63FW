#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include "pico/multicore.h"

unsigned char keyboardRestartSet(void);
#define FLASH_TARGET_OFFSET ((512+1024 + 256 + 128 + 64) * 1024)
//先做一个缩水版本，后续一直fs
//第一个字节存储当前层，第二个字节存储当前回报率，1 = 1000hz 2 = 500hz 3= 333hz 4 =250hz 5 = 200hz 。。。。。

static const uint8_t *flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
unsigned char temp[256] = {1,2,0xff};
unsigned char getFlashLayerInfo(void)
{
    return *flash_target_contents;
}

unsigned char getFlashRateInfo(void)
{
    return *(flash_target_contents + 1);
}

unsigned char writeFlashLayerInfo(unsigned char layer)
{
    unsigned char temp[256] = {1,2,0xff};
    memset((char *)temp,0xff,256);
    temp[0] = layer;
    temp[1] = getFlashRateInfo();   
    multicore_lockout_start_blocking();
    uint32_t status = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, temp, 256);
    restore_interrupts(status);
    multicore_lockout_end_blocking();
    keyboardRestartSet();
    return 0;
}

unsigned char writeFlashRateInfo(unsigned char rate)
{
    unsigned char temp[256] = {1,2,0xff};
    memset((char *)temp,0xff,256);
    temp[0] = getFlashLayerInfo();
    temp[1] = rate;
    multicore_lockout_start_blocking();
    uint32_t status = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, temp, 256);
    restore_interrupts(status);
    multicore_lockout_end_blocking();
    keyboardRestartSet();
    return 0;
}
