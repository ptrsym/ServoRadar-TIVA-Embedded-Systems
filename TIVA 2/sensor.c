//TIVA 2


#include "ES.h"
#include <TM4C129.h>
#include "my_lib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define SENSOR_PULSE_TIMER (1<<3)     // port A pin 6 timer A
#define SENSOR_INPUT_TIMER (1<<5)     // Port B pin 2 timer5A&B
#define SENSOR_PULSE_PIN (1<<6)       // currently port A pin 6 timer3A
#define PULSE_LOAD 399                //40Mhz Sys, 10Microsecond Target Period,  10micro/25ns = 400 
#define MIN_RETURN_PERIOD 100       //microseconds
#define MAX_RETURN_PERIOD 25000
#define MAX_CALIBRATED_DISTANCE 100 // in cm, placeholder

extern uint32_t last_pulse;
extern float last_distance;


	// Sensor Timer input capture Port B pin 2 Timer 5A&B
	
void Setup_SensorInput (void) {
	
	// Enable clock to GPIO Port B
	SYSCTL->RCGCGPIO |= (1<<1); 
	while ((SYSCTL->PRGPIO & (1<<1)) == 0) {__ASM("nop");} 
	
	SYSCTL->RCGCTIMER |= SENSOR_INPUT_TIMER;
	while ((SYSCTL->PRTIMER & SENSOR_INPUT_TIMER) == 0) {__ASM("nop");}
	
	
	//config port
	GPIOB_AHB->AFSEL |= (1<<2); 		 // only on input capture pin
	GPIOB_AHB->PCTL &=~ 0xF00;    	// clear pin 2
	GPIOB_AHB->PCTL |= 0x300;     	// set to timer
	GPIOB_AHB->DIR &= ~(1<<2); 			//ensure input on input capture pin
	GPIOB_AHB->DEN |= (1<<2);
	
	
	// config timer 5A (input capture)
	
	TIMER5->CTL &= ~((1<<0) | (1<<8));   //turn off timer A and B
	TIMER5->CFG = 0x4;         					// 16 bit mode
	

	TIMER5->TAMR &= ~0x3;  // set input capture 
	TIMER5->TAMR |= 0x3; 
	TIMER5->TBMR &= ~0x3;
	TIMER5->TBMR |= 0x3; 	
	
	TIMER5->TAMR &= ~(1<<3);  // set capture 
	TIMER5->TBMR &= ~(1<<3);
	
	TIMER5->TAMR |= (1<<3);  // set edge time
	TIMER5->TBMR |= (1<<3);
	
	TIMER5->TAMR &= ~(1<<4);  // set count down 
	TIMER5->TBMR &= ~(1<<4);
	
	//EVENT
	
	TIMER5->CTL &= ~((1<<2) | (1<<3));   //clear EVENT
	TIMER5->CTL &= ~((1<<10) | (1<<11));   
	
	TIMER5->CTL &= ~((1<<2) | (1<<3));   //set positive edge timer A
	TIMER5->CTL |= (1<<10);   //set negative edge timer B
	
	
	// set load and prescalar
	// 1microsecond / 25 ns = 40 ticks = 1 200 000 = needs prescaler
	// setting prescalar of 40 = 40Mhz  1.00 microseconds per tick
	
	
	TIMER5->TAILR = 0xFFFF;   // set max load
	TIMER5->TBILR = 0xFFFF;	
	
	TIMER5->TAPR =  40;      // set prescale 40 for 1 microsecond resolution
	TIMER5->TBPR =  40;
	
	TIMER5->ICR |= ((1<<2) | (1<<10));  // clear interrupt events
	
	
	//INTERRUPTS
	
	__disable_irq();
	
	TIMER5->IMR |= ((1<<2) | (1<<10)) ;  // enable interrupts for CAEIM and CBEIM
	NVIC->IP[65] = 0;
	NVIC->IP[66] = 0;
	NVIC->ISER[2] |= (1<<1);
	NVIC->ISER[2] |= (1<<2);
	
	__enable_irq();	
	
	
	TIMER5->CTL |= (1<<0); // turn on timer A to begin searching for input
		
}


//SENSOR PULSE TRIGGER

