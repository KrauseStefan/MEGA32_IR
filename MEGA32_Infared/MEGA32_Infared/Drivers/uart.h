/**************************************
* "uart.h":                           *
* Header file for Mega32 UART driver. *

  Based on Henning Hargaards template,
  Implemented by Vinnie Juul and Stefan Krause-Kjær 
***************************************/ 
void InitUART(unsigned long BaudRate, unsigned char DataBit);
unsigned char CharReady();
char ReadChar();
void SendChar(char Ch);
void SendString(char* String, int length);
void SendInteger(int Number);

/**************************************/
