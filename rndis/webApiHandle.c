
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "net.h"
#include "cJSON.h"
#include "lfs.h"
#include "keyboardScan/keyboardScan.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

static const char *s_json_header ="Access-Control-Allow-Origin:http://127.0.0.1\r\nAccess-Control-Allow-Headers:*\r\nContent-Type: application/json\r\n""Cache-Control: no-cache\r\n";

#define MACRO_MAGIC 0x4d414331u
#define MACRO_VERSION 1u
#define MACRO_MAX_ID 255u
#define MACRO_MAX_ACTIONS 64u
#define MACRO_FILE_PREFIX "macro_"

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint16_t version;
    uint16_t action_count;
    uint16_t id;
    uint8_t mode;
    uint8_t reserved;
    uint32_t crc32;
} macro_entry_t;

typedef struct __attribute__((packed)) {
    uint8_t type;
    uint8_t value;
    uint16_t param;
} macro_action_t;

typedef struct __attribute__((packed)) {
    macro_entry_t header;
    macro_action_t actions[MACRO_MAX_ACTIONS];
} macro_data_t;

unsigned char getFlashLayerInfo(void);
unsigned short *getKeyMap(unsigned char layer);
unsigned char layerNumberSet(signed char layer);
unsigned char webFileUpdatePpakgStart(struct mg_http_message *hm,struct mg_connection *c);
lfs_t * fsInit(void);
void debugStage(unsigned char core, unsigned int stage);
void debugEvent(const char *tag, int value);
void keyboardReportSetPaused(unsigned char paused);
void magic63UIBounceDiagPageEnter(void);
void magic63UIBounceDiagPageLeave(void);

static void macroFileName(unsigned int id, char *out, size_t outLen)
{
    snprintf(out, outLen, MACRO_FILE_PREFIX "%u", id);
}

static int macroIdFromFileName(const char *name)
{
    size_t prefixLen = strlen(MACRO_FILE_PREFIX);
    if(strncmp(name, MACRO_FILE_PREFIX, prefixLen) != 0) return -1;
    int id = 0;
    for(const char *p = name + prefixLen; *p != '\0'; p++)
    {
        if(*p < '0' || *p > '9') return -1;
        id = id * 10 + (*p - '0');
        if(id > MACRO_MAX_ID) return -1;
    }
    return id > 0 ? id : -1;
}

static void macroSetDefault(macro_data_t *macro, unsigned int id)
{
    memset(macro, 0, sizeof(*macro));
    macro->header.magic = MACRO_MAGIC;
    macro->header.version = MACRO_VERSION;
    macro->header.id = id;
}

static int macroLoadOne(unsigned int id, macro_data_t *macro, unsigned char createDefault)
{
    char name[24] = {0};
    lfs_file_t file;
    lfs_t *fs = fsInit();
    macroFileName(id, name, sizeof(name));
    debugStage(0, 22);
    int err = lfs_file_open(fs, &file, name, LFS_O_RDONLY);
    if(err < 0)
    {
        macroSetDefault(macro, id);
        return createDefault ? 0 : -1;
    }

    lfs_ssize_t len = lfs_file_read(fs, &file, macro, sizeof(*macro));
    lfs_file_close(fs, &file);
    if(len != sizeof(*macro)) debugEvent("web_macro_read_fail", (int)len);
    if(len != sizeof(*macro) ||
       macro->header.magic != MACRO_MAGIC ||
       macro->header.version != MACRO_VERSION ||
       macro->header.id != id ||
       macro->header.action_count > MACRO_MAX_ACTIONS)
    {
        macroSetDefault(macro, id);
        return -1;
    }
    return 0;
}

