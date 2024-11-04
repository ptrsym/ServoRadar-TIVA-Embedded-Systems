//TIVA 1 SHARES SETUP WITH SERVO SINCE NOT ON SAMEPORT
// SHARES TIMER 3A SETUP WITH SENSOR NOT ON SAME TIVA


#include "ES.h"
#include <TM4C129.h>
#include "my_lib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define PIEZO_PWM_PORT (1<<5)
#define PIEZO_PWM_PIN (1<<1)
#define PWM_MODULE (1<<0)
#define BUZZER_LOAD_VALUE 39999 /// 40000 ticks is 1ms period  (1ms/25ns)
#define BUZZER_MATCH_VALUE 19999  // 50% duty cycle is match at count 20000
#define BUZZER_FREQ 1000      // 1Khz       period = 1/1000 = 1ms   1ms/25ns

#define PIEZO_TIMER (1<<3)
#define BUZZ_DURATION 7999999 // 200ms buzz duration


//duty cycle 50%

void Setup_Piezo_PWM(void) {
	
	//Enable clock to GPIO Port F
	SYSCTL->RCGCGPIO |= PIEZO_PWM_PORT; 
	while ((SYSCTL->PRGPIO & (PIEZO_PWM_PORT)) == 0) {__ASM("nop");} 
	
	//Enable AFSEL for PF1 PWM
	GPIOF_AHB->AFSEL |= PIEZO_PWM_PIN;
	GPIOF_AHB->PCTL &=~ 0xF0;
	GPIOF_AHB->PCTL |= 0x60;
	GPIOF_AHB->DEN |= PIEZO_PWM_PIN;
	

	SYSCTL->RCGCPWM |= PWM_MODULE; // Module 0
	while ((SYSCTL->PRPWM & PWM_MODULE) == 0) {__ASM("nop");}
	
	// Set Clock & Prescale 20ms Target  20ms/25ns = 800 000 > 65535, need prescalar. 
	// select /16, 40Mhz/16 = 2.5, 1/2.5Mhz = 400ns, 20ms/400ns = 50000, load 49999
	
	//set clock use prescalar
	PWM0->CC &= ~(1<<8); // use system clock 

	// Disable & mode
	PWM0->_0_CTL &= ~(1<<1);   // count down to 0
	PWM0->_0_CTL &= ~(1<<0);   // turn off
	
	//configure PWM gen number = _0_ letter = B
	PWM0->_0_GENB |= ((0x2<<10) | (0x3<<2));       
	PWM0->_0_LOAD = BUZZER_LOAD_VALUE;
	PWM0->_0_CMPB = BUZZER_MATCH_VALUE;  
	
	PWM0->_0_CTL |= (1<<0);   //enable
	
	PWM0->ENABLE |= (1<<0);   //globally enable module 0
	
}

void Setup_Piezo_TIMER(void) {
		SYSCTL->RCGCTIMER |= PIEZO_TIMER;
		while ((SYSCTL->PRTIMER & PIEZO_TIMER) == 0) {__ASM("nop");}
		
// config timer 3A (signal pulse)
		TIMER3->CTL &= ~(1<<0);   //turn off timer A
		
		TIMER3->CFG &= 0x3;      // 32 bit mode
		
		TIMER3->TAMR &= ~0x3;  // set one shot
		TIMER3->TAMR |= 0x1; 

		
		// buzz duration is 200ms  clock is 40MHz = 200ms / 25ns = 8000000
		
		TIMER3->TAMR &= ~(1<<4);  // set count down 
		
		TIMER3->TAILR = BUZZ_DURATION;   // load for 200ms
		
		TIMER3->ICR |= (1<<0);  //clear flag
				
		//interrupt
		__disable_irq();

		TIMER3->IMR |= (1<<0) ;  // enable interrupts for TATOIM
		NVIC->IP[35] = 2;
		NVIC->ISER[1] |= (1<<3);
		
		__enable_irq();	
	
}		

void trigger_piezo(void) {
	TIMER3->TAILR = BUZZ_DURATION;
	PWM0->_0_CTL |= (1<<0);    // trigger buzzer by starting PWM waveform
	TIMER3->CTL |= (1<<0);     // start timer
}


void TIMER3A_Handler(void) {                //Triggers piezo sound duration   //CHECK comment out on TIVA2
		uint32_t status = TIMER3->MIS;
	
	if ((status & (1<<0)) == (1<<0)) {
		  PWM0->_0_CTL &= ~(1<<0);   // turn off pwm signal
			TIMER3->CTL &= ~(1<<0);    // turn off timer
			TIMER3->ICR |= (1<<0);     // reset interrupt status
	}
	
}



