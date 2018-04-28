CFLAGS     = -I. -O3
BINDIR     = bin
C_APPS     = chip8_dummy
LINUX_APPS = chip8_sdl

all: app_dir c_apps linux_apps

debug: CFLAGS = -I. -pedantic -g -DDEBUG -Wall
debug: all

# objects
CORE_OBJS = core/emu.o \
            core/system.o \
            core/opcodes.o

DUMMY_OBJS = impl/dummy/ctrls.o \
             impl/dummy/feeds.o \
             impl/dummy/simple_main.o

SDL_OBJS = impl/sdl/sdl.o \
           impl/sdl/ctrls.o \
           impl/sdl/feeds.o \
           impl/sdl/simple_main.o

%.o: %.c
	$(CC) -o $@ -c $^ $(CFLAGS)

app_dir:
	mkdir -p $(BINDIR)

c_apps: CFLAGS += -std=c99
c_apps: $(C_APPS)

linux_apps: CFLAGS += -std=gnu99
linux_apps: $(LINUX_APPS)

# app-specific targets
chip8_dummy: $(CORE_OBJS) $(DUMMY_OBJS)
	$(CC) -o $(BINDIR)/$@ $^
	
chip8_sdl: $(CORE_OBJS) $(SDL_OBJS)
	$(CC) -lSDL2 -o $(BINDIR)/$@ $^

clean:
	rm -f $(CORE_OBJS) $(DUMMY_OBJS) $(SDL_OBJS)
	rm -f $(addprefix $(BINDIR)/, $(C_APPS))
	rm -f $(addprefix $(BINDIR)/, $(LINUX_APPS))
