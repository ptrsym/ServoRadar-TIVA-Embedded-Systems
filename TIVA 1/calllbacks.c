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


// splot routines for tiva 1 and 2

void onExitProgram() {
		mode = 'E';
    writeMessage(EXIT);
    reset_flags_tiva1();           
		stopADC();									
    trigger_piezo();
		exitProgram = false;
}

void onAutomaticMode() {
		mode = 'A';
		stopADC();
    writeMessage(AUTO_MESSAGE);
		automaticMode = false;
}

void onManualMode() {
		mode = 'M';
    writeMessage(MANUAL_MESSAGE);
		startADC();
		manualMode = false;
}

void onSetMinAngle() {
    writeMessage(SET_MIN_ANGLE);
    captureInput(&min_angle);
		if ((min_angle < -60) || (min_angle > 60)) {
			ES_Uprintf(0, "\r\nAngle %d is out of bounds. Overriden to -60.\r\n", min_angle);
			min_angle = -60;
		} else if (min_angle > max_angle) {
				ES_Uprintf(0, "\r\nMin angle %d cannot be greater than current max angle. Set to -60.\r\n", min_angle);
				min_angle = -60;
		}
		else {ES_Uprintf(0, "\r\nMin angle set to: %d\r\n", min_angle);}
    setMinAngle = false;
}

void onSetMaxAngle() {
    writeMessage(SET_MAX_ANGLE);
    captureInput(&max_angle);
		if ((max_angle < -60) || (max_angle > 60)) {
			ES_Uprintf(0, "\r\nAngle %d is out of bounds. Overriden to 60.\r\n", max_angle);
			max_angle = 60;
		} else if (max_angle < min_angle) {
				ES_Uprintf(0, "\r\nMax angle %d cannot be less than current min angle. Set to 60.\r\n", max_angle);
				max_angle = 60;
		}
		else {ES_Uprintf(0, "\r\nMax angle set to: %d\r\n", max_angle);}
    setMaxAngle = false;
}

//update with each callback made

void tiva1callbacks() {
    callbacks_tiva1[0] = onExitProgram;
    callbacks_tiva1[1] = onAutomaticMode;
    callbacks_tiva1[2] = onManualMode;
    callbacks_tiva1[3] = onSetMinAngle;
    callbacks_tiva1[4] = onSetMaxAngle;
}










