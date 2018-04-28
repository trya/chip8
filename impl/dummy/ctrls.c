#include <stdio.h>
#include <stdint.h>

#include "core/system.h"
#include "iface/ctrls.h"

// keypad controller
int refresh_keystate(struct chip8_state *st)
{
	return 0;
}

int is_key_pressed(int key, struct chip8_state *st)
{
	return (st->ks >> key) & 1;
}

int wait_keypress(struct chip8_state *st)
{
	return -1;
}

// video controller
int draw_screen(struct chip8_state *st)
{
	return 0;
}

// audio controller
int enable_sound(int timer)
{
	return 0;
}

int disable_sound(void)
{
	return 0;
}

// timer controller
int init_timers_ctrl(void)
{
	return 0;
}

int refresh_timers(struct chip8_state *st)
{
	return 0;
}
