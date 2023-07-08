// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"

#include "tusb.h"
#include "lvgl.h"
#include "lv_port_disp.h"

unsigned char keyboardReportInit(void);
unsigned char rndisInit(void);
unsigned char mgLoops(void);
unsigned char dmaHandle(void);
unsigned char keyboardScanInit(void);
unsigned char st7735Int(void);
unsigned char UIdataUpdate(void);
unsigned char encoderProcess(void);
unsigned char dataSaveInit(void);
void core1_main(void);
void FK64UI(void);

unsigned char goUF2Boot(void)
{
	gpio_init(10);
	gpio_init(28);
	gpio_set_dir(10, GPIO_OUT);
	gpio_put(10, 0); 

	gpio_set_dir(28, GPIO_IN);
	gpio_pull_up(28);
	
	if(gpio_get(28) == 0)
	{
		reset_usb_boot(0, 0);
		while(1);
	}
	return 0;
}
int main(void) 
{
	stdio_init_all();
//	goUF2Boot();
	multicore_launch_core1(core1_main);
	dataSaveInit();

	st7735Int();
	lv_init();
    lv_port_disp_init();
	FK64UI();

	tusb_init();
	rndisInit();			

    while(true)
    {   
		lv_task_handler(); 
		mgLoops();
		UIdataUpdate();
		encoderProcess();
    }

  return 0;
}

void core1_main(void)
{
    multicore_lockout_victim_init();
	keyboardReportInit();
	keyboardScanInit();
	for (;;) 
	{
		tud_task();
		dmaHandle();
	}
}


