//TIVA 1


#include <TM4C129.h>
#include "my_lib.h"
#include "ES.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define ADC_POT_TIMER (1<<4) //timer 4a
#define ADC_SAMPLE_INTERVAL 0x98967F //250ms
#define POTENTIOMETER_VOLTAGE 3.3
extern volatile int target_angle;

void Setup_Potentiometer_ADC (void) {
	
		SYSCTL->RCGCGPIO |= (1<<4);
		while ((SYSCTL->PRGPIO & (1<<4)) == 0) {__ASM("nop");}

		SYSCTL->RCGCADC |= (1<<0);
		while ((SYSCTL->PRADC & (1<<0)) == 0) {__ASM("nop");}
		
		///// PORT E PINS: 3 ADC0 AIN0 (POTENTIOMETER)
		GPIOE_AHB->AMSEL |= (1<<3);		
		GPIOE_AHB->AFSEL |= (1<<3);
		GPIOE_AHB->DEN &= ~(1<<3);
		
		//set sample rate
		ADC0->PP |= 0x1;  // sample rate 125ksps  check needed value
		
		//disable sequencer 3
		ADC0->ACTSS &= ~(1<<3);
		
		//set priority using 1 sequencer no need?
		//ADC0->SSPRI ;
		
		//EMUX set trigger type for sequencer 0x5 is timer
		ADC0->EMUX &= ~(0xF000);
		ADC0->EMUX |= (0x5000);
		
		//use mux3 for seq 3 set to 0 for AIN0
		ADC0->SSMUX3 = 0x0;
		
		//set to end of list and
		ADC0->SSCTL3 = ((1<<1) | (1<<2));
		
		//interrupt config
		__disable_irq();
	
		// enable interrupts for sequencer 3
		ADC0->IM |= (1<<3);
	
		NVIC->IP[14] = 1;
	
		NVIC->ISER[0] |= (1<<14);

		__enable_irq();	
		
		
		//write to ADC->ISC |= (1<<3); to clear 
		//read ((ADC->RIS & (1<<3)) == (1<<3)) to check if interrupt 
		
		//enable - turn on when manual servo mode is set
		//ADC0->ACTSS |= (1<<3);
}


void Setup_Potentiometer_Timer(void) {
	
	SYSCTL->RCGCTIMER |= ADC_POT_TIMER; 
	while ((SYSCTL->PRTIMER & ADC_POT_TIMER) == 0) {__ASM("nop");}
	
	//disable timer
	TIMER4->CTL &= ~(1<<0);
	
	// set 16/32 bit timer in CFG, CFG = 4 for 16 bit, CFG = 0 for 32 bit
	TIMER4->CFG = 0x0;
	
	//configure mode 0x1 = one shot 0x2 = periodic
	TIMER4->TAMR = 0x2;
	
	//load start value into load register 40Mhz system clock = 25ns period. 250ms sample interval  250ms / 25ns = 0x98967F
	TIMER4->TAILR = ADC_SAMPLE_INTERVAL;
	
	// configure for event trigger
	TIMER4->CTL |= (1<<5);
	
	// turn on timeout event for trigger pulse for ADC module
	TIMER4->ADCEV |= (1<<0); 
	
	//load prescale value into TIMER7->TAPR not needed as 32-bit
	
	//write 1 to TATOCINT of TIMER7->ICR to clear timeout flag
	TIMER4->ICR |= (1<<0);
	
	//TIMER4->CTL |= (1<<0); enable when manual mode on
}

void startADC(void) {
	
	ADC0->ACTSS |= (1<<3);  //turn on adc module
	TIMER4->CTL |= (1<<0);  // start sample event timer trigger
}

void stopADC(void) {

	ADC0->ACTSS &= ~(1<<3);
	TIMER4->CTL &= ~(1<<0);
}

// handle the sampled data. convert voltage to angle setting for manual servo
void ADC0SS0_Handler(){
	uint32_t adc_reading = ADC0->SSFIFO3;

	float voltage = (adc_reading / 4095) * POTENTIOMETER_VOLTAGE;      // 12 bit mapping 4095 range, 3.3V source
	target_angle = ((voltage/POTENTIOMETER_VOLTAGE) * 120) - 60;      // map the voltage to an angle between -60 and 60	
	
	ADC0->ISC |= (1<<3); //clear interrupt flag	
}
