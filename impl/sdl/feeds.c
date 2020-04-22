#include <string.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "core/config.h"
#include "iface/vid.h"
#include "iface/aud.h"
#include "iface/ks.h"
#include "sdl.h"

/* video */
int init_video_feed(void)
{
	return 0;
}

int update_screen_buf(void *buf)
{
	memcpy(screen_buf, buf, PIXBUF_SIZE);
	return 0;
}

int signal_video_feed_end(void)
{
	return 0;
}

/* audio */
int init_audio_feed(void)
{
	return 0;
}

int play_sound(int timer)
{
	return play_sdl_sound(timer);
}

int stop_sound(void)
{
	return stop_sdl_sound();
}

int signal_audio_feed_end(void)
{
	return stop_sound();
}

/* kbd */
int init_keystate_feed(void)
{
	return 0;
}

int wait_keystate_request(void)
{
	SDL_Delay(10000);
	return 0;
}

int send_keystate(void *data)
{
	// set keystate of chip8 system

	return 0;
}

int signal_keystate_feed_end(void)
{
	SDL_CondSignal(keystate_cond);
	return 0;
}

