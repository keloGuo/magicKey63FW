#include "lfs.h"
#include "../magic63_version.h"

extern unsigned short  *keymapList[]; 
unsigned char keymapLoad(unsigned char init);
lfs_t * fsInit(void);
void debugStage(unsigned char core, unsigned int stage);
void debugEvent(const char *tag, int value);
void debugPrintf(const char* format, ...);
static struct saveData {
    unsigned int updateFlag;
    unsigned char activeLayer;
    unsigned char rate;
    unsigned char statePageFunction;
    unsigned char backLight;
} saveDataS = {
    MAGIC63_CONFIG_VERSION, //配置结构版本
    0x01,         //默认第一层，
    0x01,         //默认回报间隔1ms\1000hz
    0x02,         //默认第一个功能，显示当前的回报率，应为其他功能还没有做
    0x05,
};

static lfs_t *lfsHandle;          //文件系统的句柄

#define NET_CONFIG_MAGIC 0x4e455431u
#define NET_IP_MODE_LEGACY 0
#define NET_IP_MODE_PRESET_1 1
#define NET_IP_MODE_PRESET_2 2
#define NET_IP_MODE_PRESET_3 3
#define NET_IP_MODE_CUSTOM 4

static struct netSaveData {
    unsigned int magic;
    unsigned char mode;
    unsigned char customIp[4];
} netSaveDataS = {
    NET_CONFIG_MAGIC,
    NET_IP_MODE_PRESET_1,
    {10, 63, 63, 1},
};

static const unsigned char netPresetIp[][4] = {
    {192, 168, 3, 1},
    {10, 63, 27, 1},
    {172, 23, 63, 1},
    {192, 168, 231, 1},
};

static void netSaveSetDefault(void)
{
    netSaveDataS.magic = NET_CONFIG_MAGIC;
    netSaveDataS.mode = NET_IP_MODE_PRESET_1;
    netSaveDataS.customIp[0] = 10;
    netSaveDataS.customIp[1] = 63;
    netSaveDataS.customIp[2] = 63;
    netSaveDataS.customIp[3] = 1;
}

static void netSaveLimit(void)
{
    if(netSaveDataS.magic != NET_CONFIG_MAGIC) netSaveSetDefault();
    if(netSaveDataS.mode > NET_IP_MODE_CUSTOM) netSaveDataS.mode = NET_IP_MODE_PRESET_1;
    if(netSaveDataS.customIp[0] == 0 || netSaveDataS.customIp[0] == 127 || netSaveDataS.customIp[0] > 223) netSaveDataS.customIp[0] = 10;
    if(netSaveDataS.customIp[3] == 0 || netSaveDataS.customIp[3] == 255) netSaveDataS.customIp[3] = 1;
}

static int netSaveOpenConfig(lfs_file_t *file)
{
    if(lfsHandle == NULL) return -1;
    debugStage(0, 22);
    int err = lfs_file_open(lfsHandle, file, "netConfigData", LFS_O_RDWR | LFS_O_CREAT);
    if(err < 0) debugEvent("net_open_fail", err);
    return err;
}

static int netSaveWriteConfig(lfs_file_t *file)
{
    lfs_file_rewind(lfsHandle, file);
    lfs_ssize_t len = lfs_file_write(lfsHandle, file, &netSaveDataS, sizeof(netSaveDataS));
    if(len != sizeof(netSaveDataS))
    {
        debugEvent("net_write_fail", (int)len);
        return -1;
    }
    return 0;
}

static void netSaveLoad(void)
{
    lfs_file_t file;
    if(netSaveOpenConfig(&file) < 0)
    {
        netSaveSetDefault();
        return;
    }

    lfs_ssize_t readLen = lfs_file_read(lfsHandle, &file, &netSaveDataS, sizeof(netSaveDataS));
    if(readLen != sizeof(netSaveDataS) || netSaveDataS.magic != NET_CONFIG_MAGIC)
    {
        netSaveSetDefault();
        netSaveWriteConfig(&file);
    }
    netSaveLimit();
    lfs_file_close(lfsHandle, &file);
}

static void dataSaveSetDefault(void)
{
    saveDataS.updateFlag = MAGIC63_CONFIG_VERSION;
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
        netSaveLoad();
        keymapLoad(1);
        return 1;
    }
	lfs_ssize_t readLen = lfs_file_read(lfsHandle, &lfsConfigData, &saveDataS, sizeof(saveDataS));           //读文件

    debugPrintf("dataSaveInit %d\r\n", saveDataS.updateFlag);

    if(readLen != sizeof(saveDataS) || saveDataS.updateFlag != MAGIC63_CONFIG_VERSION)                                             //配置结构版本不一致就恢复默认值
    {
        dataSaveSetDefault();
        
        dataSaveWriteConfig(&lfsConfigData);           //写入文件
        netSaveLoad();
        lfs_file_close(lfsHandle, &lfsConfigData);                                          //关闭文件
        keymapLoad(1);
        return 0;
    }
    dataSaveLimit();
    netSaveLoad();
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

unsigned char dataSaveGetNetIpMode(void)
{
    netSaveLimit();
    return netSaveDataS.mode;
}

unsigned char dataSaveSetNetIpMode(unsigned char mode)
{
    lfs_file_t file;
    if(mode < NET_IP_MODE_PRESET_1 || mode > NET_IP_MODE_CUSTOM) mode = NET_IP_MODE_PRESET_1;
    netSaveDataS.mode = mode;
    netSaveLimit();
    if(netSaveOpenConfig(&file) < 0) return 1;
    netSaveWriteConfig(&file);
    lfs_file_close(lfsHandle, &file);
    return 0;
}

unsigned char dataSaveGetNetCustomIp(unsigned char *ip)
{
    if(ip == NULL) return 1;
    netSaveLimit();
    ip[0] = netSaveDataS.customIp[0];
    ip[1] = netSaveDataS.customIp[1];
    ip[2] = netSaveDataS.customIp[2];
    ip[3] = netSaveDataS.customIp[3];
    return 0;
}

unsigned char dataSaveSetNetCustomIp(const unsigned char *ip)
{
    lfs_file_t file;
    if(ip == NULL) return 1;
    netSaveDataS.customIp[0] = ip[0];
    netSaveDataS.customIp[1] = ip[1];
    netSaveDataS.customIp[2] = ip[2];
    netSaveDataS.customIp[3] = ip[3];
    netSaveDataS.mode = NET_IP_MODE_CUSTOM;
    netSaveLimit();
    if(netSaveOpenConfig(&file) < 0) return 1;
    netSaveWriteConfig(&file);
    lfs_file_close(lfsHandle, &file);
    return 0;
}

unsigned char dataSaveGetNetIp(unsigned char *ip)
{
    if(ip == NULL) return 1;
    netSaveLimit();
    if(netSaveDataS.mode == NET_IP_MODE_CUSTOM)
    {
        ip[0] = netSaveDataS.customIp[0];
        ip[1] = netSaveDataS.customIp[1];
        ip[2] = netSaveDataS.customIp[2];
        ip[3] = netSaveDataS.customIp[3];
    }
    else
    {
        unsigned char mode = netSaveDataS.mode;
        if(mode < NET_IP_MODE_PRESET_1 || mode > NET_IP_MODE_PRESET_3) mode = NET_IP_MODE_PRESET_1;
        ip[0] = netPresetIp[mode][0];
        ip[1] = netPresetIp[mode][1];
        ip[2] = netPresetIp[mode][2];
        ip[3] = netPresetIp[mode][3];
    }
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
