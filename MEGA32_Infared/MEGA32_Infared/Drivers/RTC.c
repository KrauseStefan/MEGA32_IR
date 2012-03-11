/*
 * RTC.c
 *
 * Created: 03-02-2012 13:52:30
 *  Author: Vinnie Juul
 */ 
#include <avr/io.h>
#include "RTC.h"
#include <avr/delay.h>

void WriteData(unsigned char addr, unsigned char data)
{
unsigned char dummy;	
	
  // Activate RTC chip select
  PORTB |= (1<<CE_PIN);
  // Send address
  SPDR = addr; 
  // Await address sent
  while(!(SPSR&(1<<SPIF)))
  {}
  // Clear SPIF flag
  dummy = SPSR;
  dummy = SPDR;
  
  // Send data
  SPDR = data;
  // Await data sent 
  while(!(SPSR&(1<<SPIF)))
  {}
  // Clear SPIF flag
  dummy = SPSR;
  dummy = SPDR;
  // Deactivate RTC chip select
  PORTB &= !(1<<CE_PIN);   
}

unsigned char ReadData(unsigned char addr)
{
unsigned char dummy;
	
  // Activate RTC chip select
  PORTB |= (1<<CE_PIN);
  // Send address
  SPDR = addr;
  // Await address sent
  while(!(SPSR&(1<<SPIF)))
  {}
  // Clear SPIF flag
  dummy = SPSR;
  dummy = SPDR;
    
  // Send "dummy byte" (to generate 8 clock periods)
  SPDR = 0; 
  // Await dummy sent
  while(!(SPSR&(1<<SPIF)))
  {}
  // Clear SPIF flag  
  dummy = SPSR;
  dummy = SPDR;
  // Deactivate RTC chip select  
  PORTB &= !(1<<CE_PIN);
  // Read and return the received data byte 
  return(SPDR);
}

// Converts binary number to BCD
unsigned char BCD(unsigned char binary)
{
unsigned char BCDresult;
   BCDresult = (binary/10)<<4;
   BCDresult |= binary%10;
   return(BCDresult);  
}

// Converts BCD number to binary
unsigned char FromBCD(unsigned char BCDnumber)
{
  return(((BCDnumber>>4)*10) + (BCDnumber & 0x0F));
}

void SPI_MasterInit(void)
{
	unsigned char dummy;
	
	// SCLK, MOSI And SS
	DDRB = 0b10110000;
		
	//enable Interrupt
	//enable spi
	//msb first
	//MAster/slave
	//clock polarity
	//Clock phase
	//clock selection - 64
	SPCR = 0b01010101;	
	
	dummy = SPSR;
	dummy = SPDR;
}

void RTCInit()
{
	SPI_MasterInit();
	
	//DDRB = 1<<4;
	//Unlock clock, bit 7 = clock, 6 = lock
	//WriteData(0x8F, 0b00000000);
	////Starts clock
	//WriteData(0x8F, 0b00000000);
}

// Sets clock registers according to the parameters 
void SetClock(unsigned char year, unsigned char month, unsigned char date, unsigned char day,
              unsigned char hour, unsigned char minutes, unsigned char seconds)
{    
	WriteData(0x86, BCD(year));
	WriteData(0x85, BCD(month-1));
	WriteData(0x84, BCD(date));
	WriteData(0x83, BCD(day-1));
	char hour_int = BCD(hour) & 0b10111111;
	WriteData(0x82, hour_int);
	WriteData(0x81, BCD(minutes));
	WriteData(0x80, BCD(seconds));
}              

// Returns 10's of year (ASCII)
char Year10()
{
	return FromBCD(ReadData(0x06))/10;
}
 

// Returns 1's of year (ASCII)
char Year1()
{  
	return FromBCD(ReadData(0x06))%10;
}

// Returns month (0-11)
unsigned char Month()
{
	return FromBCD(ReadData(0x05));
}

// Returns no.of day (1-7)
unsigned char Day()
{
	return ReadData(0x03);
}

char Date10()
{
	return FromBCD(ReadData(0x04))/10;
}

char Date1()
{
	return FromBCD(ReadData(0x04))%10;
}

char Hours10()
{
	return FromBCD(ReadData(0x02))/10;
}

char Hours1()
{
	char hour = FromBCD(ReadData(0x02));
	return (hour%10);
}

char Minutes10()
{
	char min = FromBCD(ReadData(0x01));
	return (min/10);
}

char Minutes1()
{
	char min = FromBCD(ReadData(0x01));
	return (min%10);
}

char Seconds10()
{
	char sec = FromBCD(ReadData(0x00));
	return (sec/10);
}

char Seconds1()
{
	char sec = FromBCD(ReadData(0x00));
	return (sec%10);
}