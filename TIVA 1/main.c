#include "ES.h"
#include <TM4C129.h>
#include "my_lib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define TIVA1_PACKET_SIZE 5
#define TIVA2_PACKET_SIZE 3

// input flags TIVA 1
volatile bool automaticMode = false;
volatile bool manualMode = false;
volatile bool exitProgram = false;
volatile bool setMinAngle = false;
volatile bool setMaxAngle = false;

//debounce flags  TIVA 1
volatile bool debounceFlag = false;
volatile int input;

// system variables  
volatile char mode = 'E';                	//initial flag configuration
volatile int target_angle = 0;  					//adjusted by a adc interrupt on tiva1
volatile int16_t max_angle = 60;					//adjusted by user input
volatile int16_t min_angle = -60;

//communication
volatile int16_t receiveStatus;   // for error testing

volatile int16_t sendPacket_tiva1[TIVA1_PACKET_SIZE];
volatile int16_t receivePacket_tiva1[TIVA2_PACKET_SIZE];

volatile float last_distance = 0;          //last calculated distance from the pulse width

int main (void) {
	
	ES_setSystemClk(40);
	
	//UART
	Setup_GPIO_UART();        //tested
	Setup_UART();							//tested
	Setup_UART0_Interrupt();	//tested
	Setup_DebounceTimer();		//tested

	//Piezo
	Setup_Piezo_PWM();
	Setup_Piezo_TIMER();
	
	//ADC
	Setup_Potentiometer_ADC();
	Setup_Potentiometer_Timer();
	
	//SSI
	Setup_Tiva1_SSI();
	Setup_Tiva1_SSI_Master_Timer();
	
	//Setup callbacks
	tiva1callbacks();					//tested

	writeMessage("System on. Press A or M to select a sweep mode, < or > to set the sweep angle, or ESC to exit.\r\n\n");

    while (1) {
			
				if (exitProgram) {
						callbacks_tiva1[0]();
				 } 	
				 
				if (automaticMode) {
						callbacks_tiva1[1]();
				}
	
				if (manualMode) {
						callbacks_tiva1[2]();
				}
				
				if (setMinAngle) {
						callbacks_tiva1[3]();
				}
				
				if (setMaxAngle) {
						callbacks_tiva1[4]();				
				}
		
    }		
}	

	
