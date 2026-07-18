
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>


unsigned short *getDataBuff(void);

int goTOBootModeX(void)
{
    reset_usb_boot(0, 0);
    printf("goTOBootMode \r\n");
    return 0;
}

unsigned char bootCleck(void)
{
	static unsigned char flag = 0;
	if(flag) return 0;
	flag = 1;
	unsigned short *p =  getDataBuff();
	if(p[2] == 0x0050)
	{
		goTOBootModeX();
	}
	return 0;
}