void Setup_SensorOutput (void) {
	
		// Enable clock to GPIO Port A
		SYSCTL->RCGCGPIO |= (1<<0); 
		while ((SYSCTL->PRGPIO & (1<<0)) == 0) {__ASM("nop");} 
		
		SYSCTL->RCGCTIMER |= SENSOR_PULSE_TIMER;
		while ((SYSCTL->PRTIMER & SENSOR_PULSE_TIMER) == 0) {__ASM("nop");}
		
		//config port
		GPIOA_AHB->DIR |= (1<<6); 			//ensure output on pulse pin
		GPIOA_AHB->DEN |= (1<<6);
		
		// config timer 3A (signal pulse)
		TIMER3->CTL &= ~(1<<0);   //turn off timer A
		
		TIMER3->CFG &= 0x3;      // 32 bit mode
		
		TIMER3->TAMR &= ~0x3;  // set one shot
		TIMER3->TAMR |= 0x1; 

		
		// pulse is 10microseconds period clock is 40MHz = 10microseconds / 25ns = 400 ticks
		
		TIMER3->TAMR &= ~(1<<4);  // set count down 
		
		TIMER3->TAILR = PULSE_LOAD;   // load for 10microseconds
		
		TIMER3->ICR |= (1<<0);  //clear flag
		
		
		//interrupt
		
		__disable_irq();

		TIMER3->IMR |= (1<<0) ;  // enable interrupts for TATOIM
		NVIC->IP[35] = 0;
		NVIC->ISER[1] |= (1<<3);
		
		__enable_irq();	
	
}


//rising edge
void TIMER5A_Handler(void) {               //input capture
	
	uint32_t status = TIMER5->MIS;
	
	if ((status & (1<<2)) == (1<<2)) {
			TIMER5->CTL |= (1<<8);          // start timer B
			TIMER5->ICR |= (1<<2);         //clear timer A's event flag
	}
	
}

//falling edge
void TIMER5B_Handler(void) {                  //input capture
	
	uint32_t status = TIMER5->MIS;
	uint32_t start = 0xFFFF;
	uint32_t end = 0;
	
	if ((status & (1<<10)) == (1<<10)) {
			TIMER5->CTL &= ~(1<<8);          		// turn off timer B
			end = TIMER5->TBR;									// get time of falling edge
			calc_pulse_time(start, end, &last_pulse);
		  pulse_to_distance(&last_pulse, &last_distance);
			TIMER5->TBILR = 0xFFFF; 						// reset the count		
			TIMER5->ICR |= (1<<10);         //clear timer B event interrupt flag
	}

}


void TIMER3A_Handler(void) {                //CHECK comment out when implementing tiva 1
	
		uint32_t status = TIMER3->MIS;
	
	if ((status & (1<<0)) == (1<<0)) {
		  GPIOA_AHB->DATA &= ~SENSOR_PULSE_PIN;  //toggle output pin
			TIMER3->CTL &= ~(1<<0);
			TIMER3->ICR |= (1<<0);
	}
	
}


//general functions

void calc_pulse_time(uint32_t start, uint32_t end, uint32_t *last_pulse) {
	
	uint32_t pulse_width = start - end; // calculate the pulse width
	*last_pulse = pulse_width;          // update the value for mapping
}


void pulse_sensor (void) {
	//set output pin high
	GPIOA_AHB->DATA |= SENSOR_PULSE_PIN;
	//ensure reload
	TIMER3->TAILR = PULSE_LOAD;
	//start timer
	TIMER3->CTL |= (1<<0);
}


	// map pulse width to distance value
	// update global distance value
void pulse_to_distance(uint32_t *last_pulse, float *last_distance) {                //UPDATE
			
	if (*last_pulse < MIN_RETURN_PERIOD) {
		*last_distance = 0.0;
		return;
	} else if  (*last_pulse > MAX_RETURN_PERIOD) {
		*last_distance = 404;
		return;
	}
		float pulse_width_percentage = (float)(*last_pulse - MIN_RETURN_PERIOD) / (MAX_RETURN_PERIOD - MIN_RETURN_PERIOD); // convert the pulse with to a proportional value to the max range
		*last_distance = pulse_width_percentage * MAX_CALIBRATED_DISTANCE;      //multiply by max distance to map to a value
	
		// refresh LCD map
}









