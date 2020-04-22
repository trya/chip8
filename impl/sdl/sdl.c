#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <SDL2/SDL.h>

#include "core/config.h"
#include "core/emu.h"
#include "iface/vid.h"
#include "iface/aud.h"
#include "iface/ks.h"

#define PIXEL_FORMAT ((DEPTH_BITS == 1) ? SDL_PIXELFORMAT_INDEX1LSB : SDL_PIXELFORMAT_INDEX8)

uint8_t *screen_buf;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Surface *surface;
SDL_Surface *screen;
SDL_Window *window;
SDL_Event quit_event;
SDL_mutex *refresh_mutex, *run_mutex, *keystate_mutex;
SDL_cond *keystate_cond;
int need_conv = 0;
SDL_Thread *disp_th, *event_th, *keystate_th;
SDL_AudioDeviceID devid;

struct chip8_sdl_key {
	int keynum;
	SDL_Scancode scancode;
};
static struct chip8_sdl_key keymap[16] = {
	{1, SDL_SCANCODE_1}, {2, SDL_SCANCODE_2}, {3, SDL_SCANCODE_3}, {0xC, SDL_SCANCODE_4},
	{4, SDL_SCANCODE_Q}, {5, SDL_SCANCODE_W}, {6, SDL_SCANCODE_E}, {0xD, SDL_SCANCODE_R},
	{7, SDL_SCANCODE_A}, {8, SDL_SCANCODE_S}, {9, SDL_SCANCODE_D}, {0xE, SDL_SCANCODE_F},
	{0xA, SDL_SCANCODE_Z}, {0, SDL_SCANCODE_X}, {0xB, SDL_SCANCODE_C}, {0xF, SDL_SCANCODE_V}
}; // TODO: move in config.h
uint16_t keystate;

/* sound */
static int sampling_freq = 44100;
static int wave_freq = 220;
static float wave_ampl = 0.2;
static int16_t sample_max = INT16_MAX;
static int16_t sample_min = INT16_MIN;
static int sndbuf_sz;
static size_t samples_count = 0;
static int wave = 1; // 0 for low, 1 for high
static uint8_t *sndbuf;

/* private functions */
static int atomic_exec(int (*func)(void *), void *data, SDL_mutex *mut);
static int set_run(void *data);
static int events_loop(void *data);
static int or_keystate(void *data);
static int and_keystate(void *data);
static int fill_sound_buffer(uint8_t *buf, size_t sz, int wave_freq);

int atomic_exec(int (*func)(void *), void *data, SDL_mutex *mut)
{
	int res;
	
	if (SDL_LockMutex(mut) < 0) {
		printf("SDL_LockMutex: %s\n", SDL_GetError());
		return -1;
	} else {
		res = func(data);
		if (SDL_UnlockMutex(mut) < 0) {
			printf("SDL_UnlockMutex: %s\n", SDL_GetError());
			return -1;
		}
	}
	
	return res;
}

int set_run(void *data)
{
	memcpy(&run, data, sizeof(run));
	if (!run) {
		signal_video_feed_end();
		signal_keystate_feed_end();
		signal_audio_feed_end();
	}
	return 0;
}

int blit_screen_surface(void *data)
{
	if (need_conv) {
		SDL_LowerBlit(surface, &surface->clip_rect, screen, &surface->clip_rect);
		/* LowerBlit instead of BlitSurface because verification
		 * has already been done by ConvertSurface */
	}
	
	return 0;
}

