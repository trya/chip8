#define RES_X 64
#define RES_Y 32
#define DEPTH_BITS 8

#define QUOTIENT(a,b) ((a%b==0) ? a/b : (a/b)+1)
#define PITCH_SIZE ((DEPTH_BITS < 8) ? QUOTIENT(RES_X, 8) : QUOTIENT(RES_X*DEPTH_BITS, 8))

#define PIXBUF_SIZE (PITCH_SIZE * RES_Y)
