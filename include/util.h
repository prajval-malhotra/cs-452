#ifndef _util_h_
#define _util_h_ 1

#include <stddef.h>

// extern unsigned int ROWS;
// extern unsigned int COLS;

// display
void move_cursor_debug(unsigned int row, unsigned int col);

// random number generator
unsigned int rand(unsigned int min, unsigned int max);

// conversions
int a2d(char ch);
char a2i( char ch, char **src, int base, int *nump );
void ui2a( unsigned int num, unsigned int base, char *bf );
void i2a( int num, char *bf );
int get_num(char ch);

// memory
void *memset(void *s, int c, size_t n);
void* memcpy(void* __restrict__ dest, const void* __restrict__ src, size_t n);

// strings
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dest, const char *src, size_t n);

#endif /* util.h */

