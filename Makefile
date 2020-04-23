CC =
CFLAGS =
C_APPS = chip8_dummy
LINUX_APPS = chip8_sdl

all: c_apps linux_apps

# objects
dirobjs = $(patsubst %.c,%.o,$(wildcard $(1)/*.c))
CORE_OBJS = $(call dirobjs,core)
DUMMY_OBJS = $(call dirobjs,impl/dummy)
SDL_OBJS = $(call dirobjs,impl/sdl)

%.o: %.c
	$(CC) -o $@ -c $^ -I. $(CFLAGS)

c_apps: CFLAGS += -std=c99
c_apps: $(C_APPS)

linux_apps: CFLAGS += -std=gnu99
linux_apps: $(LINUX_APPS)

# app-specific targets
chip8_dummy: $(CORE_OBJS) $(DUMMY_OBJS)
	$(CC) -o $@ $^

chip8_sdl: $(CORE_OBJS) $(SDL_OBJS)
	$(CC) -lSDL2 -o $@ $^

clean:
	rm -f $(CORE_OBJS) $(DUMMY_OBJS) $(SDL_OBJS)
	rm -f $(C_APPS) $(LINUX_APPS)

.PHONY: all clean
