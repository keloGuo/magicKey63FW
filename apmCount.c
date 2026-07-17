#include "pico/stdlib.h"
#include <stdio.h>

unsigned short *getDataBuff(void);
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

bool __not_in_flash_func(AMPstatistics)(repeating_timer_t *rt)
{
    static unsigned short dataOld[5] = {0x00,0x00,0x00,0x00,0x00}; 
    unsigned short *p = getDataBuff();    
    
    for(int i = 0; i<5;i++)
    {
       apmCount += swar(p[i]^dataOld[i]);
       dataOld[i] = p[i];
    }

    return 1;
}

 unsigned char updataApm(unsigned int v);
static unsigned int ApmSum = 0;
bool apmUpdata(repeating_timer_t *rt)
{
    static unsigned char count = 0;
    apmBuff[count] = apmCount;
    apmCount = 0;
    ApmSum += apmBuff[count];
    count ++;
    count = count % 60;
    ApmSum -= apmBuff[count];
    updataApm(ApmSum/2);
    return 1;
}
struct repeating_timer apmTimer;
struct repeating_timer apmSumTimer;
unsigned char apmTimerInit(void)
{
    add_repeating_timer_ms(1000,apmUpdata,0,&apmTimer);
    add_repeating_timer_ms(5,AMPstatistics,0,&apmSumTimer);
    return 0;
}
