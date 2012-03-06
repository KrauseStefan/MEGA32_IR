/*
 * MEGA32_Infared.c
 *
 * Created: 17-02-2012 13:18:03
 *  Author: Vinnie Juul
 *
 *
 * UART TXD --> PIND1
 * UART RXD --> PIND
 * 
 * IR Vcc (red) --> VTG PORTD
 * IR GND (blue/black) --> GND PORTD
 * IR data (green/purple) --> PIND2
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "IR_driver/IR.h"
#include "Drivers/uart.h"
#include "Drivers/RealTimeClock.h"

void ErrorData(char* message);
void input_handler(IR_TRANSMISION_DATA_S ir_data);
void UartOutput(IR_TRANSMISION_DATA_S ir_data);
void inputTest(char* input);

int main(void)
{
	char old_time = 0;
	char new_time;
	
	DDRD = 0b00000010;
	
	ir_init();
	InitUART(9600,8);
	LCDInit();
	RTCInit();
	
	SetClock(11, 5, 25, 5, 15, 35, 6);
	LCDClear();
	
	ir_receive_event = &input_handler;
	ir_error_msg = &ErrorData;
	ir_receive_input = &inputTest;
	sei(); // enable global interrupt
	//SendString("IR ready!\n\r");
    while(1)
    {
		new_time = Seconds1();
		if(old_time != new_time){
			LCDGotoXY(0,1);
			LCDDispInteger(Hours10());
			LCDDispInteger(Hours1());
			LCDDispString(":");
			LCDDispInteger(Minutes10());
			LCDDispInteger(Minutes1());
			LCDDispString(":");
			LCDDispInteger(Seconds10());
			LCDDispInteger(Seconds1());
			old_time = new_time;
		}	 
    }
	cli(); // disable global interrupt
}

void inputTest(char* input)
{
	//SendString("string length is: ", -1);
	//SendInteger(strlen(input));
	//SendString("\n", -1);
	//SendString(input, -1);
}

void ErrorData(char* message)
{
	SendString(message, -1);
}

void input_handler(IR_TRANSMISION_DATA_S ir_data)
{
	UartOutput(ir_data);
}

void UartOutput(IR_TRANSMISION_DATA_S ir_data)
{
	char *commandStr = malloc(50);
	translateCmd(ir_data.cmd, commandStr);
	
	SendString("\n\rAdr: ", -1);
	SendInteger(ir_data.adr);
	SendString("\n\rCommand: ", -1);
	SendString(commandStr, -1);
	//SendInteger(ir_data.cmd);
	SendString("\n\rBit hold: ", -1);
	if(ir_data.hold_bit == false)
	{
		SendString("Button has not been hold\n\r", -1);
	}
	else
	{
		SendString("Button has been hold\n\r", -1);
	}
	
	free(commandStr);
}
