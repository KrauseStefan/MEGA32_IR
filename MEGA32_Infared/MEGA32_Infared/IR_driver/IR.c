/*
 * IR.c
 *
 * Created: 17-02-2012 14:39:21
 *  Author: Vinnie Juul
 */ 

#include "IR.h"
#include <avr/interrupt.h>
#include <limits.h>
#include <stdio.h>

<<<<<<< HEAD
#define LED_DEBUG
=======
#define PRINT_DEBUG
#ifdef PRINT_DEBUG
	#include "../Drivers/uart.h"
#endif

>>>>>>> 9ca087fcfe2111e8394aa1bd97beab9cea6634d5

#ifdef LED_DEBUG
	#include "../Drivers/led.h"
	unsigned char led = 0;
#endif

#define INPUT_INTERRUPT				INT0
#define INPUT_PIN					((PIND & (1 << PIND2)) >> PIND2)  // returns either 0 or 4 PIND2
#define ENABLE_INPUT_INTERRUPT		GICR |= (0b00000001 << INT0) // 6
#define DISABLE_INPUT_INTERRUPT		GICR &= (0b11111110 << INT0)	// 6

#define CLEAR_TIMER0_OVERFLOW_FLAG	TIFR |= 1 << TOV0
#define ENABLE_TIMER0_OVERFLOW_INT	TIMSK |= 0b00000001 << TOIE0	// 0
#define DISABL_TIMER0_OVERFLOW_INT	TIMSK &= 0b11111110 << TOIE0	// 0
#define ENABLE_TIMER0_COMP_INT		TIMSK |= (0b00000001 << OCIE0)		// 1
#define DISABLE_TIMER0_COMP_INT		TIMSK &= (0b11111110 << OCIE0)		// 1
#define TIMER0_COUNT_REG			TCNT0
#define TIMER0_COMP_REG				OCR0

#define NUMBER_OF_MEASURINGS		24
int meaurering_count; // measurements

int half_bit_time;
int three_quarter_bit_time;

IR_TRANSMISION_DATA_S ir_data;



bool first_start_bit;

void copyData();

void ir_init()
{

	DDRA = 0xFF;
	PORTA = 0xFF;
	
	//Setup interrupt	
	if(INPUT_INTERRUPT == INT0)
	{
		//Selects INT0 interrupt as falling edge
		MCUCR |= 0 << 0;
		MCUCR |= 1 << 1;
	}
	
	//Enable interrupt
	ENABLE_INPUT_INTERRUPT;
	first_start_bit = true;
	
	//Setup Timer0
	//Prescaler 64
	// Normal mode
	// Normal compare mode
	TCCR0 = 0b00000011;
	
#ifdef LED_DEBUG
	initLEDport(led);
	led++;
	writeLEDpattern(led);
	led = 0;
#endif
}

void ir_receive(IR_TRANSMISION_DATA_S* _ir_data)
{
	//Tjek for invalid data first
	*_ir_data = ir_data;
}

//Input interrupt
ISR (INT0_vect)
{
	int bit_time;
	if(first_start_bit==true)
	{
		TIMER0_COUNT_REG = 0;
		CLEAR_TIMER0_OVERFLOW_FLAG;
		ENABLE_TIMER0_OVERFLOW_INT;
		first_start_bit = false;
	}
	else{
		bit_time = TIMER0_COUNT_REG;
		TIMER0_COUNT_REG = 0;
		DISABL_TIMER0_OVERFLOW_INT;
		half_bit_time = bit_time/2;
		three_quarter_bit_time = (bit_time/4.0*3.0);

		TIMER0_COMP_REG = three_quarter_bit_time;
		meaurering_count = 0;
		ENABLE_TIMER0_COMP_INT;
		DISABLE_INPUT_INTERRUPT;
		first_start_bit = true;
	}
}

ISR (TIMER0_COMP_vect)
{
	PORTA = 0x00;
	
	TIMER0_COUNT_REG = 0;
	TIMER0_COMP_REG = half_bit_time;
	
	// 48 is to make it 0 or 1 in asci
	inputs[meaurering_count++] = INPUT_PIN+48;

	if(meaurering_count > NUMBER_OF_MEASURINGS-1)
	{
		meaurering_count = 0;
		ENABLE_INPUT_INTERRUPT;
		DISABLE_TIMER0_COMP_INT;
		copyData();
		ir_receive_input(inputs);
	}

	PORTA = 0xFF;
}

ISR (TIMER0_OVF_vect)
{
	if (*ir_error_msg != NULL)
	{
		ir_error_msg("\n\rTime out\n\r");
	}
	first_start_bit = true;
	DISABL_TIMER0_OVERFLOW_INT;	
}

void copyData()
{
	int i;
	for(i = 0; i <NUMBER_OF_MEASURINGS; i++)
	{
		inputData[i] = inputs[i];
	}
}