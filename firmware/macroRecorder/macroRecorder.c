#include "macroRecorder.h"

#include <stdio.h>
#include <string.h>

#include "lfs.h"
#include "pico/stdlib.h"

#define MACRO_MAGIC 0x4d414331u
#define MACRO_VERSION 1u
#define MACRO_MAX_ID 255u
#define MACRO_FILE_PREFIX "macro_"
#define MACRO_DELAY_THRESHOLD_MS 10u
#define MACRO_DELAY_MAX_MS 60000u

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
    macro_action_t actions[MACRO_RECORDER_MAX_ACTIONS];
} macro_data_t;

lfs_t *fsInit(void);
void debugStage(unsigned char core, unsigned int stage);
void debugEvent(const char *tag, int value);
void ws2812MacroRecordStart(void);
void ws2812MacroRecordStop(void);
void ws2812MacroRecordKey(unsigned char ledIndex, unsigned char pressed);

static macro_rec_state_t recState = MACRO_REC_IDLE;
static macro_data_t recMacro;
static unsigned char passThrough = 0;
static unsigned int recMacroId = 1;
static uint32_t lastActionMs = 0;
static const char *errorText = "";

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

static unsigned int macroNextId(void)
{
    lfs_dir_t dir;
    struct lfs_info info;
    unsigned int maxId = 0;
    lfs_t *fs = fsInit();

    if(lfs_dir_open(fs, &dir, "/") >= 0)
    {
        while(lfs_dir_read(fs, &dir, &info) > 0)
        {
            if(info.type != LFS_TYPE_REG) continue;
            int id = macroIdFromFileName(info.name);
            if(id > 0 && (unsigned int)id > maxId) maxId = (unsigned int)id;
        }
        lfs_dir_close(fs, &dir);
    }

    if(maxId >= MACRO_MAX_ID) return 0;
    return maxId + 1;
}

static void macroSetDefault(macro_data_t *macro, unsigned int id)
{
    memset(macro, 0, sizeof(*macro));
    macro->header.magic = MACRO_MAGIC;
    macro->header.version = MACRO_VERSION;
    macro->header.id = id;
    macro->header.mode = 0;
}

static int macroSaveOne(macro_data_t *macro)
{
    char name[24] = {0};
    lfs_file_t file;
    lfs_t *fs = fsInit();

    macroFileName(macro->header.id, name, sizeof(name));
    macro->header.magic = MACRO_MAGIC;
    macro->header.version = MACRO_VERSION;
    if(macro->header.action_count > MACRO_RECORDER_MAX_ACTIONS)
    {
        macro->header.action_count = MACRO_RECORDER_MAX_ACTIONS;
    }

    debugStage(0, 22);
    int err = lfs_file_open(fs, &file, name, LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC);
    if(err < 0)
    {
        debugEvent("macro_open_fail", err);
        return -1;
    }
    lfs_ssize_t len = lfs_file_write(fs, &file, macro, sizeof(*macro));
    if(len != sizeof(*macro)) debugEvent("macro_write_fail", (int)len);
    lfs_file_close(fs, &file);
    return len == sizeof(*macro) ? 0 : -1;
}

static unsigned char macroHasSpace(unsigned int count)
{
    return (recMacro.header.action_count + count) <= MACRO_RECORDER_MAX_ACTIONS;
}

static unsigned char macroAppendRaw(uint8_t type, uint8_t value, uint16_t param)
{
    if(!macroHasSpace(1))
    {
        errorText = "FULL";
        recState = MACRO_REC_ERROR;
        ws2812MacroRecordStop();
        return 1;
    }

    macro_action_t *action = &recMacro.actions[recMacro.header.action_count++];
    action->type = type;
    action->value = value;
    action->param = param;
    return 0;
}

static unsigned char macroAppendDelay(uint32_t nowMs)
{
    if(recMacro.header.action_count == 0)
    {
        lastActionMs = nowMs;
        return 0;
    }

    uint32_t delay = nowMs - lastActionMs;
    while(delay >= MACRO_DELAY_THRESHOLD_MS)
    {
        uint16_t part = delay > MACRO_DELAY_MAX_MS ? MACRO_DELAY_MAX_MS : (uint16_t)delay;
        if(macroAppendRaw(4, 0, part)) return 1;
        delay -= part;
    }
    return 0;
}

static unsigned char macroAppendAction(uint8_t type, uint8_t value)
{
    uint32_t nowMs = to_ms_since_boot(get_absolute_time());
    if(macroAppendDelay(nowMs)) return 1;
    if(macroAppendRaw(type, value, 0)) return 1;
    lastActionMs = nowMs;
    return 0;
}

