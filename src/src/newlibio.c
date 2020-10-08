#include <sys/stat.h>
#include <errno.h>
#include <libopencm3/stm32/usart.h>
#include "newlibio.h"

int _read(int file, char *ptr, int len) { return 0; }
int _close(int file) { return -1; }
int _lseek(int file, int ptr, int dir) { return 0; }
int _isatty(int file) { return 1; }

int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _write(int file, char *ptr, int len)
{
	int i=0;
	if (file == 1) {
		for (i = 0; i < len; i++)
			usart_send_blocking(USART1, ptr[i]);
		return i;
	}
	errno = EIO;
	return -1;
}

void *_sbrk(int incr) {
	static unsigned char *heap = NULL;
	unsigned char *prev_heap;
	if (heap == NULL) {
		heap = (unsigned char *)&end;
	}
	prev_heap = heap;
	heap += incr;
	return prev_heap;
}