static int macroSaveOne(unsigned int id, macro_data_t *macro)
{
    char name[24] = {0};
    lfs_file_t file;
    lfs_t *fs = fsInit();
    macroFileName(id, name, sizeof(name));
    macro->header.magic = MACRO_MAGIC;
    macro->header.version = MACRO_VERSION;
    macro->header.id = id;
    if(macro->header.action_count > MACRO_MAX_ACTIONS) macro->header.action_count = MACRO_MAX_ACTIONS;
    debugStage(0, 22);
    int err = lfs_file_open(fs, &file, name, LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC);
    if(err < 0)
    {
        debugEvent("web_macro_open_fail", err);
        return -1;
    }
    lfs_ssize_t len = lfs_file_write(fs, &file, macro, sizeof(*macro));
    lfs_file_close(fs, &file);
    if(len != sizeof(*macro)) debugEvent("web_macro_write_fail", (int)len);
    return len == sizeof(*macro) ? 0 : -1;
}

static void macroAddJson(cJSON *array, macro_data_t *macro)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *actions = cJSON_CreateArray();
    cJSON_AddNumberToObject(root, "id", macro->header.id);
    cJSON_AddNumberToObject(root, "mode", macro->header.mode);
    cJSON_AddNumberToObject(root, "actionCount", macro->header.action_count);

    for(unsigned int i = 0; i < macro->header.action_count; i++)
    {
        macro_action_t *action = &macro->actions[i];
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "type", action->type);
        cJSON_AddNumberToObject(item, "value", action->value);
        cJSON_AddNumberToObject(item, "param", action->param);
        cJSON_AddItemToArray(actions, item);
    }
    cJSON_AddItemToObject(root, "actions", actions);
    cJSON_AddItemToArray(array, root);
}

static char *macroListJson(void)
{
    lfs_dir_t dir;
    struct lfs_info info;
    macro_data_t macro;
    cJSON *root = cJSON_CreateObject();
    cJSON *macros = cJSON_CreateArray();
    lfs_t *fs = fsInit();

    if(lfs_dir_open(fs, &dir, "/") >= 0)
    {
        while(lfs_dir_read(fs, &dir, &info) > 0)
        {
            if(info.type != LFS_TYPE_REG) continue;
            int id = macroIdFromFileName(info.name);
            if(id < 0) continue;
            if(macroLoadOne((unsigned int)id, &macro, 0) == 0) macroAddJson(macros, &macro);
        }
        lfs_dir_close(fs, &dir);
    }

    cJSON_AddNumberToObject(root, "maxActions", MACRO_MAX_ACTIONS);
    cJSON_AddItemToObject(root, "macros", macros);
    char *out = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return out;
}

static char *macroOneJson(macro_data_t *macro)
{
    cJSON *array = cJSON_CreateArray();
    macroAddJson(array, macro);
    cJSON *item = cJSON_DetachItemFromArray(array, 0);
    cJSON_Delete(array);
    cJSON_AddNumberToObject(item, "maxActions", MACRO_MAX_ACTIONS);
    char *out = cJSON_PrintUnformatted(item);
    cJSON_Delete(item);
    return out;
}

static int jsonReadInt(cJSON *item, int min, int max, int fallback)
{
    if(item == NULL || cJSON_IsNumber(item) == 0) return fallback;
    if(item->valueint < min) return min;
    if(item->valueint > max) return max;
    return item->valueint;
}

static unsigned int jsonMacroId(cJSON *json)
{
    return (unsigned int)jsonReadInt(cJSON_GetObjectItem(json, "id"), 1, MACRO_MAX_ID, 1);
}

unsigned char webReturnOk(struct mg_connection *c)
{
    mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
    return 0;
}

int goTOBootModeX(void);
static int64_t rebootToUf2Handle(alarm_id_t id, void *user_data)
{
    (void)id;
    (void)user_data;
    goTOBootModeX();
    return 0;
}

unsigned char webRebootToUf2(struct mg_connection *c)
{
    mg_http_reply(c, 200, s_json_header, "{\"state\":\"bootloader\"}");
    add_alarm_in_ms(200, rebootToUf2Handle, NULL, false);
    return 0;
}

static int64_t rebootHandle(alarm_id_t id, void *user_data)
{
    (void)id;
    (void)user_data;
    watchdog_reboot(0, 0, 0);
    return 0;
}

unsigned char webReboot(struct mg_connection *c)
{
    mg_http_reply(c, 200, s_json_header, "{\"state\":\"reboot\"}");
    add_alarm_in_ms(200, rebootHandle, NULL, false);
    return 0;
}

