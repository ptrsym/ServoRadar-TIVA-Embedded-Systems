//TIVA 1

#include "ES.h"
#include <TM4C129.h>
#include "my_lib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define SSI_GPIO (1<<0)   // PORT A
#define SSI_MODULE (1<<0)    //SSI0
#define SSI_PINS ((1<<2) | (1<<3) | (1<<4) | (1<<5))


#define SSI_SLAVE_TIMER (1<<6)
#define SSI_DR_UPDATE_INTERVAL 0x1E847F // 50ms load value

#define TIVA1_PACKET_SIZE 5
#define TIVA2_PACKET_SIZE 3


//sentpacket
extern volatile int16_t sendPacket_tiva2[TIVA2_PACKET_SIZE];
extern volatile int16_t receivePacket_tiva2[TIVA1_PACKET_SIZE];

//packet variables
volatile extern float last_distance;
volatile extern uint16_t sendStatus;

//receivepacket
extern volatile char mode; 
extern volatile int16_t min_angle;
extern volatile int16_t max_angle;
extern volatile int16_t target_angle;  

// input flags
extern volatile bool automaticMode;
extern volatile bool manualMode;
extern volatile bool exitProgram;

//automation
extern volatile bool autoPulse;    // start timer to pulse radar periodically

//page 1243 register map

//SLAVE
void Setup_Tiva2_SSI(void) {              //use SSI0 A2, A3, A4, A5 AFSEL = 0xF, SSIclk = 1MHz, SCR = 0, CPSDVSR = 40Mhz / 1Mhz  = 40
	
	SYSCTL->RCGCGPIO |= SSI_GPIO; 
	while ((SYSCTL->PRGPIO & (SSI_GPIO)) == 0) {__ASM("nop");} 
	
	//Enable pins a2, a3, a4, a5 for afsel SSI digital, PUR on
	
	GPIOA_AHB->DIR |= ((1<<2) | (1<<3) | (1<<4)); //clk fss TX
	GPIOA_AHB->DIR &= ~(1<<5);  //RX
	GPIOA_AHB->AFSEL |= SSI_PINS;
	GPIOA_AHB->PCTL &= ~0xFFFF00;
	GPIOA_AHB->PCTL |= 0xFFFF00;
	GPIOA_AHB->DEN |= SSI_PINS;
	GPIOA_AHB->PUR |= (1<<2); //set pullup for clk
	
	
	SYSCTL->RCGCSSI |= SSI_MODULE; 
	while ((SYSCTL->PRSSI & (SSI_MODULE)) == 0) {__ASM("nop");}


	SSI0->CR1 &= ~(1<<1);   	// disable SSE
	SSI0->CR1 |= (1<<2); 			// set slave
	SSI0->CPSR = 40; 					// set CPSDVSR = 40         //for 1mhz comm line
	SSI0->CR0 &= ~(0xFF00);  	// set SRC = 0
	
	SSI0->CR0 &= ~(1<<7); // set SPH = 0 AND SPO = 0 for mode 0, FRF = 0 for freescale, DSS = 0xF for 16-bit data.
	SSI0->CR0 &= ~(1<<6);
	SSI0->CR0 &= ~(3<<4);
	SSI0->CR0 |= (0xF<<0);
	

	//setup interrupts for handling receive
	__disable_irq();
	SSI0->IM |= (1<<2); // IRQ for SSI0 RXIM
	NVIC->IP[7] = 0;
	NVIC->ISER[0] |= (1<<7);
	__enable_irq();	
	
	SSI0->CR1 |= (1<<1); //enable SSE bit

}

//use timer 6A on tiva 2 as it is free here

