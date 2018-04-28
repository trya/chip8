#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "system.h"
#include "utils.h"

unsigned char chip8_fontset[80] =
{
	/* highest 4 bits are pixels, the rest are ignored */
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void init_state(struct chip8_state *st)
{
	memset(st, 0, sizeof(*st));
	st->pc = PROGRAM_START;
	/* copy fonts to 0x0 */
	memcpy(st->mem, chip8_fontset, sizeof(chip8_fontset));
	
	/* set a seed for the PRNG */
	srand(time(NULL));
}

void print_state(struct chip8_state *st)
{
	/* memory */
	printf("memory:\n");
	int x, y, addr;
	for (y = 0, addr = 0; y < MEM_SIZE/16; y++) {
		printf("%3x:", addr);
		for (x = 0; x < 16; x++) {
			printf(" %02x", st->mem[addr]);
			addr++;
		}
		putchar('\n');
	}
	putchar('\n');
	
	/* screen */
	printf("screen:\n");
	for (y = 0; y < YRES; y++) {
		for (x = 0; x < XRES; x++) {
			if (st->screen_buf[y][x]) {
				putchar('X');
			} else {
				putchar('.');
			}
		}
		putchar('\n');
	}
	putchar('\n');
	
	/* registers */
	printf("registers:\n");
	printf("pc: 0x%03x\t", st->pc);
	printf("i: 0x%03x\n", st->i);
	putchar('\n');
	printf("vx:\n");
	printf("#: ");
	for (x = 0; x <= 0xF; x++) {
		printf(" %2x", x);
	}
	putchar('\n');
	printf("v: ");
	for (x = 0; x <= 0xF; x++) {
		printf(" %2x", st->vx[x]);
	}
	printf("\n\n");
	
	/* stack */
	printf("stack:\n");
	for (x = 0; x < STACK_SIZE; x++) {
		printf(" %3x", st->stack[x]);
	}
	putchar('\n');
	printf("stack index: %u\n", st->si);
	putchar('\n');
	
	/* timers */
	printf("timers:\n");
	printf("delay: %x\t", st->d_timer);
	printf("sound: %x\n", st->s_timer);
	putchar('\n');
	
	/* keystate */
	uint16_t mask;
	char s[17] = {0};
	for (int i = 0; i < 16; i++) {
		mask = (1 << i);
		s[i] = (st->ks & mask) ? '1' : '0';
	}
	printf("ks: %s\n", s);
	putchar('\n');
}

void print_state_lite(struct chip8_state *st)
{
	int x;
	
	/* registers */
	printf("registers:\n");
	printf("pc: 0x%03x\t", st->pc);
	printf("i: 0x%03x\n", st->i);
	putchar('\n');
	printf("vx:\n");
	printf("#: ");
	for (x = 0; x <= 0xF; x++) {
		printf(" %2x", x);
	}
	putchar('\n');
	printf("v: ");
	for (x = 0; x <= 0xF; x++) {
		printf(" %2x", st->vx[x]);
	}
	printf("\n\n");
	
	/* stack */
	printf("stack:\n");
	for (x = 0; x < STACK_SIZE; x++) {
		printf(" %3x", st->stack[x]);
	}
	putchar('\n');
	printf("stack index: %u\n", st->si);
	putchar('\n');
	
	/* timers */
	printf("timers:\n");
	printf("delay: %x\t", st->d_timer);
	printf("sound: %x\n", st->s_timer);
	putchar('\n');
	
	/* keystate */
	uint16_t mask;
	char s[17] = {0};
	for (int i = 0; i < 16; i++) {
		mask = (1 << i);
		s[i] = (st->ks & mask) ? '1' : '0';
	}
	printf("ks: %s\n", s);
	putchar('\n');
}

size_t load_program(FILE *prog_file, struct chip8_state *st)
{
	rewind(prog_file);
	return fread(&st->mem[PROGRAM_START], 1, PROGRAM_SIZE, prog_file);
}
