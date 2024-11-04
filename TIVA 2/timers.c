
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


// COMMENT OUT BELOW ON TIVA 1


//TIVA 2
void Setup_PeriodicPulseTimer (void) {

	SYSCTL->RCGCTIMER |= PULSE_TIMER; 
	while ((SYSCTL->PRTIMER & PULSE_TIMER) == 0) {__ASM("nop");}
	
	//disable timer
	TIMER7->CTL &= ~(1<<0);
	
	// set 16/32 bit timer in CFG, CFG = 4 for 16 bit, CFG = 0 for 32 bit
	TIMER7->CFG = 0x0;
	
	//configure mode 0x1 = one shot 0x2 = periodic
	TIMER7->TAMR = 0x2;
	
	//load start value into load register 40Mhz system clock = 25ns period. 250ms pulse interval  250ms / 25ns = 0x989680
	TIMER7->TAILR = SENSOR_PULSE_INTERVAL;
	
	//load prescale value into TIMER7->TAPR not needed as 32-bit
	
	//write 1 to TATOCINT of TIMER7->ICR to clear timeout flag
	TIMER7->ICR |= (1<<0);
	
	//Set TAEN bit in TIMER7->CTL to start timer TIMER7->CTL |= (1<<0);
	
	//interrupts
	
	__disable_irq();
	TIMER7->IMR |= (1<<0);     // turn on interrupts for TATOINT
	NVIC->IP[100] = 0;					// 100
	NVIC->ISER[3] |= (1<<4);   // int(INTNUM/32); bit = (INTNUM%32)
	__enable_irq();	

}

//TIVA 2
void startAutoPulse(void) {
	pulse_sensor();
	TIMER7->CTL |= (1<<0);
}


//TIVA 2
void stopAutoPulse(void) {
	TIMER7->CTL &= ~(1<<0);
	TIMER7->TAILR = SENSOR_PULSE_INTERVAL;	
}


//TIVA 2
void TIMER7A_Handler(void) {                //CHECK comment out on tiva 1
	
	uint32_t status = TIMER7->MIS;
	if ((status & (1<<0)) == (1<<0)) {
			pulse_sensor();
			TIMER7->ICR |= (1<<0);
	}
}