unsigned char webFsInfo(struct mg_connection *c)
{
    int fsUsedBytes(void);
    unsigned int fsTotalBytes(void);
    unsigned int fsBlockSize(void);
    unsigned int fsBlockCount(void);

    int used = fsUsedBytes();
    unsigned int total = fsTotalBytes();
    unsigned int freeBytes = (used >= 0 && (unsigned int)used <= total) ? total - (unsigned int)used : 0;
    unsigned int usedBlocks = (used >= 0 && fsBlockSize() != 0) ? ((unsigned int)used / fsBlockSize()) : 0;

    char body[160];
    snprintf(body, sizeof(body),
             "{\"fsTotal\":%u,\"fsUsed\":%u,\"fsFree\":%u,\"fsBlockSize\":%u,\"fsBlockCount\":%u,\"fsUsedBlocks\":%u}",
             total,
             used >= 0 ? (unsigned int)used : 0,
             freeBytes,
             fsBlockSize(),
             fsBlockCount(),
             usedBlocks);
    mg_http_reply(c, 200, s_json_header, body);
    return used >= 0 ? 1 : 0;
}

static unsigned char webBounceDiagRunning = 0;
static unsigned int webBounceDiagLastHeartbeatMs = 0;
#define WEB_BOUNCE_DIAG_HEARTBEAT_TIMEOUT_MS 3000u

static void webBounceDiagStop(void)
{
    keyBounceDiagCancel();
    magic63UIBounceDiagPageLeave();
    keyboardReportSetPaused(0);
    webBounceDiagRunning = 0;
}

unsigned char webBounceDiagStart(struct mg_connection *c)
{
    keyboardReportSetPaused(1);
    keyBounceDiagBegin();
    magic63UIBounceDiagPageEnter();
    webBounceDiagRunning = 1;
    webBounceDiagLastHeartbeatMs = to_ms_since_boot(get_absolute_time());
    mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
    return 1;
}

unsigned char webBounceDiagCancel(struct mg_connection *c)
{
    webBounceDiagStop();
    mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
    return 1;
}

void webBounceDiagPollTimeout(void)
{
    if(!webBounceDiagRunning) return;
    unsigned int now = to_ms_since_boot(get_absolute_time());
    if((unsigned int)(now - webBounceDiagLastHeartbeatMs) >= WEB_BOUNCE_DIAG_HEARTBEAT_TIMEOUT_MS)
    {
        debugEvent("bounce_web_timeout", (int)(now - webBounceDiagLastHeartbeatMs));
        webBounceDiagStop();
    }
}

unsigned char webBounceDiagStatus(struct mg_connection *c)
{
    webBounceDiagLastHeartbeatMs = to_ms_since_boot(get_absolute_time());
    key_bounce_diag_result_t r = keyBounceDiagGetResult();
    if(webBounceDiagRunning &&
       (r.state == KEY_BOUNCE_DIAG_ERROR || r.state == KEY_BOUNCE_DIAG_IDLE))
    {
        webBounceDiagStop();
    }

    unsigned int count = r.edge_count;
    if(count > KEY_BOUNCE_DIAG_INTERVAL_MAX) count = KEY_BOUNCE_DIAG_INTERVAL_MAX;
    unsigned int releaseCount = r.release_edge_count;
    if(releaseCount > KEY_BOUNCE_DIAG_INTERVAL_MAX) releaseCount = KEY_BOUNCE_DIAG_INTERVAL_MAX;

    static char body[2048];
    size_t used = 0;
    used += snprintf(body + used, sizeof(body) - used,
                     "{\"state\":%u,\"stateText\":\"%s\",\"running\":%u,\"row\":%u,\"col\":%u,\"edgeCount\":%u,\"releaseEdgeCount\":%u,\"stableUs\":%u,\"pressTotalUs\":%u,\"releaseTotalUs\":%u,\"error\":%u,\"intervals\":[",
                     (unsigned int)r.state,
                     keyBounceDiagStateText(r.state),
                     (unsigned int)webBounceDiagRunning,
                     (unsigned int)r.row,
                     (unsigned int)r.col,
                     r.edge_count,
                     r.release_edge_count,
                     r.stable_us,
                     r.press_total_us,
                     r.release_total_us,
                     r.error);
    for(unsigned int i = 0; i < count && used < sizeof(body); i++)
    {
        used += snprintf(body + used, sizeof(body) - used, "%s%u", i == 0 ? "" : ",", r.intervals[i]);
    }
    if(used < sizeof(body)) used += snprintf(body + used, sizeof(body) - used, "],\"releaseIntervals\":[");
    for(unsigned int i = 0; i < releaseCount && used < sizeof(body); i++)
    {
        used += snprintf(body + used, sizeof(body) - used, "%s%u", i == 0 ? "" : ",", r.release_intervals[i]);
    }
    if(used < sizeof(body))
    {
        snprintf(body + used, sizeof(body) - used, "]}");
    }
    else
    {
        body[sizeof(body) - 1] = '\0';
    }

    mg_http_reply(c, 200, s_json_header, body);
    return 1;
}

