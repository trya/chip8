#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "system.h"
#include "opcodes.h"
#include "utils.h"
#include "iface/ctrls.h"

static void op_0(void);
static void op_1(void);
static void op_2(void);
static void op_3(void);
static void op_4(void);
static void op_5(void);
static void op_6(void);
static void op_7(void);
static void op_8(void);
static void op_9(void);
static void op_A(void);
static void op_B(void);
static void op_C(void);
static void op_D(void);
static void op_E(void);
static void op_F(void);

static void op_FX0(void);
static void op_FX1(void);
static void op_FX2(void);
static void op_FX3(void);
static void op_FX5(void);
static void op_FX6(void);

static void op_00E0(void);
static void op_00EE(void);
static void op_0NNN(void);
static void op_1NNN(void);
static void op_2NNN(void);
static void op_3XNN(void);
static void op_4XNN(void);
static void op_5XY0(void);
static void op_6XNN(void);
static void op_7XNN(void);
static void op_8XY0(void);
static void op_8XY1(void);
static void op_8XY2(void);
static void op_8XY3(void);
static void op_8XY4(void);
static void op_8XY5(void);
static void op_8XY6(void);
static void op_8XY7(void);
static void op_8XYE(void);
static void op_9XY0(void);
static void op_ANNN(void);
static void op_BNNN(void);
static void op_CXNN(void);
static void op_DXYN(void);
static void op_EX9E(void);
static void op_EXA1(void);
static void op_FX07(void);
static void op_FX0A(void);
static void op_FX15(void);
static void op_FX18(void);
static void op_FX1E(void);
static void op_FX29(void);
static void op_FX33(void);
static void op_FX55(void);
static void op_FX65(void);
static void op_null(void);

static struct chip8_state *st_cur;
static uint16_t op_cur;

struct op_desc op_descriptions[36] = {
	{ "0NNN", "call rca 1802 at %u (ignored)" },
	{ "00E0", "clear screen" },
	{ "00EE", "return" },
	{ "1NNN", "jump to %u" },
	{ "2NNN", "call %u" },
	{ "3XNN", "skip next if V%u == %u" },
	{ "4XNN", "skip next if V%u != %u" },
	{ "5XY0", "skip next if V%u == V%u" },
	{ "6XNN", "V%u := %u" },
	{ "7XNN", "V%u += %u" },
	{ "8XY0", "V%u := V%u" },
	{ "8XY1", "V%u |= V%u" },
	{ "8XY2", "V%u &= V%u" },
	{ "8XY3", "V%u ^= V%u" },
	{ "8XY4", "V%u += V%u" },
	{ "8XY5", "V%u -= V%u" },
	{ "8XY6", "V%u >= 1" },
	{ "8XY7", "V%u := V%u - V%u" },
	{ "8XYE", "V%u <= 1" },
	{ "9XY0", "skip next if V%u != V%u" },
	{ "ANNN", "I := %u" },
	{ "BNNN", "jump to %u + V0" },
	{ "CXNN", "V%u := RND & %u" },
	{ "DXYN", "draw sprite on %u lines" },
	{ "EX9E", "skip next if key(v%u) is pressed" },
	{ "EXA1", "skip next if key(v%u) is not pressed" },
	{ "FX07", "V%u := DT" },
	{ "FX0A", "V%u := putkey()" },
	{ "FX15", "DT := V%u" },
	{ "FX18", "ST := V%u" },
	{ "FX1E", "I += V%u" },
	{ "FX29", "I := &key(V%u)" },
	{ "FX33", "store BCD(V%u) from I to I+2" },
	{ "FX55", "store V0 to V%u to I" },
	{ "FX65", "restore V0 to V%u from I" },
	
	{ "null", "" }
};

static void (*op_table_N[])(void) = {
	op_0, op_1, op_2, op_3, op_4, op_5, op_6, op_7,
	op_8, op_9, op_A, op_B, op_C, op_D, op_E, op_F
};

static void (*op_table_5[])(void) = {
	op_5XY0, op_null, op_null, op_null, op_null, op_null, op_null, op_null,
	op_null, op_null, op_null, op_null, op_null, op_null, op_8XYE, op_null
};

