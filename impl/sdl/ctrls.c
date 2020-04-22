#include <stdint.h>
#include <errno.h>

#include <sys/timerfd.h>
#include <unistd.h>
#include <fcntl.h>

#include <SDL2/SDL.h>

#include "core/system.h"
#include "core/config.h"
#include "core/emu.h"
#include "iface/ctrls.h"
#include "iface/vid.h"
#include "iface/aud.h"
#include "iface/ks.h"
#include "sdl.h"

// keypad controller
int refresh_keystate(struct chip8_state *st)
{
	// TODO: atomicity
	st->ks = keystate;
	return 0;
}

int is_key_pressed(int key, struct chip8_state *st)
{
	return (st->ks >> key) & 1;
}

int wait_keypress(struct chip8_state *st)
{
	// TODO: atomic access of run
	int key;

	/* look for a pressed key */
	SDL_LockMutex(keystate_mutex);
	do {
		refresh_keystate(st);
		for (key = 0; key <= 0xF; key++) {
			if (is_key_pressed(key, st)) {
				break;
			}
		}
		if (key > 0xF) {
			// no key pressed, wait for a change of keystate
			SDL_CondWait(keystate_cond, keystate_mutex);
		}
	} while (key > 0xF && run);
	SDL_UnlockMutex(keystate_mutex);

	/* wait for the pressed key to be released
	 * (as specified in the interpreter for the COSMAS,
	 * cf. http://laurencescotford.co.uk/?p=347) */
	int released = 0;
	SDL_LockMutex(keystate_mutex);
	while (!released && run) {
		refresh_keystate(st);
		if (is_key_pressed(key, st)) {
			// key is still pressed, wait for a change of keystate
			SDL_CondWait(keystate_cond, keystate_mutex);
		} else {
			released = 1;
		}
	}
	SDL_UnlockMutex(keystate_mutex);

	return key;
}

// video controller
int draw_screen(struct chip8_state *st)
{
	SDL_LockMutex(refresh_mutex);
	update_screen_buf(st->screen_buf);
	blit_screen_surface(NULL);
	refresh_window(NULL);
	SDL_UnlockMutex(refresh_mutex);

	return 0;
}

// audio controller
int enable_sound(int timer)
{
	return play_sound(timer);
}

int disable_sound(void)
{
	return stop_sound();
}

// timer controller
int timer_fd = -1;
int init_timers_ctrl(void)
{
	struct itimerspec timer;
	// 60 ticks per second
	timer.it_interval.tv_sec = 0;
	timer.it_interval.tv_nsec = 16666667; // round(1000*1000*1000/60)
	// activation after 1/60th of a second (double 0 would disarm the timer)
	timer.it_value.tv_sec = 0;
	timer.it_value.tv_nsec = 16666667;

	if ((timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)) < 0) {
		perror("timerfd_create");
		return -1;
	}

	if (timerfd_settime(timer_fd, 0, &timer, NULL) < 0) {
		perror("timerfd_settime");
		return -1;
	}

	return 0;
}

static void refresh_timer(uint8_t *timer, uint64_t ticks)
{
	if (ticks < *timer) {
		*timer -= ticks;
	} else {
		*timer = 0;
	}
}

int refresh_timers(struct chip8_state *st)
{
	uint64_t ticks;

	if (read(timer_fd, &ticks, sizeof(ticks)) < 0) {
		if (errno == EAGAIN) {
			return 0;
		} else {
			perror("read");
			return -1;
		}
	}

	refresh_timer(&st->d_timer, ticks);
	refresh_timer(&st->s_timer, ticks);

	return 0;
}
