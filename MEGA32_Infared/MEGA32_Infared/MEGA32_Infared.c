/*
 * MEGA32_Infared.c
 *
 * Created: 17-02-2012 13:18:03
 *  Author: Vinnie Juul
 */ 

#include <avr/io.h>
#include "IR_driver/IR.h"
#include "Drivers/uart.h"

int main(void)
{
	ir_init();
	InitUART(9600,8);
    while(1)
    {
         
    }
}