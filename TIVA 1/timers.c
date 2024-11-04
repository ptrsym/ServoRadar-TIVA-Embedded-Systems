
///*****

// MAKE SURE TO COMMENT OUT THE TIMER HANDLERS USED IN SSI ON THE RESPECTIVE TIVAS IN THIS FILE

// TIVA 1 = COMMENT OUT TIMER7A HANDLER
// TIVA 2 = COMMENT OUT TIMER6A HANDLER

///*******

#include <TM4C129.h>
#include "my_lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define DEBOUNCE_DELAY_LOAD 0xC3500  // 20ms       	//32bit
#define DEBOUNCE_TIMER (1<<6)
extern volatile bool debounceFlag;

#define SENSOR_PULSE_INTERVAL 0x98967F  // 250ms     //32bit 
#define PULSE_TIMER (1<<7)
extern volatile bool autoPulse;


void Setup_DebounceTimer (void) {

	SYSCTL->RCGCTIMER |= DEBOUNCE_TIMER; 
	while ((SYSCTL->PRTIMER & DEBOUNCE_TIMER) == 0) {__ASM("nop");}
	
	//disable timer
	TIMER6->CTL &= ~(1<<0);
	
	// set 16/32 bit timer in CFG, CFG = 4 for 16 bit, CFG = 0 for 32 bit
	TIMER6->CFG = 0x0;
	
	//configure mode 0x1 = one shot 0x2 = periodic
	TIMER6->TAMR = 0x1;
	
	//load start value into load register 40Mhz system clock = 25ns period  | 50 ms 1E847F |  0xC3500 20ms |
	TIMER6->TAILR = DEBOUNCE_DELAY_LOAD;
	
	//load prescale value into TIMER6->TAPR not needed as 32-bit
	
	//write 1 to TATOCINT of TIMER6->ICR to clear timeout flag
	TIMER6->ICR |= (1<<0);
	
	//Set TAEN bit in TIMER6->CTL to start timer TIMER6->CTL |= (1<<0);

	//interrupts
	__disable_irq();
	TIMER6->IMR |= (1<<0);     // turn on interrupts for TATOINT
	NVIC->IP[98] = 0;					// 98
	NVIC->ISER[3] |= (1<<2);   // int(INTNUM/32); bit = (INTNUM%32)
	__enable_irq();	
	
}

//TIVA 1
void startDebounce(void) {
	TIMER6->CTL |= (1<<0);
	debounceFlag = true;
}

//TIVA 1
void TIMER6A_Handler(void) {             //CHECK comment out on tiva 2
	debounceFlag = false;
	TIMER6->CTL &= ~(1<<0);
	TIMER6->ICR |= (1<<0);
}


