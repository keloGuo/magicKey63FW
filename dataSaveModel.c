#include "lfs.h"

extern unsigned short  *keymapList[]; 
unsigned char keymapLoad(unsigned char init);
lfs_t * fsInit(void);
//不同版本之间如果数据结构发生变化，就修改这个值，初始化的时候比对，不一样就重新恢复默认值
#define UPDATE_FLAG 0x02
static struct saveData {
    unsigned int updateFlag;
    unsigned char activeLayer;
    unsigned char rate;
    unsigned char statePageFunction;
} saveDataS = {
    0x01,         //版本识别
    0x01,         //默认第一层，
    0x00,         //默认回报间隔1ms\1000hz
    0x02,         //默认第一个功能，显示当前的回报率，应为其他功能还没有做
};

static lfs_t *lfsHandle;          //文件系统的句柄

unsigned char dataSaveInit(void)
{
    lfs_file_t lfsConfigData; //存储配置的文件句柄

    lfsHandle = fsInit(); //初始化文件系统

	lfs_file_open(lfsHandle, &lfsConfigData, "configData", LFS_O_RDWR | LFS_O_CREAT);  //打开文件，没有就创建
	lfs_file_read(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //读文件

    printf("dataSaveInit %d\r\n",saveDataS.updateFlag);

    if(saveDataS.updateFlag != UPDATE_FLAG)                                             //版本标志不一样，就恢复默认值
    {
        saveDataS.updateFlag = UPDATE_FLAG;
        saveDataS.activeLayer = 1;
        saveDataS.rate = 0;
        saveDataS.statePageFunction = 2;
        
        lfs_file_rewind(lfsHandle, &lfsConfigData);                                         //刷新一个文件
	    lfs_file_write(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //写入文件
        lfs_file_close(lfsHandle, &lfsConfigData);                                          //关闭文件
        keymapLoad(1);
        return 0;
    }
    lfs_file_close(lfsHandle, &lfsConfigData);      
    keymapLoad(0);
    return 0;
}

//获取存储的激活层信息
unsigned char dataSaveGetActiveLayer(void)
{
    return saveDataS.activeLayer;
}
//获取当前回报间隔
unsigned char dataSaveGetRate(void)
{
    return saveDataS.rate;
}

//状态页有个功能是可以自定义的
unsigned char dataSaveGetStatePageFunction(void)
{
    return saveDataS.statePageFunction;
}

//保存激活层
unsigned char dataSaveActiveLayer(unsigned char v)
{
    lfs_file_t lfsConfigData; //存储配置的文件句柄

    lfs_file_open(lfsHandle, &lfsConfigData, "configData", LFS_O_RDWR | LFS_O_CREAT);  //打开文件，没有就创建
    lfs_file_read(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //读文件
    
    saveDataS.activeLayer = v;

    lfs_file_rewind(lfsHandle, &lfsConfigData);                                         //刷新一个文件
    lfs_file_write(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //写入文件
    lfs_file_close(lfsHandle, &lfsConfigData);                                          //关闭文件

    return 0;
}
//保存回报率
unsigned char dataSaveRate(unsigned char v)
{
    lfs_file_t lfsConfigData; //存储配置的文件句柄

    lfs_file_open(lfsHandle, &lfsConfigData, "configData", LFS_O_RDWR | LFS_O_CREAT);  //打开文件，没有就创建
    lfs_file_read(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //读文件
    
    saveDataS.rate = v;
    
    lfs_file_rewind(lfsHandle, &lfsConfigData);                                         //刷新一个文件
    lfs_file_write(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //写入文件
    lfs_file_close(lfsHandle, &lfsConfigData);                                          //关闭文件
    return 0;
}

//保存 状态页自定义的功能
unsigned char dataSaveStatePageFunction(unsigned char v)
{
    lfs_file_t lfsConfigData; //存储配置的文件句柄

    lfs_file_open(lfsHandle, &lfsConfigData, "configData", LFS_O_RDWR | LFS_O_CREAT);  //打开文件，没有就创建
    lfs_file_read(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //读文件
    
    saveDataS.statePageFunction = v;
    
    lfs_file_rewind(lfsHandle, &lfsConfigData);                                         //刷新一个文件
    lfs_file_write(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //写入文件
    lfs_file_close(lfsHandle, &lfsConfigData);                                          //关闭文件
    return 0;
}


unsigned char dataSaveGetActiveLayer(void);
unsigned char dataSaveGetRate(void);
unsigned char dataSaveActiveLayer(unsigned char v);
unsigned char dataSaveRate(unsigned char v);

unsigned char getFlashLayerInfo(void)
{
    return dataSaveGetActiveLayer();
}

unsigned char getFlashRateInfo(void)
{
    return dataSaveGetRate();
}

unsigned char writeFlashLayerInfo(unsigned char layer)
{
    dataSaveActiveLayer(layer);
    return 0;
}

unsigned char writeFlashRateInfo(unsigned char rate)
{
    dataSaveRate(rate);
    return 0;
}

//从文件系统加载键值信息
unsigned char keymapLoad(unsigned char init)
{
    lfs_file_t keymap; //存储配置的文件句柄
    for(int i = 1;i<5;i++)
    {
        char temp[30] = {'\0'};
        sprintf(temp,"keymapLayer%d",i);
        lfs_file_open(lfsHandle, &keymap, temp, LFS_O_RDWR | LFS_O_CREAT);  //打开文件，没有就创建
        if(init) 
        {
            lfs_file_rewind(lfsHandle, &keymap);                                         //刷新一个文件
            lfs_file_write(lfsHandle, &keymap, keymapList[i], 2*80);           //写入文件
        }
        else lfs_file_read(lfsHandle, &keymap, keymapList[i], 2*80);           //读文件
        lfs_file_close(lfsHandle, &keymap); 
    }
    return 0;
}

unsigned char keymapSave(unsigned char layer)
{
    lfs_file_t keymap; //存储配置的文件句柄

    char temp[30] = {'\0'};
    sprintf(temp,"keymapLayer%d",layer);
    lfs_file_open(lfsHandle, &keymap, temp, LFS_O_RDWR | LFS_O_CREAT);  //打开文件，没有就创建

    
    lfs_file_rewind(lfsHandle, &keymap);                                         //刷新一个文件
    lfs_file_write(lfsHandle, &keymap, keymapList[layer], 2*80);           //写入文件
    
    lfs_file_close(lfsHandle, &keymap); 
    
    return 0;
}