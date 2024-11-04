//TIVA 1

#include <TM4C129.h>
#include "my_lib.h"
#include "ES.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define UART 0

#define UART_RX (1<<4)
#define UART_TX (1<<5)

#define AUTO_MESSAGE 		"Starting automatic sweep...\r\n"
#define MANUAL_MESSAGE 	"Entering manual mode...\r\n"
#define SET_MIN_ANGLE		  "Set minimum sweep angle:\r\n"
#define SET_MAX_ANGLE 		"Set maximum sweep angle:\r\n"
#define EXIT						"Exiting program...\r\n"

#define MAX_INPUT_LENGTH 5

extern bool automaticMode;
extern bool manualMode;
extern bool setMinAngle;
extern bool setMaxAngle;
extern bool exitProgram;
extern bool debounceFlag;
extern int input;
extern volatile char mode;  


//GPIO SETUP FOR TEST

void Setup_GPIO_UART(void) {

		SYSCTL->RCGCGPIO |= (1<<0);
		while ((SYSCTL->PRGPIO & (1<<0)) == 0) {__ASM("nop");}
		
		//Configure port A pins 0 and 1, DEN = 1, AFSEL = 1, PCTL = 0x11, 
		
	GPIOA_AHB->AFSEL |= ((1<<0) | (1<<1));
	GPIOA_AHB->PCTL = (GPIOA_AHB->PCTL &~ 0xFF) | 0x11;
	GPIOA_AHB->DEN |= ((1<<1) | (1<<0));
}


//UART

void Setup_UART(void) {
    // UART0 configuration (example, set baud rate, data format, etc.)
	
    // Enable clock to UART0
		SYSCTL->RCGCUART |= (1<<UART);
		while ((SYSCTL->PRUART & (1<<UART)) == 0) {__ASM("nop");}
    
    // Disable UART for configuration
		UART0->CTL &= ~(1<<0);

		// SET BRD 38400  // 40Mhz Prescalar: 16
		UART0->IBRD = 65;
		UART0->FBRD = 7;

		// Line control no parity, 1 stop bit, 8 bit WLEN
		//0100 1000 = 0x60
		//Configure Line control

		UART0->LCRH = 0x60;

		// Configure Clock not needed using system clock everything 0
		UART0->CC &= ~(1<<0);

		//enable UART
		UART0->CTL |= (1<<0);
}

// MAKE SURE TO UPDATE THIS IF YOU CHANGE YOUR UART

void Setup_UART0_Interrupt(void){
	
		__disable_irq();
	
		UART0->IM |= (1<<4); // IRQ for UART0 RXIM
	
		NVIC->IP[5] = 0;
	
		NVIC->ISER[0] |= (1<<5);

		__enable_irq();	
}

void writeChar(char data) {

			while ((UART0->FR & UART_TX) != 0) {
				__ASM("NOP");
			}			
			UART0->DR = data;
}

void writeMessage(char *str) {
		int i = 0;
		while (str[i] != '\0') {
			writeChar(str[i]);
			i++;
		}
}

char getChar(void) {
	char receivedChar = UART0->DR;
	return receivedChar;
}

void process_char(char input) {

    switch (input) {
        case 'A':
            automaticMode = true;   // Flag to initiate automatic mode
						manualMode =false;
            break;

        case 'M':
            manualMode = true;      // Flag to initiate manual mode
						automaticMode = false;
            break;

        case '<':
					if ((automaticMode) || (manualMode)) {
						break;
					}
            else { setMinAngle = true;
            break;
						}

        case '>':
					if ((automaticMode) || (manualMode)) {
						break;
					}
            else { setMaxAngle = true;
            break;
						}

        case 27:  // ASCII for ESC
            exitProgram = true;     // Flag to exit the program
            break;

        default:     
            break;
    }
}

void captureInput(int16_t* value) {
	
	char set_value[MAX_INPUT_LENGTH];
	int index = 0;
	char input = 0;
	char lastInput = 0;
	
	while(1) {
				
				if (exitProgram) {return;} // detect if exit command has been sent
		
				if (debounceFlag && (input == lastInput)) {
					continue;
				}
				
				while((UART0->FR & UART_RX) == UART_RX) {   //wait for user input
					__ASM("NOP");
					if (exitProgram) {return;}
				}
				
				input = getChar();
						
				if (input == '\r' || input == '\n') {
					set_value[index] = '\0';
					writeMessage("\n\r");
					break;	
				}	
							
				if (index < MAX_INPUT_LENGTH - 1){
				set_value[index] = input;
				index++;
				writeChar(input);
				lastInput = input;
			}
		}				
	
	set_value[index] = '\0';
	*value = atoi(set_value);
}


void UART0_Handler(void) {
		
    uint32_t status = UART0->MIS;  // Read masked interrupt status
	
		if (!debounceFlag) {
			startDebounce();
			
    if ((status & UART_RX) == UART_RX) {
        process_char(getChar());  // Process the user input for different keypress flags eg esc, <, >, a, m
    }
	} 	
		UART0->ICR |= UART_RX;  // Clear RX interrupt flag
}