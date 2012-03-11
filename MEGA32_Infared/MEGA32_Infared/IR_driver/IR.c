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
int measurering_count; // measurements

int half_bit_time;
int three_quarter_bit_time;

IR_TRANSMISION_DATA_S ir_data;

bool first_start_bit;

void copyData();
bool splitResult(char *rawInput, char * address, char *command, bool *heldDown);

void ir_init()
{	
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
		measurering_count = 0;
		ENABLE_TIMER0_COMP_INT;
		DISABLE_INPUT_INTERRUPT;
		first_start_bit = true;
	}
}

ISR (TIMER0_COMP_vect)
{	
	TIMER0_COUNT_REG = 0;
	TIMER0_COMP_REG = half_bit_time;
	
	// 48 is to make it 0 or 1 in asci
	inputs[measurering_count] = (!INPUT_PIN) + '0';

	measurering_count++;
	
	if(measurering_count > NUMBER_OF_MEASURINGS - 1)
	{
		measurering_count = 0;
		ENABLE_INPUT_INTERRUPT;
		DISABLE_TIMER0_COMP_INT;
		sei(); // re enable interrupts from here on the code is not considered high priority.
		
		splitResult(inputs, &ir_data.adr, &ir_data.cmd, &ir_data.hold_bit );
		ir_receive_event(ir_data);
		
		copyData();
		ir_receive_input(inputs);
	}
}

ISR (TIMER0_OVF_vect)
{
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
	int index = 1;
	while(1){
		
		//If previus bit was equal to current bit we have an error
		if(rawInput[index -1] == rawInput[index]){
			error = true;
		}
		
		// read Command bits
		if(index >= cmdStart && index < end){
			*command = (*command) << 1;
			if(rawInput[index] != '0'){
				*command |= 1;
			}		

		}else if(index >= addressStart && index < cmdStart){
			*address = (*address) << 1;
			if(rawInput[index] != '0'){
				*address |= 1;
			}
		
		}else if(index >= start && index < addressStart){
			if(rawInput[index] == lastHeldDown){
				*heldDown = true;
			}else{
				lastHeldDown = rawInput[index];
				*heldDown = false;
			}		
		}else
			break;
			
		index += 2;
	}
	
	return error;
}


void translateCmd(char cmd, char *output){
	if(cmd >= 0 && cmd <= 9){
		sprintf(output, "%i", cmd);
		return;
	}
	
	
	switch(cmd){
		case 16:
			strcpy(output, "Vol up");
			break;
		case 17:
			strcpy(output, "Vol down");
			break;
		case 32:
			strcpy(output, "Channal up");
			break;
		case 33:
			strcpy(output, "Channal down");
			break;
		case 12:
			strcpy(output, "Power");
			break;
		case 13:
			strcpy(output, "Mute");
			break;
		case 14:
			strcpy(output, "PP");
			break;
		case 15:
			strcpy(output, "Plus in a frame");
			break;
		default:
			sprintf(output, "Undefined value: %i", cmd);		
			return;
	}
//	return output;
}
