#define XRES 64
#define YRES 32

#define MEM_SIZE      0x1000
#define PROGRAM_START 0x200
#define PROGRAM_SIZE  MEM_SIZE-PROGRAM_START
#define STACK_SIZE    16

struct chip8_state {
	uint8_t mem[MEM_SIZE]; // system RAM
	uint8_t screen_buf[YRES][XRES]; // screen buffer (1 byte per pixel)
	// [row=y][col=x]

	uint16_t pc; // program counter
	uint8_t vx[16]; // general-purpose registers
	uint16_t i; // address register

	uint16_t stack[STACK_SIZE]; // call stack
	uint8_t si; // stack index

	uint8_t d_timer; // delay timer
	uint8_t s_timer; // sound timer

	uint16_t ks;
};

void init_state(struct chip8_state *st);
void print_state(struct chip8_state *st);
void print_state_lite(struct chip8_state *st);
size_t load_program(FILE *prog_file, struct chip8_state *st);
