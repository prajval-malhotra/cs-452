#include "util.h"
#include "rpi.h"

unsigned int ROWS = 200;
unsigned int COLS = 100;
unsigned int seed = 1;

void move_cursor_debug(unsigned int row, unsigned int col) {
	uart_printf(CONSOLE, "\033[%d;%dH", row, col);
}

unsigned int rand(unsigned int min, unsigned int max) {
    unsigned int a = 1664525;
    unsigned int c = 1013904223;

    seed = (a * seed + c);

    unsigned int scaled_random = seed % (max - min + 1) + min;

    return scaled_random;
}

int get_num(char ch) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	else return -1;
}

// ascii digit to integer
int a2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

// unsigned int to ascii string
void ui2a( unsigned int num, unsigned int base, char *bf ) {
	int n = 0;
	int dgt;
	unsigned int d = 1;

	while( (num / d) >= base ) d *= base;
	while( d != 0 ) {
		dgt = num / d;
		num %= d;
		d /= base;
		if( n || dgt > 0 || d == 0 ) {
			*bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
			++n;
		}
	}
	*bf = 0;
}

// signed int to ascii string
void i2a( int num, char *bf ) {
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	ui2a( num, 10, bf );
}

// define our own memset to avoid SIMD instructions emitted from the compiler
void *memset(void *s, int c, size_t n) {
  for (char* it = (char*)s; n > 0; --n) *it++ = c;
  return s;
}

// define our own memcpy to avoid SIMD instructions emitted from the compiler
void* memcpy(void* __restrict__ dest, const void* __restrict__ src, size_t n) {
  char* sit = (char*)src;
  char* cdest = (char*)dest;
  for (size_t i = 0; i < n; ++i) *(cdest++) = *(sit++);
  return dest;
}

// taken from https://stackoverflow.com/questions/32560167/strncmp-implementation
int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        ++s1;
        ++s2;
        --n;
    }
    if ( n == 0 ) {
        return 0;
    } else {
        return ( *(unsigned char *)s1 - *(unsigned char *)s2 );
    }
}

// taken from https://linux.die.net/man/3/strncpy
char *strncpy(char *dest, const char *src, size_t n) {
    size_t ind;

    for (ind = 0; ind < n && src[ind] != '\0'; ind++) {
        dest[ind] = src[ind];
    }
    for (; ind < n; ind++) {
        dest[ind] = '\0';
    }

    return dest;
}