int refresh_window(void *data)
{
	SDL_UpdateTexture(texture, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
	
	return 0;
}

int events_loop(void *data)
{
	SDL_Event event;
	uint16_t mask;
	
	while (1) {
		if (!SDL_WaitEvent(&event)) {
			printf("SDL_WaitEvent: %s\n", SDL_GetError());
			return -1;
		}
		switch (event.type) {
		case SDL_QUIT:
#ifdef DEBUG
			printf("Quit!\n");
#endif
			atomic_exec(set_run, &(int){0}, run_mutex);
			return 0;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				atomic_exec(set_run, &(int){0}, run_mutex);
				return 0;
			case SDLK_TAB:
				reset = 1; // TODO: atomicity
				break;
			}
			
			for (int i = 0; i < 16; i++) {
				if (event.key.keysym.scancode == keymap[i].scancode) {
					mask = 1 << keymap[i].keynum;
					atomic_exec(or_keystate, &mask, keystate_mutex);
					break;
				}
			}
			break;
		case SDL_KEYUP:
			for (int i = 0; i < 16; i++) {
				if (event.key.keysym.scancode == keymap[i].scancode) {
					mask = 1 << keymap[i].keynum;
					mask = ~mask;
					atomic_exec(and_keystate, &mask, keystate_mutex);
					break;
				}
			}
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_EXPOSED) {
#ifdef DEBUG
				printf("Window exposed!\n");
#endif
				atomic_exec(refresh_window, NULL, refresh_mutex);
			}
			break;
		}
	}
}

int init_sdl(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		printf("SDL_Init: %s\n", SDL_GetError());
		return -1;
	}
	
	if (atexit(SDL_Quit) < 0) {
		printf("Failed to set exit function\n");
		return -1;
	}
	
	window = SDL_CreateWindow("CHIP-8 SDL Display",
	                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	                          640, 320, SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		printf("SDL_CreateWindow: %s\n", SDL_GetError());
		return -1;
	}
	
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		printf("SDL_CreateRenderer: %s\n", SDL_GetError());
		return -1;
	}
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	
	SDL_Delay(1000);
	
	SDL_Color colors[] = {
		{.r = 0, .g = 0, .b = 0, .a = 255},
		{.r = 255, .g = 255, .b = 255, .a = 255}
	};
	
	surface = SDL_CreateRGBSurfaceWithFormat(0, RES_X, RES_Y, DEPTH_BITS, PIXEL_FORMAT);
	if (surface == NULL) {
		printf("SDL_CreateRGBSurfaceWithFormatFrom: %s\n", SDL_GetError());
		return -1;
	}
	screen_buf = surface->pixels;
	
	if (SDL_SetPaletteColors(surface->format->palette, colors, 0, 2) < 0) {
		printf("SDL_SetPaletteColors: %s\n", SDL_GetError());
		return -1;
	}
	
	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	Uint32 format = info.texture_formats[0];
	texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STREAMING,
                                surface->w, surface->h);
	if (texture == NULL) {
		printf("SDL_CreateTexture: %s\n", SDL_GetError());
		return -1;
	}
	need_conv = (format != surface->format->format);
	if (need_conv) {
		// prepare a surface for convertion to texture
#ifdef DEBUG
		printf("Surface conversion needed\n");
#endif
		screen = SDL_ConvertSurfaceFormat(surface, format, 0);
		if (screen == NULL) {
			printf("SDL_ConvertSurfaceFormat: %s\n", SDL_GetError());
			return -1;
		}
	} else {
		screen = surface;
	}
	
	/* audio */
	SDL_AudioSpec want, have;
	
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "error: can't initialize SDL audio subsystem\n");
		exit(-1);
	}
	
	int devcount = SDL_GetNumAudioDevices(0);
