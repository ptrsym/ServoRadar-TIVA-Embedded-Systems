//TIVA 1

#include "ES.h"
#include <TM4C129.h>
#include "my_lib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


#define SSI_MASTER_TIMER (1<<7)
#define SSI_TRANSMIT_INTERVAL 0xA7D8BF  //275ms

#define SSI_GPIO (1<<0)
#define SSI_MODULE (1<<0)
#define SSI_PINS ((1<<2) | (1<<3) | (1<<4) | (1<<5))

#define TIVA1_PACKET_SIZE 5
#define TIVA2_PACKET_SIZE 3

//packet
extern volatile int16_t sendPacket_tiva1[TIVA1_PACKET_SIZE];
extern volatile int16_t receivePacket_tiva1[TIVA2_PACKET_SIZE];

//packet variables
extern volatile char mode; 
extern volatile int16_t target_angle;  
extern int16_t max_angle;
extern int16_t min_angle;

// input flags
extern volatile bool automaticMode;
extern volatile bool manualMode;
extern volatile bool exitProgram;
extern volatile bool setMinAngle;
extern volatile bool setMaxAngle;

   
extern volatile int16_t receiveStatus;
extern volatile float last_distance;



//page 1243 register map
//MASTER
void Setup_Tiva1_SSI(void) {              //use SSI0 A2, A3, A4, A5 AFSEL = 0xF, SSIclk = 1MHz, SCR = 0, CPSDVSR = 40Mhz / 1Mhz  = 40
	
	SYSCTL->RCGCGPIO |= SSI_GPIO; 
	while ((SYSCTL->PRGPIO & (SSI_GPIO)) == 0) {__ASM("nop");} 
	
	//Enable pins a2, a3, a4, a5 for afsel SSI digital, PUR on
	
	GPIOA_AHB->DIR |= ((1<<2) | (1<<3) | (1<<4)); //clk fss TX
	GPIOA_AHB->DIR &= ~(1<<5);  //RX
	GPIOA_AHB->AFSEL |= SSI_PINS;
	GPIOA_AHB->PCTL &=~ 0xFFFF00;
	GPIOA_AHB->PCTL |= 0xFFFF00;
	GPIOA_AHB->DEN |= SSI_PINS;
	GPIOA_AHB->PUR |= (1<<2); //set pullup for clk
	
	
	SYSCTL->RCGCSSI |= SSI_MODULE; 
	while ((SYSCTL->PRSSI & (SSI_MODULE)) == 0) {__ASM("nop");}


	SSI0->CR1 &= ~(1<<1);   	// disable SSE
	SSI0->CR1 &= ~(1<<2); 		// set master
	SSI0->CPSR = 40; 					// set CPSDVSR = 40
	SSI0->CR0 &= ~(0xFF00);  	// set SRC = 0
	
	SSI0->CR0 &= ~(1<<7); // set SPH = 0 AND SPO = 0 for mode 0, FRF = 0 for freescale, DSS = 0xF for 16-bit data.
	SSI0->CR0 &= ~(1<<6);
	SSI0->CR0 &= ~(3<<4);
	SSI0->CR0 |= (0xF<<0);

	//setup interrupts for handling receive
	__disable_irq();
	
	SSI0->IM |= (1<<2); // IRQ for SSI0 RXIM not empty
	
	NVIC->IP[7] = 0;
	
	NVIC->ISER[0] |= (1<<7);

	__enable_irq();	
	
	
	SSI0->CR1 |= (1<<1); //enable SSE bit

}

