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

#define LED_DEBUG
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
bool splitResult(char *rawInput, char * address, char *command, bool *heldDown);

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
	
	inputs[24] = 0;
	inputData[24] = 0;
	
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
	inputs[meaurering_count] = INPUT_PIN + 48;

	meaurering_count++;
	
	if(meaurering_count > NUMBER_OF_MEASURINGS - 1)
	{
		PORTB++;
		meaurering_count = 0;
		ENABLE_INPUT_INTERRUPT;
		DISABLE_TIMER0_COMP_INT;
		sei();
		
		PORTB++;
		splitResult(inputs, &ir_data.adr, &ir_data.cmd, &ir_data.hold_bit );
		ir_receive_event(ir_data);
		
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

bool splitResult(char *rawInput, char * address, char *command, bool *heldDown){
	const int start = 0;
	const int addressStart = 2;
	const int cmdStart = addressStart + 10;
	const int end = cmdStart + 12;
	static char lastHeldDown = '0';
	
	bool error = false;
	
	*address = 0;
	*command = 0;
	int i = 1;
	while(1){
		if(rawInput[i -1] == rawInput[i]){
			error = true;
		}
	
		if(i >= cmdStart && i < end){
			*command = (*command) * 2;
			if(rawInput[i] != '0'){
				*command += 1;
			}		

		}else if(i >= addressStart && i < cmdStart){
			*address = (*address) * 2;
			if(rawInput[i] != '0'){
				*address += 1;
			}
		
		}else if(i >= start && i < addressStart){
			if(rawInput[i] == lastHeldDown){
				*heldDown = true;
			}else{
				lastHeldDown = rawInput[i];
				*heldDown = false;
			}		
		}else
			break;
			
		i += 2;
	}
	
	return error;
}


void translateCmd(char cmd, char *output){
	if(cmd >= 54 && cmd <= 63){
		char number = ((signed int)cmd - 63) * -1;
//		char number = (signed int)cmd;
		sprintf(output, "%i", number);
//		output[0] = itoa((char)number, output, 10);		
		return;
	}
	
	
	switch(cmd){
		case 47:
			strcpy(output, "Vol up");
			break;
		case 46:
			strcpy(output, "Vol down");
			break;
		case 31:
			strcpy(output, "Channal up");
			break;
		case 30:
			strcpy(output, "Channal down");
			break;
		case 51:
			strcpy(output, "Power");
			break;
		case 50:
			strcpy(output, "Mute");
			break;
		case 49:
			strcpy(output, "PP");
			break;
		case 48:
			strcpy(output, "Plus in a frame");
			break;
		default:
			sprintf(output, "Undefined value: %i", cmd);		
			return;
	}
//	return output;
}
