#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "emu.h"
#include "system.h"
#include "opcodes.h"
#include "iface/ctrls.h"

struct chip8_state st;
uint16_t op;
size_t op_count; // for debugging purposes
int run = 1;
int reset = 0;

// TODO: implement stop, pause, resume, reset, etc.
void emulation_loop(FILE *rom_file)
{
	if (init_timers_ctrl() < 0) {
		printf("Failed to init timers\n");
		return;
	}
	
#ifdef DEBUG
	op_count = 0;
#endif
chip8_start:
	init_state(&st);
	puts("loading rom...");
	if (!load_program(rom_file, &st)) {
		fprintf(stderr, "error: could not load rom\n");
		return;
	}
	puts("rom loaded successfully, starting program now");
	
	// TODO: catch stop signal or opcode exception
	while (run && !reset) { // TODO: atomic get
		op = fetch_opcode(&st);
#ifdef DEBUG
		print_state_lite(&st);
		print_opcode(op, &st);
#endif
		exec_opcode(op, &st);
#ifdef DEBUG
		op_count++;
#endif
	}
	
	if (reset) {
		puts("resetting...");
		reset = 0;
		goto chip8_start;
	}
	
	puts("emu loop exited");
}

// TODO: signal problems with error code set on external variable
