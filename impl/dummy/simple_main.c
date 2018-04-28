#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "iface/vid.h"
#include "iface/aud.h"
#include "iface/ks.h"
#include "core/emu.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s rom_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	FILE *rom_file = fopen(argv[1], "r");
	if (!rom_file) {
		perror("fopen");
		fprintf(stderr, "Failed to open rom file\n");
		exit(EXIT_FAILURE);
	}
	
	if (init_video_feed() < 0) {
		printf("Failed to init video feed\n");
		exit(EXIT_FAILURE);
	}
	if (init_audio_feed() < 0) {
		printf("Failed to init audio feed\n");
		exit(EXIT_FAILURE);
	}
	if (init_keystate_feed() < 0) {
		printf("Failed to init keystate feed\n");
		exit(EXIT_FAILURE);
	}
	
	emulation_loop(rom_file);
	
	fclose(rom_file);
	puts("Bye!");
	
	return 0;
}
