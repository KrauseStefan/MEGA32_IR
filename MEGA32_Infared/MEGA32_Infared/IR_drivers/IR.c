/*
 * IR.c
 *
 * Created: 17-02-2012 14:39:21
 *  Author: Vinnie Juul
 */ 

#include "IR.h"
#include <avr/interrupt.h>

#define INPUT_INTERRUPT				INT0
#define ENABLE_INPUT_INTERRUPT		GICR |= 1 << INPUT_INTERRUPT
#define DISABLE_INPUT_INTERRUPT		GICR |= 0 << INPUT_INTERRUPT

#define ENABLE_TIMER0_COMP_INT		TIMSK |= 1 << 1 // TJEK AT DET ER RIGTIG BIT!
#define DISABLE_TIMER0_COMP_INT		TIMSK |= 0 << 1
#define TIMER0_COUNT_REG			TCNT0
#define TIMER0_COMP_REG				OCR0

#define NUMBER_OF_MEASURINGS		24

int half_bit_time;
int three_quarter_bit_time;
char adr; // 5 bit
char cmd; // 6 bit
bool hold_bit;

bool first_start_bit;
int meaurering_count;




void ir_init()
{
	//Setup interrupt	
	if(INPUT_INTERRUPT == INT0)
	{
		MCUCR |= 1 << 0;
		MCUCR |= 1 << 1;
	}
	else if(INPUT_INTERRUPT == INT1)
	{
		MCUCR |= 1 << 2;
		MCUCR |= 1 << 3;
	}
	else
	{
		MCUCSR |= 0 << 6;
	}
	
	//Enable interrupt
	ENABLE_INPUT_INTERRUPT;
	first_start_bit = true;
	
	//Setup Timer0
	//Prescaler 256
	// Normal mode
	// Normal compare mode
	TCCR0 = 0b00000100;	
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
	int bit_time;
	if(first_start_bit)
	{
		TIMER0_COMP_REG = 255;
		TIMER0_COUNT_REG = 0;
		ENABLE_TIMER0_COMP_INT;
		first_start_bit = !first_start_bit;
	}
	else{
		bit_time = TIMER0_COMP_REG;
		TIMER0_COUNT_REG = 0;
		half_bit_time = bit_time/2;
		three_quarter_bit_time = float(bit_time/4*3);
		TIMER0_COMP_REG = three_quarter_bit_time;
		meaurering_count = 0;
		DISABLE_INPUT_INTERRUPT;
	}
	// Measure time between the 2 start bits
	// calculate half and quarter bit time
	// Enable timer0 interrupt
	// start timer 0 to measure first time after 3 quarter bit time
}

ISR (TIMER0_COMP_vect)
{
	//SKAL VI STOPPE MED AT TÆLLE/LÆSE NÅR VI HAR LÆST 24 GANGE?
	TIMER0_COUNT_REG = 0;
	if(meaurering_count > 0 && meaurering_count < NUMBER_OF_MEASURINGS)
	{
		//READ INPUT VALUE
		meaurering_count++
	}
	else if(meaurering_count == 0)
	{
		TIMER0_COMP_REG = half_bit_time;
		//READ INPUT VALUE
		meaurering_count++;
	}
	else
	{
		// INVALID DATA
		// output invalidt message
	}
	// Set Compare register OCR0 first time there is Interrupt
	// Reset timer register TCNT0
	// Read input value
	// Increase a counter
	// after 24 times
	// Invalid data?
}