#ifdef DEBUG
	printf("List of audio devices: \n");
	for (int i = 0; i < devcount; i++) {
		printf("%d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
	}
#endif
	
	if (!devcount) {
		fprintf(stderr, "warning: no audio device detected\n");
	}
	
	memset(&want, 0, sizeof(want));
	want.freq = 44100; // don't forget to convert buffers if that frequency is unavailable
	want.format = AUDIO_S16LSB;
	want.channels = 1;
	want.samples = 4096;
	want.callback = NULL;
	want.userdata = NULL;
	
	devid = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (!devid) {
		fprintf(stderr, "error: can't open default audio device (%s)\n", SDL_GetError());
		return -1;
	}
	SDL_PauseAudioDevice(devid, 0);
	
	sndbuf_sz = (sampling_freq / 60)*2; // buffer for 1/60th of a second
	sndbuf = malloc(sndbuf_sz);
	if (!sndbuf) {
		fprintf(stderr, "error: failed to allocate memory\n");
		return -1;
	}
	sample_max *= wave_ampl;
	sample_min *= wave_ampl;
	
	/* mutexes and condition variables */
	run_mutex = SDL_CreateMutex();
	if (run_mutex == NULL) {
		printf("SDL_CreateMutex: %s\n", SDL_GetError());
		return -1;
	}
	refresh_mutex = SDL_CreateMutex();
	if (refresh_mutex == NULL) {
		printf("SDL_CreateMutex: %s\n", SDL_GetError());
		return -1;
	}
	keystate_mutex = SDL_CreateMutex();
	if (keystate_mutex == NULL) {
		printf("SDL_CreateMutex: %s\n", SDL_GetError());
		return -1;
	}
	keystate_cond = SDL_CreateCond();
	if (keystate_cond == NULL) {
		printf("SDL_CreateCond: %s\n", SDL_GetError());
		return -1;
	}
	
	return 0;
}

int exec_sdl_threads(void)
{
	atomic_exec(refresh_window, NULL, refresh_mutex);
	atomic_exec(set_run, &(int){1}, run_mutex);
	
	quit_event.type = SDL_QUIT; // push this event in case of imminent exit
	
	event_th = SDL_CreateThread(events_loop, "Events loop", NULL);
	if (event_th == NULL) {
		printf("SDL_CreateThread: %s\n", SDL_GetError());
		return -1;
	}
	
	return 0;
}

void wait_sdl_threads(void)
{
	int res;
	
	SDL_WaitThread(event_th, &res);
	if (res < 0) {
		printf("Events thread returned unsuccessfully\n");
	} else {
		printf("Events thread returned successfully\n");
	}
}

void cleanup_sdl(void)
{
	SDL_CloseAudioDevice(devid);
	SDL_DestroyMutex(refresh_mutex);
	SDL_DestroyMutex(run_mutex);
	SDL_DestroyMutex(keystate_mutex);
	SDL_DestroyCond(keystate_cond);
}

int or_keystate(void *data)
{
	uint16_t mask;
	memcpy(&mask, data, sizeof(mask));
	keystate |= mask;
	SDL_CondSignal(keystate_cond);
	return 0;
}

int and_keystate(void *data)
{
	uint16_t mask;
	memcpy(&mask, data, sizeof(mask));
	keystate &= mask;
	SDL_CondSignal(keystate_cond);
	return 0;
}

int fill_sound_buffer(uint8_t *buf, size_t sz, int wave_freq)
{
	/* assuming S16LSB samples at amplitude 'ampl'
	 * for a square wave of frequency 'wave_freq'
	 * to a mono sound device of frequency 'sampling_freq' */
	
	unsigned period = (sampling_freq/2) / wave_freq;
	if ((sampling_freq/2) % wave_freq == 0) {
		period++; // ceiling
	}
	
	for (size_t i = 0; i < sz; i += 2) {
		if (samples_count % period == 0) {
			wave = wave ? 0 : 1;
		}
		
		if (wave) { 
			buf[i]   = sample_max & 0xFF;
			buf[i+1] = sample_max >> 8;
		} else {
			buf[i]   = sample_min & 0xFF;
			buf[i+1] = sample_min >> 8;
		}
		
		samples_count++;
	}
	
	return 0;
}

int play_sdl_sound(int timer)
{
	for (int i = 0; i < timer; i++) {
		fill_sound_buffer(sndbuf, sndbuf_sz, wave_freq);
		SDL_QueueAudio(devid, sndbuf, sndbuf_sz);
	}
	return 0;
}

int stop_sdl_sound(void)
{
	SDL_ClearQueuedAudio(devid);
	return 0;
}