unsigned char webGetMacro(struct mg_http_message *hm,struct mg_connection *c)
{
    const char *p = hm->body.ptr;
	cJSON *tempJson = cJSON_Parse((const char *)(p));
    macro_data_t macro;
    unsigned int macroId = 1;

    if(tempJson != NULL)
    {
        macroId = jsonMacroId(tempJson);
    }

    macroLoadOne(macroId, &macro, 1);
    char *out = macroOneJson(&macro);
    if(tempJson != NULL) cJSON_Delete(tempJson);

    if(out == NULL)
    {
        mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
        return 0;
    }

	mg_http_reply(c, 200, s_json_header, out);
	free(out);
	return 1;
}

unsigned char webListMacro(struct mg_connection *c)
{
    char *out = macroListJson();
    if(out == NULL)
    {
        mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
        return 0;
    }
	mg_http_reply(c, 200, s_json_header, out);
	free(out);
	return 1;
}

unsigned char webSetMacro(struct mg_http_message *hm,struct mg_connection *c)
{
    const char *p = hm->body.ptr;
	cJSON *tempJson = cJSON_Parse((const char *)(p));

	if(tempJson == NULL)
    {
        mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
        return 0;
    }

	cJSON *actionsJson = cJSON_GetObjectItem(tempJson, "actions");
    if(actionsJson == NULL || cJSON_IsArray(actionsJson) == 0)
    {
        cJSON_Delete(tempJson);
        mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
        return 0;
    }

    unsigned int macroId = jsonMacroId(tempJson);
    int mode = jsonReadInt(cJSON_GetObjectItem(tempJson, "mode"), 0, 255, 0);
    int actionCount = cJSON_GetArraySize(actionsJson);
    if(actionCount > MACRO_MAX_ACTIONS) actionCount = MACRO_MAX_ACTIONS;

    macro_data_t macro;
    macroSetDefault(&macro, macroId);
    macro.header.mode = mode;
    macro.header.action_count = actionCount;

    for(int i = 0; i < actionCount; i++)
    {
        cJSON *item = cJSON_GetArrayItem(actionsJson, i);
        if(item == NULL) continue;
        macro.actions[i].type = jsonReadInt(cJSON_GetObjectItem(item, "type"), 0, 255, 0);
        macro.actions[i].value = jsonReadInt(cJSON_GetObjectItem(item, "value"), 0, 255, 0);
        macro.actions[i].param = jsonReadInt(cJSON_GetObjectItem(item, "param"), 0, 65535, 0);
    }

    cJSON_Delete(tempJson);
    if(macroSaveOne(macroId, &macro) < 0)
    {
        mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
        return 0;
    }

	mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
	return 1;
}

