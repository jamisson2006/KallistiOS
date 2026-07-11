#ifndef STUB_CONT_H
#define STUB_CONT_H
#define CONT_DPAD_UP 1
#define CONT_DPAD_DOWN 2
#define CONT_DPAD_LEFT 4
#define CONT_DPAD_RIGHT 8
#define CONT_A 16
#define CONT_B 32
#define CONT_START 64
typedef struct { int buttons; int joyx, joyy; } cont_state_t;
#endif