static void (*op_table_8[])(void) = {
	op_8XY0, op_8XY1, op_8XY2, op_8XY3, op_8XY4, op_8XY5, op_8XY6, op_8XY7,
	op_null, op_null, op_null, op_null, op_null, op_null, op_8XYE, op_null
};

static void (*op_table_9[])(void) = {
	op_9XY0, op_null, op_null, op_null, op_null, op_null, op_null, op_null,
	op_null, op_null, op_null, op_null, op_null, op_null, op_8XYE, op_null
};

static void (*op_table_F[])(void) = {
	op_FX0 , op_FX1 , op_FX2 , op_FX3 ,	op_null, op_FX5 , op_FX6 , op_null,
	op_null, op_null, op_null, op_null,	op_null, op_null, op_null, op_null
};

static void (*op_table_FX0[])(void) = {
	op_null, op_null, op_null, op_null, op_null, op_null, op_null, op_FX07,
	op_null, op_null, op_FX0A, op_null, op_null, op_null, op_null, op_null
};

static void (*op_table_FX1[])(void) = {
	op_null, op_null, op_null, op_null, op_null, op_FX15, op_null, op_null,
	op_FX18, op_null, op_null, op_null, op_null, op_null, op_FX1E, op_null
};

static void (*op_table_FX2[])(void) = {
	op_null, op_null, op_null, op_null, op_null, op_null, op_null, op_null,
	op_null, op_FX29, op_null, op_null, op_null, op_null, op_null, op_null
};

static void (*op_table_FX3[])(void) = {
	op_null, op_null, op_null, op_FX33, op_null, op_null, op_null, op_null,
	op_null, op_null, op_null, op_null, op_null, op_null, op_null, op_null
};

static void (*op_table_FX5[])(void) = {
	op_null, op_null, op_null, op_null, op_null, op_FX55, op_null, op_null,
	op_null, op_null, op_null, op_null, op_null, op_null, op_null, op_null
};

static void (*op_table_FX6[])(void) = {
	op_null, op_null, op_null, op_null, op_null, op_FX65, op_null, op_null,
	op_null, op_null, op_null, op_null, op_null, op_null, op_null, op_null
};

static void op_0(void)
{
	switch (u16_low_byte(op_cur)) {
	case 0xE0:
		op_00E0();
		break;
	case 0xEE:
		op_00EE();
		break;
	default:
		op_0NNN();
	}
}

/* .NNN group */
static void op_1(void)
{
	op_1NNN();
}

static void op_2(void)
{
	op_2NNN();
}

static void op_A(void)
{
	op_ANNN();
}

static void op_B(void)
{
	op_BNNN();
}

/* .XNN group */
static void op_3(void)
{
	op_3XNN();
}

static void op_4(void)
{
	op_4XNN();
}

static void op_6(void)
{
	op_6XNN();
}

static void op_7(void)
{
	op_7XNN();
}

static void op_C(void)
{
	op_CXNN();
}

/* .XY0 group */
static void op_5(void)
{
	op_table_5[u16_fourth_nibble(op_cur)]();
}

static void op_9(void)
{
	op_table_9[u16_fourth_nibble(op_cur)]();
}

/* .XYN group */
static void op_D(void)
{
	op_DXYN();
}

/* 8XY. group */
static void op_8(void)
{
	op_table_8[u16_fourth_nibble(op_cur)]();
}

/* EX.. group */
static void op_E(void)
{
	switch (u16_low_byte(op_cur)) {
	case 0x9E:
		op_EX9E();
		break;
	case 0xA1:
		op_EXA1();
		break;
	default:
		op_null();
	}
}

/* FX.. group */
static void op_F(void)
{
	op_table_F[u16_third_nibble(op_cur)]();
}

static void op_FX0(void)
{
	op_table_FX0[u16_fourth_nibble(op_cur)]();
}

static void op_FX1(void)
{
	op_table_FX1[u16_fourth_nibble(op_cur)]();
}

static void op_FX2(void)
{
	op_table_FX2[u16_fourth_nibble(op_cur)]();
}

static void op_FX3(void)
{
	op_table_FX3[u16_fourth_nibble(op_cur)]();
}

static void op_FX5(void)
{
	op_table_FX5[u16_fourth_nibble(op_cur)]();
}

static void op_FX6(void)
{
	op_table_FX6[u16_fourth_nibble(op_cur)]();
}