unsigned char webDeleteMacro(struct mg_http_message *hm,struct mg_connection *c)
{
    const char *p = hm->body.ptr;
	cJSON *tempJson = cJSON_Parse((const char *)(p));
    if(tempJson == NULL)
    {
        mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
        return 0;
    }

    unsigned int macroId = jsonMacroId(tempJson);
    char name[24] = {0};
    macroFileName(macroId, name, sizeof(name));
    lfs_remove(fsInit(), name);
    cJSON_Delete(tempJson);
	mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
	return 1;
}

static int codexStatusCode(const char *state)
{
    if(strcmp(state, "idle") == 0) return 0;
    if(strcmp(state, "notLoaded") == 0) return 0;
    if(strcmp(state, "working") == 0) return 1;
    if(strcmp(state, "active") == 0) return 1;
    if(strcmp(state, "success") == 0) return 2;
    if(strcmp(state, "error") == 0) return 3;
    if(strcmp(state, "systemError") == 0) return 3;
    if(strcmp(state, "waiting") == 0) return 4;
    return -1;
}

unsigned char webCodexStatus(struct mg_http_message *hm,struct mg_connection *c)
{
    const char *p = hm->body.ptr;
	cJSON *tempJson = cJSON_Parse((const char *)(p));

	if(tempJson == NULL)
    {
        mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
        return 0;
    }

	cJSON *tempJsonState = cJSON_GetObjectItem(tempJson,"state");
	if(tempJsonState == NULL || cJSON_IsString(tempJsonState) == 0)
	{
		cJSON_Delete(tempJson);
        mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
		return 0;
	}

    int status = codexStatusCode(tempJsonState->valuestring);
    if(status < 0)
    {
		cJSON_Delete(tempJson);
        mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
		return 0;
    }

    unsigned char ws2812CodexStatus(unsigned char status);
    ws2812CodexStatus((unsigned char)status);
	cJSON_Delete(tempJson);
	mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
	return 1;
}

//获取指定层的keymap数据，并创建一个json 对象，添加一个key ，数组；
char *keyMapJson(unsigned char layer)
{
  unsigned short *p = getKeyMap(layer);
  static char out[640];
  size_t used = 0;

  used += snprintf(out + used, sizeof(out) - used, "{\"keyMap\":[");
  for(int i = 0;i < 80 && used < sizeof(out);i++)
  {
    unsigned int value = p[i] & 0xffffu;
    used += snprintf(out + used, sizeof(out) - used, "%s%u", i == 0 ? "" : ",", value);
  }
  if(used < sizeof(out))
  {
    snprintf(out + used, sizeof(out) - used, "],\"layer\":%u}", layer);
  }
  else
  {
    out[sizeof(out) - 1] = '\0';
  }

  return out;
}
/*web 端获取 激活层的数据 ，json layer = 0xff，返回当前激活的层，和keymap数据，
    json 参数 {"layer"：n}
    如果n 大于 0 且，小于5，则设置激活层为n，同时返回第n层数据
    否则返回当前激活的层，和层数据
*/
unsigned char getKeyboardMap(struct mg_http_message *hm,struct mg_connection *c)
{
	const char *p = hm->body.ptr;

	cJSON *tempJson = cJSON_Parse((const char *)(p));
	if(tempJson == NULL) 
    {
        webReturnOk(c);
        return 0;
    }
	cJSON *tempJsonlayer = cJSON_GetObjectItem(tempJson,"layer");
	if(tempJsonlayer == NULL) 
	{
		cJSON_Delete(tempJson);
		return 0;
	}
    unsigned char tempLayer = ((tempJsonlayer->valueint > 0) && (tempJsonlayer->valueint < 5)) ? tempJsonlayer->valueint : getFlashLayerInfo();	
    printf("getKeyboardMap req=%d active=%d\r\n", tempJsonlayer->valueint, tempLayer);

    if(tempLayer == tempJsonlayer->valueint)  layerNumberSet(tempJsonlayer->valueint);

	char *q = keyMapJson(tempLayer);
	mg_http_reply(c, 200, s_json_header, q);
    cJSON_Delete(tempJson);
	
	return 0;
}


