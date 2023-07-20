#include "pico/stdlib.h"

void encoderCallback(int t,unsigned char key);
static unsigned int timeCount = 0;
//编码器扫描 这里只处理 AB相位，按键别的地方已经处理了
unsigned char  __not_in_flash_func( encoderScan)(unsigned short a,unsigned short b)
{
    static unsigned short aOld = 0; //按下时1，松开是0
    static unsigned short bOld = 0;
    
    if(timeCount) 
    {
        timeCount --;
        return 0;
    }

    if((a != aOld))//
    {   
        if((aOld != 0) && (b != 0))
        {
            encoderCallback(1,3);
            timeCount  = 30;
        }
        aOld = a;
    }

    if((b != bOld))
    {
        if((bOld != 0) && (a != 0))
        {
            encoderCallback(-1,3);
            timeCount  = 30;
        }
           
        bOld = b;
    }
    return 0;
}
//单独处理编码器按键
unsigned char  __not_in_flash_func(encoderKeyScan)(unsigned char newStete)
{
    static unsigned int count = 0;
    static unsigned char oldState = 1;
    count ++;
    unsigned char hop = (oldState != newStete);
    if(timeCount) 
    {
        timeCount --;
        return 0;
    }
    //printf("encoderKeyScan %d %d %d\r\n",hop,newStete,count);
    if(hop == 0) //没有变化
    {
        if((newStete  == 0)&&(count == 700)) //新值一直是0，所以一直是按下的
        {
            encoderCallback(0,2);//触发长按，
            timeCount  = 30;
        }
        return 0;
    }
    oldState = newStete;
    if(newStete  == 0) count =0; //刚刚按下，从新计数
    if(newStete == 1) //刚刚松开 
    {
        if((count <700) && (count > 10)) 
        {
            encoderCallback(0,1);//触发短按
            timeCount  = 30;
        }
    }
    return 0;
}

unsigned char  __not_in_flash_func( encoderScanKeyboard)(unsigned short a,unsigned short b,unsigned short *p)
{

    static unsigned int timeCount = 0;

    static unsigned short aOld = 0; //按下时1，松开是0
    static unsigned short bOld = 0;

    if(timeCount) 
    {
        timeCount --;
        return 0;
    }

    if((a != aOld))//
    {   
        if((aOld != 0) && (b != 0))
        {
            p[0] &= 0x7fff;
            timeCount  = 30;
        }
        aOld = a;
    }

    if((b != bOld))
    {
        if((bOld != 0) && (a != 0))
        {
            p[2]  &= 0x7fff;
            timeCount  = 30;
        }
        bOld = b;
    }
    return 0;
}