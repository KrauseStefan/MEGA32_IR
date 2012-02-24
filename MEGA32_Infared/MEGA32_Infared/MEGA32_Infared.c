/*
 * MEGA32_Infared.c
 *
 * Created: 17-02-2012 13:18:03
 *  Author: Vinnie Juul
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "IR_driver/IR.h"
#include "Drivers/uart.h"

void ErrorData(char* message);
void input_handler(IR_TRANSMISION_DATA_S ir_data);
void UartOutput(IR_TRANSMISION_DATA_S ir_data);

int main(void)
{
	DDRD = 0b00000010;
	ir_init();
	InitUART(9600,8);
	ir_receive_event = &input_handler;
	ir_error_msg = &ErrorData;
	sei(); // enable global interrupt
	SendString("IR ready!\n");
    while(1)
    {
         
    }
	cli(); // disable global interrupt
}

void ErrorData(char* message)
{
	SendString(message);
}

void input_handler(IR_TRANSMISION_DATA_S ir_data)
{
	UartOutput(ir_data);
}

void UartOutput(IR_TRANSMISION_DATA_S ir_data)
{
	SendString("\nAdr: ");
	SendChar(ir_data.adr);
	SendString("\nCommand: ");
	SendChar(ir_data.cmd);
	SendString("\nBit hold: ");
	if(ir_data.hold_bit)
	{
		SendString("Button has not been hold\n");
	}
	else
	{
		SendString("Button has been hold\n");
	}
	
}
