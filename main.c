// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "hardware/clocks.h"
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
unsigned char encoderTmierInit(void);
unsigned char dataSaveInit(void);
void core1_main(void);
void FK64UI(void);
unsigned char apmTimerInit(void);

#define PLL_SYS_KHZ (250 * 1000)
unsigned char sysClockInit(void)
{
	set_sys_clock_khz(PLL_SYS_KHZ, true);
	printf("clock_get_hz = %d \r\n",clock_get_hz(clk_sys));
	return 0;
}
// void __not_in_flash_func(some_function_name)(int arg1, int arg2) {
//     // ...
// }
unsigned char  __not_in_flash_func(testRamFunc)(char *p)
{
	printf("\r\ntestRamFunc %s\r\n",p);
	return 1;
}

int main(void) 
{
	sysClockInit();
	stdio_init_all();
	
	printf("testRamFunc %d %08x \r\n",testRamFunc("12345"),testRamFunc);

	multicore_launch_core1(core1_main);
	dataSaveInit();
	st7735Int();
	lv_init();
    lv_port_disp_init();
	FK64UI();

	tusb_init();
	rndisInit();			

	printf("PICO_FLASH_SPI_CLKDIV = %d",PICO_FLASH_SPI_CLKDIV);

	encoderTmierInit();
	apmTimerInit();
    while(true)
    {   
		lv_task_handler(); 
		mgLoops();
		UIdataUpdate();
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


