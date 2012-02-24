/* 
  File name : "led.h"
  
  Header file for STK500 LED panel.
  The LED panel is connected to ATMega16 Port C
  
  Author : Henning Hargaard
  Date : 11.9.2008
*/

void initLEDport(unsigned char init_pattern);
void writeLEDpattern(unsigned char pattern);
void turnOnLED(unsigned char led_nr);
void turnOffLED(unsigned char led_nr);
void toggleLED(unsigned char led_nr);
unsigned char LEDon(unsigned char led_nr);
unsigned char LEDstatus();
