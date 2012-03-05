/*
 * I2C.c
 *
 * Created: 17-02-2012 13:47:45
 *  Author: Vinnie Juul
 */ 

#include <avr/io.h>
#define F_CPU 3686400

void i2c_init();
void i2c_start();
void i2c_write(unsigned char data);
unsigned char i2c_read(unsigned char isLast);
void i2c_stop();