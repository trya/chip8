int init_sdl(void);
int exec_sdl_threads(void);
void wait_sdl_threads(void);
void cleanup_sdl(void);

int play_sdl_sound(int timer);
int stop_sdl_sound(void);

extern uint8_t *screen_buf;
extern SDL_mutex *refresh_mutex;
int blit_screen_surface(void *data);
int refresh_window(void *data);

extern uint16_t keystate;
extern SDL_mutex *keystate_mutex;
extern SDL_cond *keystate_cond;