/* opcode 00E0: clears the screen */
static void op_00E0(void)
{
	memset(st_cur->screen_buf, 0, sizeof(st_cur->screen_buf));
	
	if (draw_screen(st_cur) < 0) {
		fprintf(stderr, "error: could not draw screen\n");
		// signal problem
	}
}

/* opcode 00EE: returns from a subroutine */
static void op_00EE(void)
{
	// no check is done on si, which could be 0
	st_cur->pc = st_cur->stack[--st_cur->si];
}

/* opcode 0NNN: calls RCA 1802 program at address NNN */
static void op_0NNN(void)
{
	// not supported
}

/* opcode 1NNN: jumps to address NNN */
static void op_1NNN(void)
{
	st_cur->pc = op_cur & 0xFFF;
}

/* opcode 2NNN: calls subroutine at NNN */
static void op_2NNN(void)
{
	// no check is done on si, which could be 15
	st_cur->stack[st_cur->si++] = st_cur->pc;
	st_cur->pc = op_cur & 0xFFF;
}

/* opcode ANNN: sets I to the address NNN */
static void op_ANNN(void)
{
	st_cur->i = op_cur & 0xFFF;
}

/* opcode BNNN: jumps to the address NNN plus V0 */
static void op_BNNN(void)
{
	st_cur->pc = (op_cur & 0xFFF) + st_cur->vx[0];
}

/* opcode 3XNN: skips the next instruction if VX equals NN */
static void op_3XNN(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t nn = u16_low_byte(op_cur);
	if (st_cur->vx[idx] == nn) {
		st_cur->pc += 2;
	}
}

/* opcode 4XNN: skips the next instruction if VX doesn't equal NN */
static void op_4XNN(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t nn = u16_low_byte(op_cur);
	if (st_cur->vx[idx] != nn) {
		st_cur->pc += 2;
	}
}

/* opcode 6XNN: sets VX to NN */
static void op_6XNN(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t nn = u16_low_byte(op_cur);
	st_cur->vx[idx] = nn;
}

/* opcode 7XNN: adds NN to VX */
static void op_7XNN(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t nn = u16_low_byte(op_cur);
	st_cur->vx[idx] += nn;
}

/* opcode CXNN: sets VX to the result of a bitwise and operation
 * on a random number and NN */
static void op_CXNN(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t nn = u16_low_byte(op_cur);
	st_cur->vx[idx] = u8_rand() & nn;
}

/* opcode 5XY0: skips the next instruction if VX equals VY */
static void op_5XY0(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t idy = u16_third_nibble(op_cur);
	if (st_cur->vx[idx] == st_cur->vx[idy]) {
		st_cur->pc += 2;
	}
}

/* opcode 9XY0: skips the next instruction if VX doesn't equal VY */
static void op_9XY0(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t idy = u16_third_nibble(op_cur);
	if (st_cur->vx[idx] != st_cur->vx[idy]) {
		st_cur->pc += 2;
	}
}

/* opcode DXYN: draws N 8-bit sprites at I starting with (VX,VY),
 * the sprite shall not wrap if it has pixels which coordinates
 * (modulo the screen resolution) go over the screen borders
 * (cf. http://laurencescotford.co.uk/?p=304) */
static void op_DXYN(void)
{
	uint8_t x = st_cur->vx[u16_second_nibble(op_cur)] % XRES;
	uint8_t y = st_cur->vx[u16_third_nibble(op_cur)] % YRES;
	uint8_t n = u16_fourth_nibble(op_cur);
	uint16_t i = st_cur->i;
	
	// distances from screen borders
	int d_side = XRES - x;
	int d_bottom = YRES - y;
	
	// height and width of sprite on screen (max. 8xN)
	uint8_t w = d_side < 8 ? d_side : 8;
	uint8_t h = d_bottom < n ? d_bottom : n;
	
	/* drawing loop: XOR pixels with the a-th bit of the sprite at i,
	 * if at least one pixel is set from 1 to 0, VF is set to 1 */
	int a; // column
	uint8_t b; // line
	uint8_t sprite, pixel;
	st_cur->vx[0xF] = 0; // assuming no collision
	for (b = 0; b < h; b++) {
		sprite = st_cur->mem[i];
		for (a = 0; a < w; a++) {
			pixel = (sprite >> (7-a)) & 1;
			if (st_cur->screen_buf[y][x] != pixel) {
				st_cur->screen_buf[y][x] = 1;
			} else if (pixel) {
				st_cur->screen_buf[y][x] = 0;
				st_cur->vx[0xF] = 1;
			} /* else st_cur->screen_buf[y][x] == 0 && pixel == 0 */
			x++;
		}
		x -= w;
		y++;
		i++;
	}
	
	if (draw_screen(st_cur) < 0) {
		fprintf(stderr, "error: could not draw screen\n");
		// signal problem
	}
}

