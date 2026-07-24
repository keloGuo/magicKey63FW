#ifndef MACRO_RECORDER_H
#define MACRO_RECORDER_H

#include <stdbool.h>
#include <stdint.h>

#define MACRO_RECORDER_MAX_ACTIONS 64u

typedef enum {
    MACRO_REC_IDLE = 0,
    MACRO_REC_PASS_CONFIRM,
    MACRO_REC_RECORDING,
    MACRO_REC_SAVING,
    MACRO_REC_SAVED,
    MACRO_REC_ERROR,
} macro_rec_state_t;

void macroRecorderPageEnter(void);
void macroRecorderWheelClick(void);
void macroRecorderWheelRotate(int direction);
void macroRecorderWheelLongPress(void);

unsigned char macroRecorderHandleKeyEvent(unsigned short keyValue, unsigned char pressed, unsigned char ledIndex);
unsigned char macroRecorderIsRecording(void);
unsigned char macroRecorderShouldPassThrough(void);
macro_rec_state_t macroRecorderGetState(void);
unsigned int macroRecorderGetMacroId(void);
unsigned int macroRecorderGetActionCount(void);
unsigned char macroRecorderGetPassThrough(void);
const char *macroRecorderGetErrorText(void);

#endif
