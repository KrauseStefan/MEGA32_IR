/*
 * IR.c
 *
 * Created: 17-02-2012 14:39:21
 *  Author: Vinnie Juul
 */ 

#include "IR.h"
#include <avr/interrupt.h>
#include <limits.h>
#include <stdlib.h>


#define LED_DEBUG
#ifdef LED_DEBUG
	#include "../Drivers/led.h"
	unsigned char led = 0;
#endif

#define INPUT_INTERRUPT				INT0
#define INPUT_PIN					~(PIND & (1<<PIND2))  // returns either 0 or 4
#define ENABLE_INPUT_INTERRUPT		GICR |= (0b00000001 << 6)
#define DISABLE_INPUT_INTERRUPT		GICR &= (0b11111110 << 6)

#define ENABLE_TIMER0_COMP_INT		TIMSK |= (0b00000001 << 1)
#define DISABLE_TIMER0_COMP_INT		TIMSK &= (0b11111110 << 1)
#define TIMER0_COUNT_REG			TCNT0
#define TIMER0_COMP_REG				OCR0

#define NUMBER_OF_MEASURINGS		24

typedef enum{
	FIRST_MEASURMENTS,
	ZERO_BIT,
	ONE_BIT,
	RECEIVING_DONE
}INPUT_STATE;

int half_bit_time;
int three_quarter_bit_time;

IR_TRANSMISION_DATA_S ir_data;

bool zero_value_bit;
bool one_value_bit;

INPUT_STATE input_state;

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
	
#ifdef LED_DEBUG
	initLEDport(led);
	led++;
	writeLEDpattern(led);
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
	if(first_start_bit)
	{
		TIMER0_COMP_REG = 255;
		TIMER0_COUNT_REG = 0;
		ENABLE_TIMER0_COMP_INT;
		first_start_bit = !first_start_bit;
#ifdef LED_DEBUG
	led++;
	writeLEDpattern(led);
#endif
	}
	else{
		bit_time = TIMER0_COMP_REG;
		TIMER0_COUNT_REG = 0;
		half_bit_time = bit_time/2;
		three_quarter_bit_time = (float)(bit_time/4*3);
		TIMER0_COMP_REG = three_quarter_bit_time;
		meaurering_count = 0;
		input_state = FIRST_MEASURMENTS;
		DISABLE_INPUT_INTERRUPT;
	}
	// Measure time between the 2 start bits
	// calculate half and quarter bit time
	// Enable timer0 interrupt
	// start timer 0 to measure first time after 3 quarter bit time
}

ISR (TIMER0_COMP_vect)
{
	if(meaurering_count >= 0 && meaurering_count < NUMBER_OF_MEASURINGS)
	{
		TIMER0_COUNT_REG = 0;
		switch(input_state)
		{
			case FIRST_MEASURMENTS:
				TIMER0_COUNT_REG = half_bit_time;
				zero_value_bit = INPUT_PIN;
				meaurering_count++;
				input_state = ONE_BIT;
				break;
			case ZERO_BIT:
				zero_value_bit = INPUT_PIN;
				meaurering_count++;
				input_state = ONE_BIT;
				break;
			case ONE_BIT:
				one_value_bit = INPUT_PIN;
				if(zero_value_bit != one_value_bit)
				{
					int temp = meaurering_count/2;
					if(temp == 0)
					{
						ir_data.hold_bit = one_value_bit;
					}
					else if(temp <= 5 && temp > 0)
					{
						ir_data.adr |= one_value_bit << temp;
					}
					else if(temp <= 11 && temp > 0)
					{
						ir_data.cmd |= one_value_bit << (temp-5);
					}
					else
					{
						if (*ir_error_msg != NULL)
						{
							ir_error_msg("\nToo many bytes received\n");
						}
					}
				}
				else
				{
					if (*ir_error_msg != NULL)
					{
						ir_error_msg("\nBoth high/low bits\n");
					}
				}
				
				if(meaurering_count != NUMBER_OF_MEASURINGS-1)
				{
					meaurering_count++;
					input_state = ZERO_BIT;
				}
				else
				{
					meaurering_count = 0;
					input_state = RECEIVING_DONE;
				}					
				break;
			case RECEIVING_DONE:
				if(*ir_receive_event != NULL)
				{
					ir_receive_event(ir_data);
				}
				ENABLE_INPUT_INTERRUPT;
				DISABLE_TIMER0_COMP_INT;
				break;
			default:
				break;
		}
	}
}