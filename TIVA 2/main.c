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

// system variables  
volatile char mode = 'E';                	//initial flag configuration
volatile int target_angle = 0;  					//adjusted by a adc interrupt on tiva1
volatile int16_t max_angle = 60;					//adjusted by user
volatile int16_t min_angle = -60;
int16_t sweep_angle = 0;          				//current position of servo during auto mode
uint32_t last_pulse = 0;          				//last measured pulse width of object
volatile float last_distance = 0;         //last calculated distance from the pulse width

//communication
volatile int16_t sendStatus;
volatile int16_t sendPacket_tiva2[TIVA2_PACKET_SIZE];
volatile int16_t receivePacket_tiva2[TIVA1_PACKET_SIZE];

int main (void) {
	
	ES_setSystemClk(40);
	
	//Servo
	Setup_Servo_PWM();        //sets everything as intended
	
	//Sensor
	Setup_SensorInput();
	Setup_SensorOutput();
	Setup_PeriodicPulseTimer(); //for periodic sensor pulsing @250ms
	
	//SSI
	Setup_Tiva2_SSI();
	Setup_Tiva2_SSI_Slave_Timer();

	//Setup callbacks
	tiva2callbacks();					

    while (1) {
			
				if (exitProgram) {
						callbacks_tiva2[0]();
				 } 	
				 
				if (automaticMode) {
						callbacks_tiva2[1]();
				}
	
				if (manualMode) {
						callbacks_tiva2[2]();
				}
					
    }		
}	

	
