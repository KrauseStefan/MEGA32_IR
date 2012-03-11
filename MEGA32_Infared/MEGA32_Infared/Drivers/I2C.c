/************************************************
*  AMS: LAB16                                   * 
*  LM75 and the Mega32 2-wire Interface         *
*                                               *
*  STK500 setup:                                *
*    Header "RS232 spare" connected to RXD/TXD: *
*      RXD = PORTD, bit0                        *
*      TXD = PORTD, bit1                        *
*    I2C data bus (LM75 slaves):                *
*      SCL connected to PORTC, bit0             *
*      SDA connected to PORTC, bit1             *
*    Two LM75 slaves powered from STK500:       *
*      Address no.1 = 0.                        *
*      Address no.2 = 7.                        *
*                                               *
*   Henning Hargaard, January 11, 2012          * 
*************************************************/
#include <avr/io.h>
#include <avr/delay.h>
#include "I2C.h"
#include "uart.h"

void i2c_init()
{
  // ---> Write code here to initialize the TWBR register
  // LM75 clock has be to be lower than 400 kHz (according to LM75 data sheet)
  // The TWBR must be at least 10 in master mode (Mega32 data book)
  // 3686,4kHz/(16+2(TWBR)*4^TWPS)
  TWBR = 0x0A;
  //Prescaler value = 1;
  TWSR = TWSR & 0b11111100;
  
}

void i2c_start()
{
  TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
  //When TWINT is 1, start condition sent
  while ((TWCR & (1<<TWINT)) == 0)
  {}  	
}

void i2c_write(unsigned char data)
{
  TWDR = data;
  TWCR = (1<<TWINT) | (1<<TWEN);
  while ((TWCR & (1<<TWINT)) == 0)
  {}	
}

unsigned char i2c_read (unsigned char isLast)
{
  if (isLast == 0) //If we want to read more than 1 byte
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
  else             //If we want to read only one byte
    TWCR = (1<<TWINT) | (1<<TWEN);
  while ((TWCR & (1<<TWINT)) == 0)
  {}
  return TWDR;
}

void i2c_stop()
{
  TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}