/* opcode 8XY0: set VX to the value of VY */
static void op_8XY0(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t idy = u16_third_nibble(op_cur);
	st_cur->vx[idx] = st_cur->vx[idy];
}

/* opcode 8XY1: sets VX to VX or VY */
static void op_8XY1(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t idy = u16_third_nibble(op_cur);
	st_cur->vx[idx] |= st_cur->vx[idy];
}

/* opcode 8XY2: sets VX to VX and VY */
static void op_8XY2(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t idy = u16_third_nibble(op_cur);
	st_cur->vx[idx] &= st_cur->vx[idy];
}

/* opcode 8XY3: sets VX to VX xor VY */
static void op_8XY3(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t idy = u16_third_nibble(op_cur);
	st_cur->vx[idx] ^= st_cur->vx[idy];
}

/* opcode 8XY4: adds VY to VX.
 * VF is set to 1 when there's a carry, and to 0 when there isn't */
static void op_8XY4(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t idy = u16_third_nibble(op_cur);
	if (st_cur->vx[idy] > 0xFF - st_cur->vx[idx]) {
		st_cur->vx[0xF] = 1;
	} else {
		st_cur->vx[0xF] = 0;
	}
	st_cur->vx[idx] += st_cur->vx[idy];
}

/* opcode 8XY5: VY is subtracted from VX.
 * VF is set to 0 when there's a borrow, and 1 when there isn't */
static void op_8XY5(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t idy = u16_third_nibble(op_cur);
	if (st_cur->vx[idx] < st_cur->vx[idy]) {
		st_cur->vx[0xF] = 0;
	} else {
		st_cur->vx[0xF] = 1;
	}
	st_cur->vx[idx] -= st_cur->vx[idy];
}

/* opcode 8XY6: shifts VX right by one.
 * VF is set to the value of the least significant bit
 * of VX before the shift */
static void op_8XY6(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	st_cur->vx[0xF] = st_cur->vx[idx] & 1;
	st_cur->vx[idx] >>= 1;
}

/* opcode 8XY7: sets VX to VY minus VX.
 * VF is set to 0 when there's a borrow, and 1 when there isn't */
static void op_8XY7(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t idy = u16_third_nibble(op_cur);
	if (st_cur->vx[idy] < st_cur->vx[idx]) {
		st_cur->vx[0xF] = 1;
	} else {
		st_cur->vx[0xF] = 0;
	}
	st_cur->vx[idx] = st_cur->vx[idy] - st_cur->vx[idx];
}

/* opcode 8XYE: shifts VX left by one.
 * VF is set to the value of the most significant bit
 * of VX before the shift */
static void op_8XYE(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	st_cur->vx[0xF] = st_cur->vx[idx] & 0x80; // or 10000000b
	st_cur->vx[idx] <<= 1;
}

/* opcode EX9E: skips the next instruction
 * if the key stored in VX is pressed */
static void op_EX9E(void)
{
	if (refresh_keystate(st_cur) < 0) {
		fprintf(stderr, "error: could not refresh key state\n");
		// signal problem
	}
	
	uint8_t idx = u16_second_nibble(op_cur);
	if (is_key_pressed(st_cur->vx[idx], st_cur)) {
		st_cur->pc += 2;
	}
}

/* opcode EXA1: skips the next instruction
 * if the key stored in VX isn't pressed */
static void op_EXA1(void)
{
	if (refresh_keystate(st_cur) < 0) {
		fprintf(stderr, "error: could not refresh key state\n");
		// signal problem
	}
	
	uint8_t idx = u16_second_nibble(op_cur);
	if (!is_key_pressed(st_cur->vx[idx], st_cur)) {
		st_cur->pc += 2;
	}
}