unsigned char setKeyValue(struct mg_http_message *hm,struct mg_connection *c)
{ 
    const char *p = hm->body.ptr;
	cJSON *tempJson = cJSON_Parse((const char *)(p));
	printf("webDataProcess = %s \r\n",p);

	if(tempJson == NULL) 
    {
        webReturnOk(c);
        return 0;
    }
	cJSON *tempJsonid = cJSON_GetObjectItem(tempJson,"id");
	cJSON *tempJsonKeyValue = cJSON_GetObjectItem(tempJson,"value");

	if(tempJsonid == NULL || tempJsonKeyValue ==NULL) 
	{
		cJSON_Delete(tempJson);
        webReturnOk(c);
		return 0;
	}

	printf("getFlashLayerInfo() = %d \r\n",getFlashLayerInfo());

	unsigned short *q = getKeyMap(getFlashLayerInfo());
	q[tempJsonid->valueint - 1] = tempJsonKeyValue->valueint;
    unsigned char keymapSave(unsigned char layer);
    keymapSave(getFlashLayerInfo());
    cJSON_Delete(tempJson);
	mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
	return 1;
}

//默认flash里面是没有网页数据，通过这个接口上传页面文件，只能一个html文件，上传前需要打包数据
unsigned char fsWriteHtmlFile(unsigned char *p,unsigned int len);
unsigned char fsEraseHtmlFile(void);

unsigned char webFileUpdate(struct mg_http_message *hm,struct mg_connection *c)
{ 
	const char *p = hm->body.ptr;
	//printf("webFileUpdate = %s \r\n",p);
	fsWriteHtmlFile((unsigned char *)p,hm->body.len);
	mg_http_reply(c, 200, s_json_header, "{\"success\":\"ok\"}");
	return 1;
}

//删除文件，重新上传，如果没有重新上传，再次上电之前的数据还在
unsigned char webFileDelete(struct mg_connection *c)
{ 
	fsEraseHtmlFile();
	mg_http_reply(c, 200, s_json_header, "{\"reload\":\"ok\"}");
	return 1;
}

//升级版，避免内存不够用，每次传输1k数据
static unsigned char webFileUpdatePbuff[1025] = {0};
static unsigned char fileType = 0;  //1 网页文件，2GIF文件
#define GIF_UPLOAD_MAX_SIZE (512 * 1024)

char *dataEndScan(char *p,unsigned int len)
{
    char *dataEnd = NULL;
    char *pTemp = p; 
    for(int i = 0;;i++)
    {
        dataEnd = strstr(pTemp,"------WebKitFormBoundary");
        if(dataEnd ==NULL)
        {
            pTemp += strlen(pTemp) ;
            pTemp += 1;
            if(pTemp > (p+len))
            {
                return NULL;
            }
        }
        else
        {
            return dataEnd;
        }
    }

}

