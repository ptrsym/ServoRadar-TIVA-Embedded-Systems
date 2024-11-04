//TIVA 1


#include <TM4C129.h>
#include "my_lib.h"
#include "ES.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define SSI_PINS ((1<<0) | (1<<1) | (1<<2) | (1<<3))
#define SSI_GPIO (1<<3)
#define SSI_MODULE (1<<2)

extern volatile float last_distance;

// use if no LCD
void display_distance(float last_distance) {
	if (last_distance == 0.0) {
		writeMessage("No object detected.\r\n");
		return;
	} else if (last_distance == 404) {
		writeMessage("No object detected or Timeout.\r\n");
		return;
	}
	ES_Uprintf(0, "Distance to Object: %.2f\r\n", last_distance);
}


void Setup_LCD_DISPLAY(void) {

//page 1243 register map
//MASTER
//use SSI2 D0, D1, D2, D3 AFSEL = 0xF, SSIclk = 1MHz, SCR = 0, CPSDVSR = 40Mhz clk / 2Mhz baud = 40
	SYSCTL->RCGCGPIO |= SSI_GPIO; 
	while ((SYSCTL->PRGPIO & (SSI_GPIO)) == 0) {__ASM("nop");} 
	
	//Enable pins a2, a3, a4, a5 for afsel SSI digital, PUR on
	
	GPIOD_AHB->DIR |= SSI_PINS;
	GPIOD_AHB->AFSEL |= (1<<1) | (1<<2) | (1<<3);
	GPIOD_AHB->PCTL &=~ 0xFFF0;
	GPIOD_AHB->PCTL |= 0xFFF0;
	GPIOD_AHB->DEN |= SSI_PINS;
	GPIOD_AHB->PUR |= (1<<3); //set pullup for clk
	
	
	SYSCTL->RCGCSSI |= SSI_MODULE; 
	while ((SYSCTL->PRSSI & (SSI_MODULE)) == 0) {__ASM("nop");}


	SSI0->CR1 &= ~(1<<1);   	// disable SSE
	SSI0->CR1 &= ~(1<<2); 		// set master
	SSI0->CPSR = 20; 					// set CPSDVSR = 20 for 2Mhz
	SSI0->CR0 &= ~(0xFF00);  	// set SRC = 0
	
	SSI0->CR0 |= (1<<7); // set SPH = 1 AND SPO = 1 for mode 3, FRF = 0 for freescale, DSS = 0xF for 16-bit data.
	SSI0->CR0 |= (1<<6);
	SSI0->CR0 &= ~(3<<4);
	SSI0->CR0 |= (0x7<<0);

//	//setup interrupts for handling receive
//	__disable_irq();
//	
//	SSI2->IM |= (1<<2); // IRQ for SSI2 RXIM not empty
//	
//	NVIC->IP[54] = 0;
//	
//	NVIC->ISER[1] |= (1<<22);

//	__enable_irq();	
	
	SSI0->CR1 |= (1<<1); //enable SSE bit

}

