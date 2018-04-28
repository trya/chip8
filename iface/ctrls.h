// keypad controller
int init_keypad_ctrl(void);
int refresh_keystate(struct chip8_state *st);
int is_key_pressed(int key, struct chip8_state *st);
int wait_keypress(struct chip8_state *st);

// video controller
int init_video_ctrl(void);
int draw_screen(struct chip8_state *st);

// audio controller
int init_audio_ctrl(void);
int enable_sound(int timer);
int disable_sound(void);

// timers controller
int init_timers_ctrl(void);
int refresh_timers(struct chip8_state *st);
