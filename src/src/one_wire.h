#ifndef _ONE_WIRE_H
#define _ONE_WIRE_H

/*
#define ONE_WIRE_WRITE_PORT GPIOA
#define ONE_WIRE_WRITE_PIN GPIO0
#define ONE_WIRE_WRITE_RCC_BIT RCC_GPIOA

#define ONE_WIRE_READ_PORT GPIOA
#define ONE_WIRE_READ_PIN GPIO7
#define ONE_WIRE_READ_RCC_BIT RCC_GPIOA
*/

int one_wire_setup(void);
void one_wire_stop(void);
void one_wire_write_read(int *w_bytes, int *r_bytes, int w_size, int r_size);
int one_wire_crc(int *data, int size);
#endif