void Setup_Tiva1_SSI_Master_Timer(void) {

	SYSCTL->RCGCTIMER |= SSI_MASTER_TIMER; 
	while ((SYSCTL->PRTIMER & SSI_MASTER_TIMER) == 0) {__ASM("nop");}
	
	//disable timer
	TIMER7->CTL &= ~(1<<0);
	
	// set 16/32 bit timer in CFG, CFG = 4 for 16 bit, CFG = 0 for 32 bit
	TIMER7->CFG = 0x0;
	
	//configure mode 0x1 = one shot 0x2 = periodic
	TIMER7->TAMR = 0x2;
	
	//load start value into load register 40Mhz system clock = 25ns period. 275ms transmit interval  275ms / 25ns = 0xA7D8BF
	TIMER7->TAILR = SSI_TRANSMIT_INTERVAL;
	
	//load prescale value into TIMER7->TAPR not needed as 32-bit
	
	//write 1 to TATOCINT of TIMER7->ICR to clear timeout flag
	TIMER7->ICR |= (1<<0);
	
	//Set TAEN bit in TIMER7->CTL to start timer TIMER7->CTL |= (1<<0);
	
	//interrupts
	
	__disable_irq();
	TIMER7->IMR |= (1<<0);     	// turn on interrupts for TATOINT
	NVIC->IP[100] = 0;					// 100
	NVIC->ISER[3] |= (1<<4);   	// int(INTNUM/32); bit = (INTNUM%32)
	__enable_irq();	

}

void makePacket_tiva1(char mode, int16_t min_angle, int16_t max_angle, int16_t target_angle) {
	sendPacket_tiva1[0] = mode;
	sendPacket_tiva1[1] = min_angle;
	sendPacket_tiva1[2] = max_angle;
	sendPacket_tiva1[3] = target_angle;
	
	int16_t sendChecksum = (mode ^ min_angle ^ max_angle ^ target_angle);
	
	sendPacket_tiva1[4] = sendChecksum;
}

//unpack a received packet
void unpackPacket_tiva1(volatile int16_t receivedPacket[TIVA2_PACKET_SIZE]) {
	int16_t receivedChecksum = (receivedPacket[0] ^ receivedPacket[1]);          //CHECK potential logic assignment
	if (receivedChecksum != receivedPacket[2]) {
		receiveStatus = 0; 				
		writeMessage("\r\nERROR: checksum mismatch\r\n");
		return;		
	}
	for (int index = 0; index < TIVA2_PACKET_SIZE-1; index++) {
	switch(index) {
		case 0:
			switch(receivedPacket[index]) {
				case 0:
						writeMessage("\r\nERROR: checksum mismatch on TIVA2 RECEIVE\r\n");		
						break;
				case 1:
						writeMessage("\r\nERROR: unrecognised mode detected on TIVA2\r\n");
						break;
				case 2:
					writeMessage("\r\nERROR: no angle packet frame detected\r\n");                         
						break;
				case 3:
						writeMessage("\r\nStatus: OK\r\n");                          // REMOVE when not troubleshooting
						break;
				default:
					break;
				}
			break;
		case 1:
			last_distance = receivedPacket[index];
			display_distance(last_distance);
			break;
				
		default:
			break;	
				}
			}
		}

//TIVA 1
void TIMER7A_Handler(void) {
	uint32_t status = TIMER7->MIS;
	int index = 0;
	makePacket_tiva1(mode, min_angle, max_angle, target_angle);
	
	if ((status & (1<<0)) == (1<<0)) {
		while (SSI0->SR & (1<<1)) {        											// while transmit fifo not full	
			SSI0->DR = sendPacket_tiva1[index++]; 								//write packet to DR	
			writeMessage("\r\nsent packet frame.\r\n"); 					// REMOVE testing messaage
			}
		}
	
	index = 0;
	TIMER7->ICR |= (1<<0);
}


//Tiva 1
void SSI0_Handler(void) {                   //CHECK Comment out implementing tiva 2
	
	static int index = 0;   									//remember the index as each frame is caught
	uint32_t status = SSI0->MIS;
	
	if ((status & (1<<2)) == (1<<2)) {
		 	int16_t frame = SSI0->DR;
		if (index < TIVA2_PACKET_SIZE) {
			receivePacket_tiva1[index++] = frame;     //build the packet
		}
	}
		if (index == TIVA2_PACKET_SIZE) {          	//when last frame is reached
			unpackPacket_tiva1(receivePacket_tiva1); 	// parse the packet and update flags
			index = 0;                           			//reset for the next packet
	}		
	SSI0->ICR |= (1<<2); //clear interrupt
}


void reset_flags_tiva1(void) {
	setMinAngle = false;
	setMaxAngle = false;
	automaticMode = false;
  manualMode = false;
}




