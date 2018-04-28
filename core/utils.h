static inline uint8_t u16_first_nibble(uint16_t word)
{
	uint8_t nibble = word >> 12;
	return nibble;
}

static inline uint8_t u16_second_nibble(uint16_t word)
{
	uint8_t nibble = (word >> 8) & 0xF;
	return nibble;
}

static inline uint8_t u16_third_nibble(uint16_t word)
{
	uint8_t nibble = (word >> 4) & 0xF;
	return nibble;
}

static inline uint8_t u16_fourth_nibble(uint16_t word)
{
	uint8_t nibble = word & 0xF;
	return nibble;
}

static inline uint8_t u16_high_byte(uint16_t word)
{
	uint8_t byte = word >> 8;
	return byte;
}

static inline uint8_t u16_low_byte(uint16_t word)
{
	uint8_t byte = word & 0xFF;
	return byte;
}

static inline uint8_t u8_rand(void)
{
	uint8_t byte = rand() % 0x100;
	return byte;
}
