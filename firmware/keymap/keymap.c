#include "stdio.h"

/* ---------------------------------------------
键值定义: 
    short 无符号 2个字节 16位 
        高8位确定按键类型  
            0：普通键盘 
            1：鼠标功能
            2：多媒体键
            3：宏功能
            4: 层选择 0 - 8 对应8层
            5: 屏幕功能页跳转
        低8位键值：
            键盘：对应hid键值+8，1-8，功能按键占用 
            鼠标：低4位 0 -2 ，鼠标左中右3个按键，高4位滚轮上下
            对媒体： 
            宏功能， 
            屏幕功能页跳转：0x01-0x7f 为主功能页序号，0x81-0xff 为对应主功能的内部页序号
*/

unsigned short keymap1[]={
    0xFFFFu+0u, 0xFFFFu+0u, 0x0029u+8u, 0x001eu+8u, 0x001fu+8u, 0x0020u+8u, 0x0021u+8u, 0x0022u+8u, 0x0023u+8u, 0x0024u+8u, 0x0025u+8u, 0x0026u+8u, 0x0027u+8u, 0x002Du+8u, 0x002eu+8u, 0x002au+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x002bu+8u, 0x0014u+8u, 0x001au+8u, 0x0008u+8u, 0x0015u+8u, 0x0017u+8u, 0x001cu+8u, 0x0018u+8u, 0x000cu+8u, 0x0012u+8u, 0x0013u+8u, 0x002fu+8u, 0x0030u+8u, 0x0031u+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x0039u+8u, 0x0004u+8u, 0x0016u+8u, 0x0007u+8u, 0x0009u+8u, 0x000au+8u, 0x000bu+8u, 0x000du+8u, 0x000eu+8u, 0x000fu+8u, 0x0033u+8u, 0x0034u+8u, 0x0028u+8u, 0xFFFFu+0u, 
    0xFFFFu+0u, 0xFFFFu+0u, 0x0001u+0u, 0x001du+8u, 0x001bu+8u, 0x0006u+8u, 0x0019u+8u, 0x0005u+8u, 0x0011u+8u, 0x0010u+8u, 0x0036u+8u, 0x0037u+8u, 0x0038u+8u, 0x0052u+8u, 0x0005u+0u, 0x004fu+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x0000u+0u, 0x0003u+0u, 0x0002u+0u, 0xFFFFu+0u, 0xFFFFu+0u, 0x002cu+8u, 0xFFFFu+0u, 0xFFFFu+0u, 0x0006u+0u, 0x0402u+0u, 0x0050u+8u, 0x0051u+8u, 0xFFFFu+0u, 0xFFFFu+0u
};

unsigned short keymap2[]={
    0xFFFFu+0u, 0xFFFFu+0u, 0x0029u+8u, 0x001eu+8u, 0x001fu+8u, 0x0020u+8u, 0x0021u+8u, 0x0022u+8u, 0x0023u+8u, 0x0024u+8u, 0x0025u+8u, 0x0026u+8u, 0x0027u+8u, 0x002Du+8u, 0x002eu+8u, 0x002au+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x002bu+8u, 0x0014u+8u, 0x001au+8u, 0x0008u+8u, 0x0015u+8u, 0x0017u+8u, 0x001cu+8u, 0x0018u+8u, 0x000cu+8u, 0x0012u+8u, 0x0013u+8u, 0x002fu+8u, 0x0030u+8u, 0x0031u+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x0039u+8u, 0x0004u+8u, 0x0016u+8u, 0x0007u+8u, 0x0009u+8u, 0x000au+8u, 0x000bu+8u, 0x000du+8u, 0x000eu+8u, 0x000fu+8u, 0x0033u+8u, 0x0034u+8u, 0x0028u+8u, 0xFFFFu+0u, 
    0xFFFFu+0u, 0xFFFFu+0u, 0x0001u+0u, 0x001du+8u, 0x001bu+8u, 0x0006u+8u, 0x0019u+8u, 0x0005u+8u, 0x0011u+8u, 0x0010u+8u, 0x0036u+8u, 0x0037u+8u, 0x0038u+8u, 0x0052u+8u, 0x0005u+0u, 0x004fu+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x0000u+0u, 0x0003u+0u, 0x0002u+0u, 0xFFFFu+0u, 0xFFFFu+0u, 0x002cu+8u, 0xFFFFu+0u, 0xFFFFu+0u, 0x0006u+0u, 0x0403u+0u, 0x0050u+8u, 0x0051u+8u, 0xFFFFu+0u, 0xFFFFu+0u
};


