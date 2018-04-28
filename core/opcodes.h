uint16_t fetch_opcode(struct chip8_state *st);
void exec_opcode(uint16_t op, struct chip8_state *st);
void print_opcode(uint16_t op, struct chip8_state *st);

struct op_desc {
	char op[5];
	char desc[81]; // 80 columns + '/0'
};

// TODO: print opcode description
