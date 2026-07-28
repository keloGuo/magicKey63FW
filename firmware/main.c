// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "hardware/clocks.h"
#include "tusb.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include <malloc.h>

unsigned char keyboardReportInit(void);
unsigned char rndisInit(void);
unsigned char mgLoops(void);
void webBounceDiagPollTimeout(void);
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
void cdcDebugInit(void);
void debugStage(unsigned char core, unsigned int stage);
void debugHeartbeat(unsigned char core);
void debugMallinfoSample(unsigned int arena, unsigned int ordblks, unsigned int uordblks, unsigned int fordblks, unsigned int keepcost);
void debugEvent(const char *tag, int value);
void debugEventText(const char *tag, const char *text, int value);
void debugPrintf(const char* format, ...);
void scratchInit(void);
void keyBounceDiagPoll(void);
void keyBounceDiagCancel(void);
void keyboardReportSetPaused(unsigned char paused);
void magic63UsbSerialInit(void);
void magic63RndisMacInit(void);

void bounceDiagUiExitRequest(void)
{
	keyBounceDiagCancel();
	keyboardReportSetPaused(0);
}

#define PLL_SYS_KHZ (250 * 1000)
unsigned char sysClockInit(void)
{
	set_sys_clock_khz(PLL_SYS_KHZ, true);
	debugPrintf("clock_get_hz = %d \r\n", clock_get_hz(clk_sys));
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
	cdcDebugInit();
	debugEventText("boot", "cdc_debug_init", 0);
	scratchInit();
	debugEventText("boot", "scratch_init", 0);
	multicore_lockout_victim_init();
	debugPrintf("cpu lvgl id %d \r\n", *((unsigned int *)(0xd0000000)));
	debugPrintf("clock_get_hz = %d \r\n", clock_get_hz(clk_sys));

	multicore_launch_core1(core1_main);
	debugEventText("boot", "core1_launch", 0);
	dataSaveInit();
	debugEventText("boot", "data_save", 0);
	keymap2HidReport();
	debugEventText("boot", "hid_desc", 0);
	st7735Int();
	debugEventText("boot", "lcd", 0);
	lv_init();
	lv_port_disp_init();
	debugEventText("boot", "lvgl", 0);
	
	magic63UI();
	debugEventText("boot", "ui", 0);

	magic63UsbSerialInit();
	debugEventText("boot", "usb_serial", 0);
	magic63RndisMacInit();
	debugEventText("boot", "rndis_mac", 0);
	tusb_init();
	debugEventText("boot", "tusb", 0);
	rndisInit();			
	debugEventText("boot", "rndis", 0);

	//encoderTmierInit();
	apmTimerInit();
	//uiDataUpdateInit();
	ws2812Init();
	debugEventText("boot", "ws2812", 0);

	tftBackLightCtrlInit();
	debugEventText("boot", "main_loop", 0);

    while(true) 
    {
		debugStage(0, 3);
		encoderProcess(NULL);
		UIdataUpdate(NULL);
		debugStage(0, 1);
		lv_task_handler(); 
		debugStage(0, 2);
		mgLoops();
		webBounceDiagPollTimeout();
		struct mallinfo mi = mallinfo();
		debugMallinfoSample((unsigned int)mi.arena, (unsigned int)mi.ordblks,
		                    (unsigned int)mi.uordblks, (unsigned int)mi.fordblks,
		                    (unsigned int)mi.keepcost);
		debugHeartbeat(0);
    }
  	return 0;
}
void cdc_task(void);
void core1_main(void)
{
	multicore_lockout_victim_init();
	debugEventText("core1", "start", 0);
	keyboardScanInit();
	debugEventText("core1", "scan_init", 0);
	sleep_ms(20);
	bootCleck();
	debugEventText("core1", "boot_check", 0);
	keyboardReportInit();
	debugEventText("core1", "hid_report", 0);
	encoderKeyCleckInit();
	debugEventText("core1", "encoder_key", 0);
	for (;;) 
	{
		debugStage(1, 10);
		tud_task();
		debugStage(1, 11);
		cdc_task();
		keyBounceDiagPoll();
		debugStage(1, 12);
		debugHeartbeat(1);
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

