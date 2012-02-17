/*
 * IR.c
 *
 * Created: 17-02-2012 14:39:21
 *  Author: Vinnie Juul
 */ 

#include "IR.h"
#include <avr/interrupt.h>

int half_bit_time;
int three_quarter_bit_time;
char adr; // 5 bit
char cmd; // 6 bit
bool hold_bit;

void ir_init()
{
	//Setup interrupt 0
	//Setup Timer0 + timer0 interrupt
}

void ir_receive(char* _adr, char* _cmd, bool _hold_bit)
{
	//Tjek for invalid data first
	*_adr = adr;
	*_cmd = cmd;
	*_hold_bit = hold_bit;
}

//Input interrupt
ISR (INT0_vect)
{
	// Measure time between the 2 start bits
	// calculate half and quarter bit time
	// Enable timer0 interrupt
	// start timer 0 to measure first time after 3 quarter bit time
}

ISR (TIMER0_OVF_vect)
{
	// Set Compare register OCR0 first time there is Interrupt
	// Reset timer register TCNT0
	// Read input value
	// Increase a counter
	// after 24 times
	// Invalid data?
}
