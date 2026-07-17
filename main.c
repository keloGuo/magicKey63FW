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
unsigned char keyboardScanInit(void);
unsigned char st7735Int(void);
unsigned char uiDataUpdateInit(void);
unsigned char encoderTmierInit(void);
unsigned char dataSaveInit(void);
void core1_main(void);
void magic63UI(void);
unsigned char apmTimerInit(void);
unsigned char bootCleck(void);
unsigned char TestPrintf(void);
unsigned char encoderKeyCleckInit(void);
unsigned char keymap2HidReport(void);
unsigned char ws2812Init(void);
unsigned char tftBackLightCtrlInit(void);
unsigned char apmUpdataCleck(void);

#define PLL_SYS_KHZ (250 * 1000)
unsigned char sysClockInit(void)
{
	set_sys_clock_khz(PLL_SYS_KHZ, true);
	printf("clock_get_hz = %d \r\n",clock_get_hz(clk_sys));
	return 0;
}

unsigned char tftBackLightTest(void)
{
	gpio_init(12);
    gpio_set_dir(12, GPIO_OUT);
	gpio_put(12,1);
	return 0;
}
bool encoderProcess(repeating_timer_t *rt) ;
bool UIdataUpdate(repeating_timer_t *rt);
int main(void) 
{
	sysClockInit();
	stdio_init_all();
	multicore_lockout_victim_init();
	printf("cpu lvgl id %d \r\n",*((unsigned int *)(0xd0000000)));
	printf("clock_get_hz = %d \r\n",clock_get_hz(clk_sys));

	multicore_launch_core1(core1_main);
	dataSaveInit();
	keymap2HidReport();
	st7735Int();
	lv_init();
	lv_port_disp_init();
	
	magic63UI();

	tusb_init();
	rndisInit();			

	//encoderTmierInit();
	apmTimerInit();
	//uiDataUpdateInit();
	ws2812Init();

	tftBackLightCtrlInit();

    while(true) 
    {   
		encoderProcess(NULL);
		UIdataUpdate(NULL);
		lv_task_handler(); 
		mgLoops();
    }
  	return 0;
}
void cdc_task(void);
void core1_main(void)
{
	multicore_lockout_victim_init();
	keyboardScanInit();
	sleep_ms(20);
	bootCleck();
	keyboardReportInit();
	encoderKeyCleckInit();
	for (;;) 
	{
		tud_task();
		cdc_task();
		// TestPrintf();
	}
}

struct repeating_timer backLightTimer;

bool backLightTimerHandle(repeating_timer_t *rt)
{
	gpio_init(12);
    gpio_set_dir(12, GPIO_OUT);
	gpio_put(12,1);
	return 1;

}
unsigned char tftBackLightCtrlInit(void)
{
	add_repeating_timer_ms(60000,backLightTimerHandle,0,&backLightTimer);
	return 0;
}
unsigned char tftBackLightCtrlTimerRestart(void)
{
	cancel_repeating_timer(&backLightTimer);
	gpio_init(12);
    gpio_set_dir(12, GPIO_OUT);
	gpio_put(12,0);
	add_repeating_timer_ms(60000,backLightTimerHandle,0,&backLightTimer);
	return 0;
}