unsigned char webFileUpdateP(struct mg_http_message *hm,struct mg_connection *c)
{
    unsigned int lenT = 0;
	const char *p = hm->body.ptr;

    char *dataStart = strstr(p,"Content-Type: application/octet-stream");

    //printf("webFileUpdateP %s \n", p);

    if(dataStart != NULL)
    {
        dataStart += strlen("Content-Type: application/octet-stream");
        dataStart += 4;
        char *dataEnd = dataEndScan(dataStart,hm->body.len-strlen("Content-Type: application/octet-stream") - 4); //&& dataEnd != NULL strstr(dataStart,"------WebKitFormBoundary")

        if(dataEnd != NULL)
        {
            dataEnd -= 2;
            lenT = dataEnd - dataStart;    

            if(lenT <= 1024)
            {
                memcpy(webFileUpdatePbuff,dataStart,lenT); 
                webFileUpdatePbuff[lenT] = '\0';
                // printf("webFileUpdateP = %d \n%s \r\n",lenT,webFileUpdatePbuff); //打印带尾巴
                mg_http_reply(c, 200, s_json_header, "{\"success\":\"ok\"}");
                return 0;
            }
        }

    }

    mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
    return 0;
}
unsigned char fsWriteHtmlFilePakg(unsigned int offset,unsigned char *p,unsigned int len,unsigned char isOver);
unsigned char fsWriteGifFilePakg(unsigned int offset,unsigned char *p,unsigned int len,unsigned char isOver);
unsigned char webFileUpdatePpakgEnter(struct mg_http_message *hm,struct mg_connection *c)
{
    const char *p = hm->body.ptr;
	cJSON *tempJson = cJSON_Parse((const char *)(p));
	//printf("webFileUpdatePpakgEnter = %s \r\n",p);

	if(tempJson == NULL) 
    {
        mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
        return 0;
    }

	cJSON *tempJsonLen = cJSON_GetObjectItem(tempJson,"len");
	cJSON *tempJsoncheckSum = cJSON_GetObjectItem(tempJson,"cleckSum");
    cJSON *tempJsonOffset = cJSON_GetObjectItem(tempJson,"offset");
    cJSON *tempJsonOver = cJSON_GetObjectItem(tempJson,"over");

	if(tempJsonLen == NULL || tempJsoncheckSum ==NULL|| tempJsonOffset ==NULL|| tempJsonOver ==NULL) 
	{
		cJSON_Delete(tempJson);
        mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
		return 0;
	}

    if(tempJsonLen->valueint < 0 || tempJsonLen->valueint > 1024)
    {
        cJSON_Delete(tempJson);
        mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
        return 0;
    }

    unsigned int checkSumTemp = 0;
    for(int i = 0;i<tempJsonLen->valueint;i++)
    {
        checkSumTemp += webFileUpdatePbuff[i];
    }
    
    printf("len = %d checkSumTemp = %d  tempJsoncheckSum = %d over = %d len = %d\r\n",tempJsonLen->valueint,checkSumTemp,tempJsoncheckSum->valueint,tempJsonOver->valueint,tempJsonLen->valueint);

    if(checkSumTemp == tempJsoncheckSum->valueint)
    {
        unsigned char writeResult = 1;
        if(fileType == 1) writeResult = fsWriteHtmlFilePakg(tempJsonOffset->valueint,webFileUpdatePbuff,tempJsonLen->valueint,tempJsonOver->valueint);
        else if(fileType == 2) writeResult = fsWriteGifFilePakg(tempJsonOffset->valueint,webFileUpdatePbuff,tempJsonLen->valueint,tempJsonOver->valueint);
        if(tempJsonOver->valueint) fileType = 0;
        cJSON_Delete(tempJson);
        if(writeResult == 0)
        {
            mg_http_reply(c, 200, s_json_header, "{\"state\":\"ok\"}");
            return 0;
        }
        mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
        return 0;
    }

    printf("webFileUpdatePpakgEnter check erro\n");
    cJSON_Delete(tempJson);
    mg_http_reply(c, 200, s_json_header, "{\"state\":\"fail\"}");
    return 0;
}
unsigned char webFileUpdatePpakgStart(struct mg_http_message *hm,struct mg_connection *c)
{
    const char *p = hm->body.ptr;
	cJSON *tempJson = cJSON_Parse((const char *)(p));
	printf("webFileUpdatePpakgStart = %s \r\n",p);

	if(tempJson == NULL) 
    {
        mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
        return 0;
    }

	cJSON *tempJsonSize = cJSON_GetObjectItem(tempJson,"size");
	cJSON *tempJsonType = cJSON_GetObjectItem(tempJson,"type");
    if(tempJsonSize == NULL || tempJsonType == NULL)
    {
        cJSON_Delete(tempJson);
        mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
        return 0;
    }

    if(tempJsonType->valueint == 2 && tempJsonSize->valueint > GIF_UPLOAD_MAX_SIZE)
    {
        cJSON_Delete(tempJson);
        fileType = 0;
        mg_http_reply(c, 200, s_json_header, "{\"success\":\"fail\"}");
        return 0;
    }

    fileType = tempJsonType->valueint;

    printf("len = %d tempJsonSize = %d  tempJsonType = %d\r\n",tempJsonSize->valueint,tempJsonType->valueint);
    cJSON_Delete(tempJson);
    mg_http_reply(c, 200, s_json_header, "{\"success\":\"ok\"}");
    return 0; 
}
