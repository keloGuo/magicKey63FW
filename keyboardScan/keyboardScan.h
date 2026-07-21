#ifndef KEYBOARD_SCAN
#define KEYBOARD_SCAN

#define KEY_BOUNCE_DIAG_INTERVAL_MAX 64u

typedef enum {
    KEY_BOUNCE_DIAG_IDLE = 0,
    KEY_BOUNCE_DIAG_SELECT,
    KEY_BOUNCE_DIAG_WAIT_RELEASE,
    KEY_BOUNCE_DIAG_WAIT_PRESS,
    KEY_BOUNCE_DIAG_CAPTURE,
    KEY_BOUNCE_DIAG_WAIT_KEY_RELEASE,
    KEY_BOUNCE_DIAG_RELEASE_CAPTURE,
    KEY_BOUNCE_DIAG_DONE,
    KEY_BOUNCE_DIAG_ERROR
} key_bounce_diag_state_t;

typedef struct {
    key_bounce_diag_state_t state;
    unsigned char row;
    unsigned char col;
    unsigned int edge_count;
    unsigned int intervals[KEY_BOUNCE_DIAG_INTERVAL_MAX];
    unsigned int release_edge_count;
    unsigned int release_intervals[KEY_BOUNCE_DIAG_INTERVAL_MAX];
    unsigned int stable_us;
    unsigned int press_total_us;
    unsigned int release_total_us;
    unsigned int error;
} key_bounce_diag_result_t;

void keyBounceDiagBegin(void);
void keyBounceDiagCancel(void);
void keyBounceDiagPoll(void);
key_bounce_diag_result_t keyBounceDiagGetResult(void);
const char *keyBounceDiagStateText(key_bounce_diag_state_t state);

#endif
