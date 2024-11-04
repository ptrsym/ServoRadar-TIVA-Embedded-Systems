#include "ES.h"
#include <TM4C129.h>
#include "my_lib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define AUTO_MESSAGE 		"Starting automatic sweep...\r\n"
#define MANUAL_MESSAGE 	"Entering manual mode...\r\n"
#define SET_MIN_ANGLE		  "Set minimum sweep angle:\r\n"
#define SET_MAX_ANGLE 		"Set maximum sweep angle:\r\n"
#define EXIT						"Exiting program...\r\n"

#define NUM_CALLBACKS 5

CallbackFunction callbacks_tiva1[NUM_CALLBACKS];
CallbackFunction callbacks_tiva2[NUM_CALLBACKS];  

// input flags
extern bool automaticMode;
extern bool manualMode;
extern bool exitProgram;
extern bool setMinAngle;
extern bool setMaxAngle;
extern volatile char mode;   

//debounce flags
extern bool debounceFlag;
extern int input;

// system variables
extern int16_t sweep_angle;            
extern volatile int16_t target_angle;  //adjusted by a timer interrupt
extern int16_t max_angle;
extern int16_t min_angle;
extern uint32_t last_pulse;
extern float last_distance;


void tiva2_onExitProgram() {
    reset_flags_tiva2();
		stopAutoPulse();
		exitProgram = false;
}

void tiva2_onAutomaticMode() {
		manualMode = false;
		automaticMode = true;
		startAutoPulse();                
    automatic_mode_loop(&sweep_angle);
}

void tiva2_onManualMode() {   
		automaticMode = false;
		manualMode = true;
		startAutoPulse();
    manual_mode_loop(&sweep_angle, &target_angle);
}

void tiva2callbacks() {
	callbacks_tiva2[0] = tiva2_onExitProgram;
	callbacks_tiva2[1] = tiva2_onAutomaticMode;
	callbacks_tiva2[2] = tiva2_onManualMode;
}