/* opcode FX07: sets VX to the value of the delay timer */
static void op_FX07(void)
{
	if (refresh_timers(st_cur) < 0) {
		fprintf(stderr, "error: could not refresh timers\n");
		// signal problem
	}
	
	uint8_t idx = u16_second_nibble(op_cur);
	st_cur->vx[idx] = st_cur->d_timer;
}

/* opcode FX0A: a key press is awaited, and then stored in VX */
static void op_FX0A(void)
{
	int key;
	if ((key = wait_keypress(st_cur)) < 0) {
		fprintf(stderr, "error: keypress awaiting failed\n");
		// signal problem
	}
	
	uint8_t idx = u16_second_nibble(op_cur);
	st_cur->vx[idx] = key;
}

/* opcode FX15: sets the delay timer to VX */
static void op_FX15(void)
{
	if (refresh_timers(st_cur) < 0) {
		fprintf(stderr, "error: could not refresh timers\n");
		// signal problem
	}
	
	uint8_t idx = u16_second_nibble(op_cur);
	st_cur->d_timer = st_cur->vx[idx];
}

/* opcode FX15: sets the sound timer to VX */
static void op_FX18(void)
{
	if (refresh_timers(st_cur) < 0) {
		fprintf(stderr, "error: could not refresh timers\n");
		// signal problem
	}
	
	uint8_t idx = u16_second_nibble(op_cur);
	int rem_timer = st_cur->s_timer;
	st_cur->s_timer = st_cur->vx[idx];
	int diff = (int)st_cur->s_timer - rem_timer;
	
	// TODO: checks
	if (diff < 0) {
		/* discard current sound playing before replaying */
		disable_sound();
		enable_sound(st_cur->s_timer);
	} else if (diff > 0) {
		enable_sound(diff);
	}
}

/* opcode FX1E: adds VX to I */
static void op_FX1E(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	st_cur->i += st_cur->vx[idx];
}

/* opcode FX29: sets I to the location of the sprite
 * for the character in VX. Characters 0-F (in hexadecimal)
 * are represented by a 4x5 font. */
static void op_FX29(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	st_cur->i = st_cur->vx[idx] * 5; // height*x + origin, with origin = 0
}

/* opcode FX33: stores the binary-coded decimal representation of VX,
 * with the most significant of three digits at the address in I,
 * the middle digit at I plus 1, and the least significant digit at I plus 2. */
static void op_FX33(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	uint8_t vx = st_cur->vx[idx];
	
	uint8_t ones = vx % 10;
	vx /= 10;
	uint8_t tens = vx % 10;
	vx /= 10;
	/* since 0 <= vx <= 255, 0 <= vx/100 <= 2,
	 * so there's no need to apply modulus */
	uint8_t hundreds = vx;
	
	st_cur->mem[st_cur->i] = hundreds;
	st_cur->mem[st_cur->i+1] = tens;
	st_cur->mem[st_cur->i+2] = ones;
}

/* opcode FX55: stores V0 to VX (including VX) in memory
 * starting at address I */
static void op_FX55(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	memcpy(&st_cur->mem[st_cur->i], &st_cur->vx, idx+1);
}

/* opcode FX65: fills V0 to VX (including VX) with values
 * from memory starting at address I */
static void op_FX65(void)
{
	uint8_t idx = u16_second_nibble(op_cur);
	memcpy(&st_cur->vx, &st_cur->mem[st_cur->i], idx+1);
}

static void op_null(void)
{
	/* TBD: break here and ask the user, or default action
	 * (continue, debug, abort) */
	fprintf(stderr, "Illegal instruction 0x%04X at address 0x%03X\n", op_cur, st_cur->pc);
	exit(EXIT_FAILURE);
}

uint16_t fetch_opcode(struct chip8_state *st)
{
	uint8_t *mem = st->mem;
	uint16_t pc = st->pc;
	
	return (mem[pc] << 8) | mem[pc+1]; // 2-byte big-endian opcodes
}

void exec_opcode(uint16_t op, struct chip8_state *st)
{
	st->pc += 2;
	st_cur = st;
	op_cur = op;
	op_table_N[u16_first_nibble(op_cur)]();
}

void print_opcode(uint16_t op, struct chip8_state *st)
{
	printf("%03X: %04X\n", st->pc, op);
}
