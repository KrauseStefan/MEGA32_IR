/* 
  File name : "led.c"
  
  Implementation file for STK500 LED panel.
  The LED panel is connected to ATMega16 Port C
  
  Author : Henning Hargaard
  Date : 16.9.2008
*/

//#include <mega32.h>
#include <avr/io.h>
#include "led.h"
#define MAX_LED_NR 7
#define LED_DDR DDRA
#define LED_PORT PORTA

void initLEDport(unsigned char init_pattern)
{
  // Sæt alle PORTC's ben til at være udgange 
  LED_DDR = 0b11111111;
  // Hent parameteren, vend alle bit, og skriv til lysdioderne
  // Bittene skal vendes, da HW er indrettet, 
  // så et 0 vil tænde en lysdiode
  LED_PORT = ~init_pattern;     
}

void writeLEDpattern(unsigned char pattern)
{
  // Hent parameteren, vend alle bit, og skriv til lysdioderne
  // Bittene skal vendes, da HW er indrettet, 
  // så et 0 vil tænde en lysdiode
  LED_PORT = ~pattern;   
}

void turnOnLED(unsigned char led_nr)
{
// Lokal variabel
unsigned char mask;
  // Vi skal kun lave noget, hvis led_nr < 8
  if (led_nr <= MAX_LED_NR)
  {
    // Dan maske på basis af parameteren (led_nr)
    mask = ~(0b00000001 << led_nr);
    // Tænd den aktuelle lysdiode (de andre ændres ikke)
    LED_PORT = LED_PORT & mask;
  }   
}

void turnOffLED(unsigned char led_nr)
{
// Lokal variabel
unsigned char mask;
  // Vi skal kun lave noget, hvis led_nr < 8
  if (led_nr <= MAX_LED_NR)
  {
    // Dan maske på basis af parameteren (led_nr)
    mask = 0b00000001 << led_nr;
    // Sluk den aktuelle lysdiode (de andre ændres ikke)
    LED_PORT = LED_PORT | mask;
  }  
}

void toggleLED(unsigned char led_nr)
{
// Lokal variabel
unsigned char mask;
  // Vi skal kun lave noget, hvis led_nr < 8
  if (led_nr <= MAX_LED_NR)
  {
    // Dan maske på basis af parameteren (led_nr)
    mask = 0b00000001 << led_nr;
    // Toggle den aktuelle lysdiode (de andre ændres ikke)
    LED_PORT = LED_PORT ^ mask;
  }  
}

unsigned char LEDon(unsigned char led_nr)
{
// Lokal variabel
unsigned char mask;
  // Vi skal kun lave noget, hvis led_nr < 8
  if (led_nr <= MAX_LED_NR)
  {
    // Dan maske på basis af parameteren (led_nr)
    mask = 0b00000001 << led_nr;
    return (~LED_PORT & mask);
  }
  else
    return 0;   
} 

unsigned char LEDstatus()
{
  return (~LED_PORT);
}