void Setup_Tiva2_SSI_Slave_Timer(void) {

	SYSCTL->RCGCTIMER |= SSI_SLAVE_TIMER; 
	while ((SYSCTL->PRTIMER & SSI_SLAVE_TIMER) == 0) {__ASM("nop");}
	
	//disable timer
	TIMER6->CTL &= ~(1<<0);
	
	// set 16/32 bit timer in CFG, CFG = 4 for 16 bit, CFG = 0 for 32 bit
	TIMER6->CFG = 0x0;
	
	//configure mode 0x1 = one shot 0x2 = periodic
	TIMER6->TAMR = 0x2;
	
	//load start value into load register 40Mhz system clock = 25ns period. 50ms load
	TIMER6->TAILR = SSI_DR_UPDATE_INTERVAL;
	
	//load prescale value into TIMER6->TAPR not needed as 32-bit
	
	//write 1 to TATOCINT of TIMER6->ICR to clear timeout flag
	TIMER6->ICR |= (1<<0);
	
	//Set TAEN bit in TIMER6->CTL to start timer TIMER6->CTL |= (1<<0);
	
	//interrupts
	__disable_irq();
	TIMER6->IMR |= (1<<0);     				// turn on interrupts for TATOINT
	NVIC->IP[98] = 0;									// 98
	NVIC->ISER[3] |= (1<<2);   				// int(INTNUM/32); bit = (INTNUM%32)
	__enable_irq();	
	
	
	TIMER6->CTL |= (1<<0); // turn on updates for preloading the packet
}


// make a packet to send to tiva 1
void makePacket_tiva2(int16_t status, float objectDistance) {
	
	int16_t sendDistance = (int16_t)objectDistance;
	
	sendPacket_tiva2[0] = status;
	sendPacket_tiva2[1] = sendDistance;
	
	int16_t sendChecksum = (status ^ sendDistance);	
	
	sendPacket_tiva2[2] = sendChecksum;
}


//unpack a received packet
void unpackPacket_tiva2(volatile int16_t receivedPacket[TIVA1_PACKET_SIZE]) {
	int16_t receivedChecksum = (receivedPacket[0] ^ receivedPacket[1] ^ receivedPacket[2] ^ receivedPacket[3]);          //CHECK potential logic assignment
	if (receivedChecksum != receivedPacket[4]) {
		sendStatus = 0; 				// error checksums dont match
		return;		
	}
	for (int index = 0; index < TIVA1_PACKET_SIZE-1; index++) {
	switch(index) {
		case 0:
			switch(receivedPacket[index]) {
				case'A':
						mode = 'A';
						manualMode = false;
						automaticMode = true;
						break;
				case'M':
						mode = 'M';
						automaticMode = false;
						manualMode = true;
						break;
				case'E':
						mode = 'E';
						exitProgram = true;
						reset_flags_tiva2();
						break;
				default:
					sendStatus = 1; 	// unrecognised mode detected error
					break;
				}
			break;
		case 1:
			min_angle = receivedPacket[index];
			break;
				
		case 2:
			max_angle = receivedPacket[index];
			break;
		
		case 3:
			target_angle = receivedPacket[index];
			break;
		
		default:
			sendStatus = 2;     // no packet detected error
			break;	
		
				}
			}
	sendStatus = 3; 		// status good - expected operation
		}
	
//TIVA 2																				//CHECK comment out implementing tiva 1
void TIMER6A_Handler(void) {
	uint32_t status = TIMER6->MIS;
	makePacket_tiva2(sendStatus, last_distance);  	// updates packet with recent variables
	int index = 0;
	
	if ((status & (1<<0)) == (1<<0)) {
		if (((SSI0->SR) & (1<<0)) == 0) {      				// if there is already data waiting to be sent dont load
			while (SSI0->SR & (1<<1)) {        			 		// else load the DR and fifo while it's not full    //could lockup the system potentially
				SSI0->DR = sendPacket_tiva2[index++];			
			}
		}
	}
	TIMER6->ICR |= (1<<0);
}


//Tiva 2	
void SSI0_Handler() {									//CHECK comment out implementing tiva 1
	static int index = 0;   												//remember the index as each frame is caught
	
	uint32_t status = SSI0->MIS;
	
	if ((status & (1<<2)) == (1<<2)) {
		 	int16_t frame = SSI0->DR;
		if (index < TIVA1_PACKET_SIZE) {
			receivePacket_tiva2[index++] = frame;     	//build the packet
		}
	}
	
		if (index == TIVA1_PACKET_SIZE) {          // when last frame is reached
			unpackPacket_tiva2(receivePacket_tiva2); // parse the packet and update flags
			index = 0;                           		 // reset for the next packet
	}		
	SSI0->ICR |= (1<<2); //clear interrupt
}


void reset_flags_tiva2(void) {
	automaticMode = false;
  manualMode = false;
}












