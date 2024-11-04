//TIVA 2

#include "ES.h"
#include <TM4C129.h>
#include "my_lib.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define SERVO_PWM_PORT (1<<5)  // PORT F 
#define SERVO_PWM_PIN (1<<1)   // PF1
#define PWM_MODULE (1<<0)
#define SERVO_PWM_LOAD 49999     

extern int16_t sweep_angle;
extern int16_t max_angle;
extern int16_t min_angle;

extern bool automaticMode;
extern bool manualMode;
extern bool exitProgram;

void Setup_Servo_PWM(void) {
	
	//Enable clock to GPIO Port F
	SYSCTL->RCGCGPIO |= SERVO_PWM_PORT; 
	while ((SYSCTL->PRGPIO & (SERVO_PWM_PORT)) == 0) {__ASM("nop");} 
	
	//Enable AFSEL for PF1 PWM
	GPIOF_AHB->AFSEL |= SERVO_PWM_PIN;
	GPIOF_AHB->PCTL &=~ 0xF0;
	GPIOF_AHB->PCTL |= 0x60;
	GPIOF_AHB->DEN |= SERVO_PWM_PIN;
	

	SYSCTL->RCGCPWM |= PWM_MODULE; // Module 0
	while ((SYSCTL->PRPWM & PWM_MODULE) == 0) {__ASM("nop");}
	
	// Set Clock & Prescale 20ms Target  20ms/25ns = 800 000 > 655355, need prescalar. 
	// select /16, 40Mhz/16 = 2.5, 1/2.5Mhz = 400ns, 20ms/400ns = 50000, load 49999
	
	//set clock use prescalar
	PWM0->CC = 0x103;  

	// Disable & mode
	PWM0->_0_CTL &= ~(1<<1);   // count down to 0
	PWM0->_0_CTL &= ~(1<<0);   // turn off
	
	//configure PWM gen number = _0_ letter = B
	PWM0->_0_GENB |= ((0x2<<10) | (0x3<<2));       
	PWM0->_0_LOAD = SERVO_PWM_LOAD;
	PWM0->_0_CMPB = calc_match_value(&sweep_angle);  // sets to default sweep angle on startup
	
	PWM0->_0_CTL |= (1<<0);   //enable
	
	PWM0->ENABLE |= (1<<0);   //globally enable module 0
	
}


uint32_t calc_match_value(int16_t *sweep_angle) {
	
	  // Ensure angle is within the valid range (-60 to 60 degrees)
    if (*sweep_angle < -60) *sweep_angle = -60;
    if (*sweep_angle > 60) *sweep_angle = 60;

    // Calculate pulse width (in seconds)
    float pulse_width = 1 + (*sweep_angle + 60) / 120.0;  // From 1 ms to 2 ms

    // Convert pulse width (in ms) to match value for PWM
    uint32_t match_value = SERVO_PWM_LOAD - (pulse_width / 20.0) * SERVO_PWM_LOAD;

    return match_value;  
}

// adjusts the waveform in the PWM
void set_servo_angle(int16_t *sweep_angle) {
	
	PWM0->_0_CMPB = calc_match_value(sweep_angle);

}
	
void automatic_mode_loop(int16_t *sweep_angle) {
	while (automaticMode) {
    static bool increasing = true;  // Tracks whether we're sweeping up or down

    // If the current angle is out of bounds, correct by moving the servo gradually
    if (*sweep_angle < min_angle) {
        (*sweep_angle)++;  // Gradually move towards the new min_angle
        set_servo_angle(sweep_angle);  // Physically move the servo
				increasing = true;
        continue;  // Exit early to let the servo adjust
			
    } else if (*sweep_angle > max_angle) {
        (*sweep_angle)--;  // Gradually move towards the new max_angle
        set_servo_angle(sweep_angle);  // Physically move the servo
				increasing = false;
        continue;  // Exit early to let the servo adjust
    }

    // Sweep logic
    if (increasing) {
        (*sweep_angle)++;
        if (*sweep_angle >= max_angle) {
            increasing = false;  // Start sweeping back
        }
    } else {
        (*sweep_angle)--;
        if (*sweep_angle <= min_angle) {
            increasing = true;  // Start sweeping forward
        }
    }
    set_servo_angle(sweep_angle);  // Move the servo to the new angle
		ES_msDelay(40); // give it time to move

    // Check if any user input flags should terminate or change the mode
    if (manualMode || exitProgram) {
        return;  // Exit automatic mode
    }
	}
}

void manual_mode_loop(int16_t *sweep_angle, volatile int16_t *target_angle) {           
	//target angle will be calculated by a timer that polls the ADC reading and maps it to a valid angle,
	int16_t current_angle = *sweep_angle;     // get the current servo position

	while (manualMode) {
				int target_angle_value = *target_angle;

        // Gradually adjust current_angle towards target_angle
        if (current_angle < target_angle_value) {
            current_angle++;  // Increment towards target
        } else if (current_angle > target_angle_value) {
            current_angle--;  // Decrement towards target
        }
        // Update the PWM to reflect the new angle
        set_servo_angle(&current_angle);   
        ES_msDelay(40);    

			if (automaticMode || exitProgram) {
        return;  // Exit automatic mode
    }
	}

}


