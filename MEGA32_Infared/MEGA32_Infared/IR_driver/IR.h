/*
 * IR.c
 *
 * Created: 17-02-2012 14:08:08
 *  Author: Vinnie Juul
 */ 
#ifndef IR_H
#define IR_H

#include <stdbool.h>

char inputData[24];
char inputs[24];

typedef struct  
{
	char adr; // 5 bit
	char cmd; // 6 bit
	bool hold_bit;
}IR_TRANSMISION_DATA_S;

void ir_init();

// one single char not an array
void ir_receive(IR_TRANSMISION_DATA_S* ir_data);
void (*ir_receive_event)(IR_TRANSMISION_DATA_S ir_data);
void (*ir_error_msg)(char* message);
void (*ir_receive_input)(char* input);

#endif