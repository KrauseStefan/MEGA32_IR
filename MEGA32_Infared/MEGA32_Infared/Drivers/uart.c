/**************************************************
* "uart.c":                                       *
* Implementation file for the Mega32 UART driver. *
*  STK500 setup:                                  *
*  Header "RS232 spare" connected to RXD/TXD:     *
*  RXD = PORTD, bit0                              *
*  TXD = PORTD, bit1                              *
*                                                 *
* Henning Hargaard, 1/11 2011                     *
***************************************************/
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include "uart.h"

// Constants
#define XTAL 3686400  

const char STR_BUFFER_SIZE = 255;
char *strBuffer[255];
static char inputNum = 0;
static char sendNum = 0;
static char sendCharCount = 0;

void USART_TX_Routine(char val){

    // Send the character pointed to by "String"
    SendChar(val);
    // Advance the pointer one step
    sendCharCount++;
}

ISR(USART_TXC_vect){
  sei();
  char val = strBuffer[sendNum][sendCharCount];
   
  if(val != 0)
  {
	  USART_TX_Routine(val);      
  }else{
    SendChar(0);
	sendCharCount = 0;
	free(strBuffer[sendNum]);
	sendNum++;
	if(sendNum == inputNum)
		UCSRB &= ~(1 << TXCIE); // disable tx interrupts
  }	
};

/*************************************************************************
USART initialization.
    Asynchronous mode.
	RX and TX enabled.
	No interrupts enabled.
	Number of Stop Bits = 1.
	No Parity.
	Baud rate = Parameter.
	Data bits = Parameter.
Parameters:
	BaudRate: Wanted Baud Rate.
	Databits: Wanted number of Data Bits.
*************************************************************************/
void InitUART(unsigned long BaudRate, unsigned char DataBit)
{
  unsigned int TempUBRR;

  if ((BaudRate >= 110) && (BaudRate <= 115200) && (DataBit >=5) && (DataBit <= 8))
  { 
    // "Normal" clock, no multiprocessor mode (= default)
    UCSRA = 0b00100000;
    // Interrupts disabled
    // Receiver enabled
    // Transmitter enabled
    // No 9 bit operation
    UCSRB = 0b00011000;	
    // Asynchronous operation, 1 stop bit, no parity
    // Bit7 always has to be 1
    // Bit 2 and bit 1 controls the number of databits
    UCSRC = 0b10000000 | (DataBit-5)<<1;
    // Set Baud Rate according to the parameter BaudRate:
    // Select Baud Rate (first store "UBRRH--UBRRL" in local 16-bit variable,
    //                   then write the two 8-bit registers separately):
    TempUBRR = XTAL/(16*BaudRate) - 1;
    // Write upper part of UBRR
    UBRRH = TempUBRR >> 8;
    // Write lower part of UBRR
    UBRRL = TempUBRR;
  }  
}

/*************************************************************************
  Returns 0 (FALSE), if the UART has NOT received a new character.
  Returns value <> 0 (TRUE), if the UART HAS received a new character.
*************************************************************************/
unsigned char CharReady()
{
   return UCSRA & (1<<RXC);
}

/*************************************************************************
Awaits new character received.
Then this character is returned.
Blocking call
*************************************************************************/
char ReadChar()
{
  // Wait for new character received
  while ( (UCSRA & (1<<RXC)) == 0 )
  {}                        
  // Then return it
  return UDR;
}

/*************************************************************************
Awaits transmitter-register ready.
Then it send the character.
Parameter :
	Ch : Character for sending. 
*************************************************************************/
void SendChar(char Ch)
{
  // if transmitter register full (ready for new character)  
    // Wait for transmitter register empty (ready for new character)
  while ( (UCSRA & (1<<UDRE)) == 0 ){}

  // Then send the character
  UDR = Ch;
}

/*************************************************************************
Sends 0-terminated string.
Parameter:
   String: Pointer to the string. 
   int: Number of characters excluding 0 termination. (can be set to -1 if needed.)
*************************************************************************/
void SendString(char* String, int length)
{ 
  if((inputNum+1) == sendNum )
	return;
	
  if(length <= 0){
	  length = strlen(String);
  }
    
  strBuffer[inputNum] = malloc(length+1);
  strcpy(strBuffer[inputNum], String);
	
  if(!(UCSRB & (1 << TXCIE))){
    UCSRB |= (1 << TXCIE); // enable TX interrupt.    
	USART_TX_Routine(strBuffer[inputNum][0]);
  }  
  
  inputNum++;
}

/*************************************************************************
Converts the integer "Number" to an ASCII string - and then sends this
string using the USART.
Makes use of the C standard library <stdlib.h>.
Parameter:
      Number: The integer to be converted and send. 
*************************************************************************/
void SendInteger(int Number)
{
  char array[7];
  // Convert the integer to an ASCII string (array), radix = 10 
  itoa(Number, array, 10);
  // - then send the string
  SendString(array, -1);
}

/**************************************************/
