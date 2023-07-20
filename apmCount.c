#include "pico/stdlib.h"
#include <stdio.h>


unsigned char swar(unsigned short temp)
{
    temp = (temp & 0x5555) + ((temp >> 1) & 0x5555) ;
    temp = (temp & 0x3333) + ((temp >> 2) & 0x3333) ;
    temp = (temp & 0x0F0F) + ((temp >> 4) & 0x0F0F) ;
    temp = (temp*(0x0101) >> 8);
    return temp;
}
unsigned int apmCount = 0;
static unsigned int apmBuff[60] = {0};
static unsigned int ApmSum = 0;
unsigned char __not_in_flash_func(AMPstatistics)(unsigned short *p)
{
    p[0] &= 0x3fff; //去掉最高的两位
    p[2] &= 0x3fff;

    for(int i = 0; i<5;i++)
    {
        apmCount += swar(p[i]);
    }
    return 0;
}


unsigned char APMShowUpdateStatePage(unsigned int apm);
unsigned char updataApmData(unsigned int a);
bool apmUpdata(repeating_timer_t *rt)
{
    static unsigned char count = 0;
    apmBuff[count] = apmCount;

    ApmSum += apmCount;
    
    count += 1;
    count = count % 60;
    ApmSum -= apmBuff[count];

    //printf("apm %d \r\n",ApmSum/2);
    updataApmData(ApmSum/2);
    APMShowUpdateStatePage(ApmSum/2);
    apmCount = 0;
    return 1;
}
struct repeating_timer apmTimer;
unsigned char apmTimerInit(void)
{
    add_repeating_timer_ms(1000,apmUpdata,0,&apmTimer);
    return 0;
}