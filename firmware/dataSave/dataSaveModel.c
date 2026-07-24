#include "lfs.h"

extern unsigned short  *keymapList[]; 
unsigned char keymapLoad(unsigned char init);
lfs_t * fsInit(void);
void debugStage(unsigned char core, unsigned int stage);
void debugEvent(const char *tag, int value);
//不同版本之间如果数据结构发生变化，就修改这个值，初始化的时候比对，不一样就重新恢复默认值
#define UPDATE_FLAG 0x03
static struct saveData {
    unsigned int updateFlag;
    unsigned char activeLayer;
    unsigned char rate;
    unsigned char statePageFunction;
    unsigned char backLight;
} saveDataS = {
    0x01,         //版本识别
    0x01,         //默认第一层，
    0x01,         //默认回报间隔1ms\1000hz
    0x02,         //默认第一个功能，显示当前的回报率，应为其他功能还没有做
    0x05,
};

static lfs_t *lfsHandle;          //文件系统的句柄

static void dataSaveSetDefault(void)
{
    saveDataS.updateFlag = UPDATE_FLAG;
    saveDataS.activeLayer = 1;
    saveDataS.rate = 1;
    saveDataS.statePageFunction = 2;
    saveDataS.backLight = 5;
}

static void dataSaveLimit(void)
{
    if(saveDataS.activeLayer < 1 || saveDataS.activeLayer > 4) saveDataS.activeLayer = 1;
    if(saveDataS.rate < 1 || saveDataS.rate > 20) saveDataS.rate = 1;
    if(saveDataS.statePageFunction > 3) saveDataS.statePageFunction = 2;
    if(saveDataS.backLight > 20) saveDataS.backLight = 5;
}

static int dataSaveOpenConfig(lfs_file_t *file)
{
    debugStage(0, 22);
    int err = lfs_file_open(lfsHandle, file, "configData", LFS_O_RDWR | LFS_O_CREAT);
    if(err < 0) debugEvent("cfg_open_fail", err);
    return err;
}

static int dataSaveWriteConfig(lfs_file_t *file)
{
    lfs_file_rewind(lfsHandle, file);
    lfs_ssize_t len = lfs_file_write(lfsHandle, file, &saveDataS, sizeof(saveDataS));
    if(len != sizeof(saveDataS))
    {
        debugEvent("cfg_write_fail", (int)len);
        return -1;
    }
    return 0;
}

