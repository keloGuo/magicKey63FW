#include "tusb.h"

unsigned char tempFlag = 0;
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    printf("itf %d ,report_id %d report_type %d , bufferlen = %d \n",itf,report_id,report_type,bufsize);
	unsigned char updateVirtuallyLed(unsigned char data);
  	if(report_id == 0 && report_type == 2) updateVirtuallyLed(buffer[0]);
    tempFlag = 1;
}
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
	return 0;
}

//根据新的矩阵数据，生成report数据，通过USB发送给PC
//这里用一个定时器，每1ms，一次，多了也收不过来

// unsigned char sendHidReport(unsigned char state)
// {
// 	if ( !tud_hid_ready() ) return 0;
// 	static  unsigned char temp[] = {0x00,0x00,0x00,0,0,0,0,0,0,0,0,0,0,0,0,0};

// 	unsigned char i = 20/8;
// 	unsigned char j = 20%8;

// 	temp[i] = state ? (temp[i] & (~(1 << j))) : (temp[i] | (1 << j)) ; 
// 	tud_hid_n_report(0,0,temp,16);
// 	printf("sendHidReport = %d \r\n",state);
// 	return 0;
// }

unsigned short *getkeyMatrixData(void);
unsigned short getKeymapByXY(unsigned char x,unsigned char y);
unsigned char layerChangeKeyHandle(unsigned char k,unsigned char layer);

bool keyMatrix2ReportData(repeating_timer_t *rt)
{
    if ( !tud_hid_ready() ) return true;
	unsigned char temp[] = {0x00,0x00,0x00,0,0,0,0,0,0,0,0,0,0,0,0,0};
    unsigned short *data = getkeyMatrixData();
    for(int i = 0;i< 5;i++)
    {
        for(int j = 0; j<16;j++)
        {
            unsigned short t = getKeymapByXY(4-i,j);
            if((t >> 8) == 0x04)
            {
                layerChangeKeyHandle((~data[i]) & (0x8000 >> j),(t&0xff));
            }
            else
            {
                if((~data[i]) & (0x8000 >> j))
                {
                    if(t>>8) continue;
                    temp[t/8] |= (1 << (t%8));
                }
            }

        }
    }
    tud_hid_n_report(0,0,temp,16);
    return true ;
}

struct repeating_timer keyboardReportTimer;
unsigned char keyboardReportInit(void)
{
    add_repeating_timer_ms(1,keyMatrix2ReportData,0,&keyboardReportTimer);
    return 0;
}