unsigned short keymap3[]={
    0xFFFFu+0u, 0xFFFFu+0u, 0x0029u+8u, 0x001eu+8u, 0x001fu+8u, 0x0020u+8u, 0x0021u+8u, 0x0022u+8u, 0x0023u+8u, 0x0024u+8u, 0x0025u+8u, 0x0026u+8u, 0x0027u+8u, 0x002Du+8u, 0x002eu+8u, 0x002au+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x002bu+8u, 0x0014u+8u, 0x001au+8u, 0x0008u+8u, 0x0015u+8u, 0x0017u+8u, 0x001cu+8u, 0x0018u+8u, 0x000cu+8u, 0x0012u+8u, 0x0013u+8u, 0x002fu+8u, 0x0030u+8u, 0x0031u+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x0039u+8u, 0x0004u+8u, 0x0016u+8u, 0x0007u+8u, 0x0009u+8u, 0x000au+8u, 0x000bu+8u, 0x000du+8u, 0x000eu+8u, 0x000fu+8u, 0x0033u+8u, 0x0034u+8u, 0x0028u+8u, 0xFFFFu+0u, 
    0xFFFFu+0u, 0xFFFFu+0u, 0x0001u+0u, 0x001du+8u, 0x001bu+8u, 0x0006u+8u, 0x0019u+8u, 0x0005u+8u, 0x0011u+8u, 0x0010u+8u, 0x0036u+8u, 0x0037u+8u, 0x0038u+8u, 0x0052u+8u, 0x0005u+0u, 0x004fu+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x0000u+0u, 0x0003u+0u, 0x0002u+0u, 0xFFFFu+0u, 0xFFFFu+0u, 0x002cu+8u, 0xFFFFu+0u, 0xFFFFu+0u, 0x0006u+0u, 0x0404u+0u, 0x0050u+8u, 0x0051u+8u, 0xFFFFu+0u, 0xFFFFu+0u
};

unsigned short keymap4[]={
    0xFFFFu+0u, 0xFFFFu+0u, 0x0029u+8u, 0x001eu+8u, 0x001fu+8u, 0x0020u+8u, 0x0021u+8u, 0x0022u+8u, 0x0023u+8u, 0x0024u+8u, 0x0025u+8u, 0x0026u+8u, 0x0027u+8u, 0x002Du+8u, 0x002eu+8u, 0x002au+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x002bu+8u, 0x0014u+8u, 0x001au+8u, 0x0008u+8u, 0x0015u+8u, 0x0017u+8u, 0x001cu+8u, 0x0018u+8u, 0x000cu+8u, 0x0012u+8u, 0x0013u+8u, 0x002fu+8u, 0x0030u+8u, 0x0031u+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x0039u+8u, 0x0004u+8u, 0x0016u+8u, 0x0007u+8u, 0x0009u+8u, 0x000au+8u, 0x000bu+8u, 0x000du+8u, 0x000eu+8u, 0x000fu+8u, 0x0033u+8u, 0x0034u+8u, 0x0028u+8u, 0xFFFFu+0u, 
    0xFFFFu+0u, 0xFFFFu+0u, 0x0001u+0u, 0x001du+8u, 0x001bu+8u, 0x0006u+8u, 0x0019u+8u, 0x0005u+8u, 0x0011u+8u, 0x0010u+8u, 0x0036u+8u, 0x0037u+8u, 0x0038u+8u, 0x0052u+8u, 0x0005u+0u, 0x004fu+8u,
    0xFFFFu+0u, 0xFFFFu+0u, 0x0000u+0u, 0x0003u+0u, 0x0002u+0u, 0xFFFFu+0u, 0xFFFFu+0u, 0x002cu+8u, 0xFFFFu+0u, 0xFFFFu+0u, 0x0006u+0u, 0x0401u+0u, 0x0050u+8u, 0x0051u+8u, 0xFFFFu+0u, 0xFFFFu+0u
};