unsigned char dataSaveInit(void)
{
    lfs_file_t lfsConfigData; //存储配置的文件句柄

    lfsHandle = fsInit(); //初始化文件系统

	int err = dataSaveOpenConfig(&lfsConfigData);  //打开文件，没有就创建
	if(err < 0)
    {
        dataSaveSetDefault();
        keymapLoad(1);
        return 1;
    }
	lfs_ssize_t readLen = lfs_file_read(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //读文件

    printf("dataSaveInit %d\r\n",saveDataS.updateFlag);

    if(readLen != sizeof(saveDataS) || saveDataS.updateFlag != UPDATE_FLAG)                                             //版本标志不一样，就恢复默认值
    {
        dataSaveSetDefault();
        
        dataSaveWriteConfig(&lfsConfigData);           //写入文件
        lfs_file_close(lfsHandle, &lfsConfigData);                                          //关闭文件
        keymapLoad(1);
        return 0;
    }
    dataSaveLimit();
    lfs_file_close(lfsHandle, &lfsConfigData);      
    keymapLoad(0);
    return 0;
}

//获取存储的激活层信息
unsigned char dataSaveGetActiveLayer(void)
{
    dataSaveLimit();
    return saveDataS.activeLayer;
}
//获取当前回报间隔
unsigned char dataSaveGetRate(void)
{
    dataSaveLimit();
    return saveDataS.rate;
}

unsigned char dataSaveGetBackLight(void)
{
    dataSaveLimit();
    return saveDataS.backLight;
}


//状态页有个功能是可以自定义的
unsigned char dataSaveGetStatePageFunction(void)
{
    dataSaveLimit();
    return saveDataS.statePageFunction;
}

//保存激活层
unsigned char dataSaveActiveLayer(unsigned char v)
{
    lfs_file_t lfsConfigData; //存储配置的文件句柄
    if(v < 1 || v > 4) v = 1;

    if(dataSaveOpenConfig(&lfsConfigData) < 0) return 1;  //打开文件，没有就创建
    lfs_ssize_t readLen = lfs_file_read(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //读文件
    if(readLen != sizeof(saveDataS)) debugEvent("cfg_read_fail", (int)readLen);
    
    saveDataS.activeLayer = v;

    dataSaveWriteConfig(&lfsConfigData);           //写入文件
    lfs_file_close(lfsHandle, &lfsConfigData);                                          //关闭文件

    return 0;
}
//保存回报率
unsigned char dataSaveRate(unsigned char v)
{
    lfs_file_t lfsConfigData; //存储配置的文件句柄
    if(v < 1 || v > 20) v = 1;

    if(dataSaveOpenConfig(&lfsConfigData) < 0) return 1;  //打开文件，没有就创建
    lfs_ssize_t readLen = lfs_file_read(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //读文件
    if(readLen != sizeof(saveDataS)) debugEvent("cfg_read_fail", (int)readLen);
    
    saveDataS.rate = v;
    
    dataSaveWriteConfig(&lfsConfigData);           //写入文件
    lfs_file_close(lfsHandle, &lfsConfigData);                                          //关闭文件
    return 0;
}
//保存背光效果
unsigned char dataSaveBackLight(unsigned char v)
{
    lfs_file_t lfsConfigData; //存储配置的文件句柄

    if(dataSaveOpenConfig(&lfsConfigData) < 0) return 1;  //打开文件，没有就创建
    lfs_ssize_t readLen = lfs_file_read(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //读文件
    if(readLen != sizeof(saveDataS)) debugEvent("cfg_read_fail", (int)readLen);
    
    saveDataS.backLight = v;
    
    dataSaveWriteConfig(&lfsConfigData);           //写入文件
    lfs_file_close(lfsHandle, &lfsConfigData);                                          //关闭文件
    return 0;
}
//保存 状态页自定义的功能
unsigned char dataSaveStatePageFunction(unsigned char v)
{
    lfs_file_t lfsConfigData; //存储配置的文件句柄
    if(v > 3) v = 2;

    if(dataSaveOpenConfig(&lfsConfigData) < 0) return 1;  //打开文件，没有就创建
    lfs_ssize_t readLen = lfs_file_read(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //读文件
    if(readLen != sizeof(saveDataS)) debugEvent("cfg_read_fail", (int)readLen);
    
    saveDataS.statePageFunction = v;
    
    dataSaveWriteConfig(&lfsConfigData);           //写入文件
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

unsigned char writeFlashBackLightInfo(unsigned char v)
{
    dataSaveBackLight(v);
    return 0;
}

unsigned char getFlashBackLightInfo(void)
{
    return dataSaveGetBackLight();
}


//从文件系统加载键值信息
unsigned char keymapLoad(unsigned char init)
{
    lfs_file_t keymap; //存储配置的文件句柄
    for(int i = 1;i<5;i++)
    {
        char temp[30] = {'\0'};
        sprintf(temp,"keymapLayer%d",i);
        debugStage(0, 22);
        int err = lfs_file_open(lfsHandle, &keymap, temp, LFS_O_RDWR | LFS_O_CREAT);  //打开文件，没有就创建
        if(err < 0)
        {
            debugEvent("keymap_open_fail", err);
            continue;
        }
        if(init) 
        {
            lfs_file_rewind(lfsHandle, &keymap);                                         //刷新一个文件
            lfs_ssize_t len = lfs_file_write(lfsHandle, &keymap, keymapList[i], 2*80);           //写入文件
            if(len != 2*80) debugEvent("keymap_write_fail", (int)len);
        }
        else
        {
            lfs_ssize_t len = lfs_file_read(lfsHandle, &keymap, keymapList[i], 2*80);           //读文件
            if(len != 2*80) debugEvent("keymap_read_fail", (int)len);
        }
        lfs_file_close(lfsHandle, &keymap); 
    }
    return 0;
}

unsigned char keymapSave(unsigned char layer)
{
    lfs_file_t keymap; //存储配置的文件句柄

    char temp[30] = {'\0'};
    sprintf(temp,"keymapLayer%d",layer);
    debugStage(0, 22);
    int err = lfs_file_open(lfsHandle, &keymap, temp, LFS_O_RDWR | LFS_O_CREAT);  //打开文件，没有就创建
    if(err < 0)
    {
        debugEvent("keymap_open_fail", err);
        return 1;
    }

    
    lfs_file_rewind(lfsHandle, &keymap);                                         //刷新一个文件
    lfs_ssize_t len = lfs_file_write(lfsHandle, &keymap, keymapList[layer], 2*80);           //写入文件
    if(len != 2*80) debugEvent("keymap_write_fail", (int)len);
    
    lfs_file_close(lfsHandle, &keymap); 
    
    return 0;
}