static unsigned char macroKeyboardUsage(unsigned short keyValue, unsigned char *usage)
{
    unsigned char value = keyValue & 0xff;
    if((keyValue >> 8) != 0) return 1;
    if(value >= 8) *usage = value - 8;
    else *usage = 0xe0 + value;
    return 0;
}

static void macroRecorderRefreshId(void)
{
    recMacroId = macroNextId();
    if(recMacroId == 0)
    {
        errorText = "ID FULL";
        recState = MACRO_REC_ERROR;
    }
}

void macroRecorderPageEnter(void)
{
    if(recState == MACRO_REC_RECORDING || recState == MACRO_REC_SAVING) return;
    errorText = "";
    passThrough = 0;
    recState = MACRO_REC_IDLE;
    macroRecorderRefreshId();
    ws2812MacroRecordStop();
}

static void macroRecorderStart(void)
{
    if(recMacroId == 0)
    {
        errorText = "ID FULL";
        recState = MACRO_REC_ERROR;
        return;
    }

    macroSetDefault(&recMacro, recMacroId);
    lastActionMs = to_ms_since_boot(get_absolute_time());
    errorText = "";
    recState = MACRO_REC_RECORDING;
    ws2812MacroRecordStart();
}

static void macroRecorderSave(void)
{
    recState = MACRO_REC_SAVING;
    ws2812MacroRecordStop();
    if(recMacro.header.action_count == 0)
    {
        errorText = "EMPTY";
        recState = MACRO_REC_ERROR;
        return;
    }

    if(macroSaveOne(&recMacro) < 0)
    {
        errorText = "SAVE";
        recState = MACRO_REC_ERROR;
        return;
    }

    recState = MACRO_REC_SAVED;
}

void macroRecorderWheelClick(void)
{
    if(recState == MACRO_REC_IDLE)
    {
        passThrough = 0;
        recState = MACRO_REC_PASS_CONFIRM;
    }
    else if(recState == MACRO_REC_PASS_CONFIRM)
    {
        macroRecorderStart();
    }
    else if(recState == MACRO_REC_RECORDING)
    {
        macroRecorderSave();
    }
    else if(recState == MACRO_REC_SAVED || recState == MACRO_REC_ERROR)
    {
        macroRecorderPageEnter();
    }
}

void macroRecorderWheelRotate(int direction)
{
    (void)direction;
    if(recState == MACRO_REC_PASS_CONFIRM)
    {
        passThrough = !passThrough;
    }
}

void macroRecorderWheelLongPress(void)
{
    if(recState == MACRO_REC_PASS_CONFIRM || recState == MACRO_REC_ERROR || recState == MACRO_REC_SAVED)
    {
        macroRecorderPageEnter();
    }
    else if(recState == MACRO_REC_RECORDING)
    {
        errorText = "CANCEL";
        recState = MACRO_REC_ERROR;
        ws2812MacroRecordStop();
    }
}

unsigned char macroRecorderHandleKeyEvent(unsigned short keyValue, unsigned char pressed, unsigned char ledIndex)
{
    if(recState != MACRO_REC_RECORDING) return 0;
    if(keyValue == 0xffff) return 0;

    unsigned char type = (keyValue >> 8) & 0xff;
    unsigned char value = keyValue & 0xff;
    unsigned char usage = 0;
    unsigned char err = 0;

    ws2812MacroRecordKey(ledIndex, pressed);

    if(type == 0)
    {
        if(macroKeyboardUsage(keyValue, &usage) == 0)
        {
            err = macroAppendAction(pressed ? 1 : 2, usage);
        }
    }
    else if(pressed && type == 2)
    {
        err = macroAppendAction(5, value);
    }
    else if(pressed && type == 1)
    {
        err = macroAppendAction(6, value);
    }

    return err;
}

unsigned char macroRecorderIsRecording(void)
{
    return recState == MACRO_REC_RECORDING;
}

unsigned char macroRecorderShouldPassThrough(void)
{
    return passThrough != 0;
}

macro_rec_state_t macroRecorderGetState(void)
{
    return recState;
}

unsigned int macroRecorderGetMacroId(void)
{
    return recMacroId;
}

unsigned int macroRecorderGetActionCount(void)
{
    return recMacro.header.action_count;
}

unsigned char macroRecorderGetPassThrough(void)
{
    return passThrough;
}

const char *macroRecorderGetErrorText(void)
{
    return errorText;
}
