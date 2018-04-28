#include <string.h>

#include "core/config.h"
#include "iface/vid.h"
#include "iface/aud.h"
#include "iface/ks.h"

/* video */
int init_video_feed(void)
{
	return 0;
}

int update_screen_buf(void *buf)
{
	memset(buf, 0, PIXBUF_SIZE);
	
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
	return 0;
}

int stop_sound(void)
{
	return 0;
}

int signal_audio_feed_end(void)
{
	return 0;
}

/* kbd */
int init_keystate_feed(void)
{
	return 0;
}

int wait_keystate_request(void)
{
	return 0;
}

int send_keystate(void *data)
{
	return 0;
}

int signal_keystate_feed_end(void)
{
	return 0;
}