unsigned short  *keymap = keymap1;
unsigned short  *keymapList[] = {NULL,keymap1,keymap2,keymap3,keymap4}; 
unsigned char keymapChange(unsigned char layer,unsigned char layerTemp)
{
    if(layerTemp != 0xff)
    {
        if((layerTemp > 0) && (layerTemp <5))
            keymap = keymapList[layerTemp];
    }
    else
    { 
        if((layer > 0) && (layer <5))
            keymap = keymapList[layer];
    }
    return 0;
}


unsigned short *getKeyMap(unsigned char layer)
{
    if(layer > 4) return NULL;
    return keymapList[layer];
}
unsigned char GetlayerNumber(void);
unsigned char getlayerTempNumber(void);

unsigned short getKeymapByXY(unsigned char x,unsigned char y)
{
    unsigned char t = x*16 +y;
    unsigned char temp = GetlayerNumber();
    if(t > 159) return 0;
    if((getlayerTempNumber() != 0xff) && ((keymapList[temp][t] >> 8) == 4)) return keymapList[temp][t]; //当前在临时层，且主层这里是层切换按钮
    return keymap[t];//当前没在临时层

}  
unsigned char updateLayerTempNumber(unsigned char data);
void userPrintf(const char* format, ...);
void debugPrintf(const char* format, ...);

unsigned char layerChangeKeyHandle(unsigned char k,unsigned char layer)
{
    static unsigned char moState = 0;
    static unsigned char layerOld = 0;
    //userPrintf("layerChangeKeyHandle %d %d %d %d \n",k,layer,moState,layerOld);
    if(moState && (layerOld != layer))  return 0;   //当前切换到了临时层新进来的层序号和就的不一样，就不处理，一个只能按一个临时层的按键
    if(k == 0)
    {   
        layerOld = 0;
        if(moState != 0)
        {
             moState = 0;
             updateLayerTempNumber(0xff);
             return 2;
        }
        return 0;
    }
    if(moState) return 0;
    // userPrintf("layerChangeKeyHandle1 %d \n",layer);
    updateLayerTempNumber(layer);
    moState = 1;
    layerOld = layer;
    return 1;
}

#define HID3_KEY_COUNT 80
#define HID3_REPORT_DESC_SIZE (14 + (HID3_KEY_COUNT * 6) + 1)

// 根据keymap生成报表描述符。非普通键声明为 Constant Input，避免占位 usage 被主机识别成按键。
unsigned char reportBuff[HID3_REPORT_DESC_SIZE] = {0};

static unsigned char keymapValueToKeyboardUsage(unsigned short keyValue, unsigned char *usage)
{
    if((keyValue >> 8) != 0) return 0;

    unsigned char value = keyValue & 0xff;
    if(value >= 8) *usage = value - 8;
    else *usage = value + 0xe0;

    return 1;
}

unsigned char keymap2HidReport(void)
{
    int reportBuffCount = 0;
    const unsigned char header[] = {
        0x05, 0x01,     // Usage Page (Generic Desktop)
        0x09, 0x06,     // Usage (Keyboard)
        0xA1, 0x01,     // Collection (Application)
        0x05, 0x07,     // Usage Page (Keyboard)
        0x15, 0x00,     // Logical Minimum (0)
        0x25, 0x01,     // Logical Maximum (1)
        0x75, 0x01      // Report Size (1)
    };

    for(unsigned int i = 0; i < sizeof(header); i++)
    {
        reportBuff[reportBuffCount++] = header[i];
    }

    for(unsigned char i = 0; i < HID3_KEY_COUNT; i++)
    {
        unsigned char usage = 0;
        unsigned char isKeyboard = keymapValueToKeyboardUsage(keymap[i], &usage);
        reportBuff[reportBuffCount++] = 0x09;                 // Usage
        reportBuff[reportBuffCount++] = isKeyboard ? usage : 0x00;
        reportBuff[reportBuffCount++] = 0x95;                 // Report Count
        reportBuff[reportBuffCount++] = 0x01;
        reportBuff[reportBuffCount++] = 0x81;                 // Input
        reportBuff[reportBuffCount++] = isKeyboard ? 0x02 : 0x03;
    }

    reportBuff[reportBuffCount++] = 0xC0;                     // End Collection
    return 0;
}

unsigned char reportHid(void)
{
    for(int i = 0; i < 79; i++)
    {
        debugPrintf("%d %d", reportBuff[i*2], reportBuff[i*2+1]);
    }
    return 0;
}
