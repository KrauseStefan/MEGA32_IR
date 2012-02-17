/*
 * IR.c
 *
 * Created: 17-02-2012 14:08:08
 *  Author: Vinnie Juul
 */ 

#include <stdbool.h>

void ir_init();

// one single char not an array
void ir_receive(char* adr, char* cmd, bool hold_bit);
void (ir_receive_event*)(char adr, char cmd, bool hold_bit);
