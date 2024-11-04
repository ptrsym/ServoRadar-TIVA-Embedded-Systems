#ifndef MY_LIB_H
#define MY_LIB_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define NUM_CALLBACKS 5

#define TIVA1_PACKET_SIZE 5
#define TIVA2_PACKET_SIZE 3

typedef void (*CallbackFunction)();


/// USING:

/// TIMER 3A (SENSOR OUTPUT ONESHOT), 4A PERIODIC POTENTIOMETER ADC SAMPLER,   5A+B (SENSOR INPUT CAPTURE), 6A (DEBOUNCE TIMER)  7A (PERIODIC SENSOR PULSE GENRATION)
/// UART0
/// PORT A PINS: 0, 1 (UART0), 6 (SENSOR OUTPUT PULSE)
/// PORT F PINS: 1 (SERVO PWM MODULE)
/// PORT B PINS: 2 (SENSOR INPUT CAPTURE)
/// PORT E PINS: 3 ADC0 AIN0 (POTENTIOMETER)


//Setups

extern void Setup_GPIO_UART(void) ;

extern void Setup_UART(void);

extern void Setup_UART0_Interrupt(void);

extern void Setup_SensorInput (void);

extern void Setup_SensorOutput (void);

extern void Setup_Servo_PWM (void);

extern void Setup_PeriodicPulseTimer(void);

extern void Setup_Potentiometer_ADC(void);

extern void Setup_Potentiometer_Timer(void);

extern void Setup_Tiva1_SSI(void);

extern void Setup_Tiva1_SSI_Master_Timer(void);

extern void Setup_Tiva2_SSI(void);

extern void Setup_Tiva2_SSI_Slave_Timer(void);

extern void Setup_Piezo_PWM(void);

extern void Setup_Piezo_TIMER(void);

//ISR Handlers

extern void SSI0_Handler(void);

extern void UART0_Handler(void);

extern void TIMER3A_Handler(void);

extern void TIMER5A_Handler(void);

extern void TIMER5B_Handler(void);

extern void TIMER6A_Handler(void);  

extern void TIMER7A_Handler(void);

extern void ADC0SS0_Handler(void);

//UART

extern void process_char(char received_char);

extern void writeChar(char data);

extern void writeMessage(char *str);

extern char getChar(void);

extern void captureInput(int16_t* value);

//TIVA 1 COMMUNICATION

extern void reset_flags_tiva1(void);

extern void makePacket_tiva1(char mode, int16_t min_angle, int16_t max_angle, int16_t target_angle);

extern void unpackPacket_tiva1(volatile int16_t receivedPacket[TIVA2_PACKET_SIZE]);


//TIVA 2 COMMUNICATION

extern void reset_flags_tiva2(void);

extern void makePacket_tiva2(int16_t status, float objectDistance);

extern void unpackPacket_tiva2(volatile int16_t receivedPacket[TIVA1_PACKET_SIZE]);


//CALLBACKS

extern CallbackFunction callbacks_tiva1[NUM_CALLBACKS];  

extern void tiva1callbacks();

extern void onExitProgram(); 
extern void onAutomaticMode();
extern void onManualMode(); 
extern void onSetMaxAngle();
extern void onSetMinAngle();

//CALLBACKS TIVA2

extern CallbackFunction callbacks_tiva2[NUM_CALLBACKS]; 

extern void tiva2callbacks();

extern void tiva2_onExitProgram();
extern void tiva2_onAutomaticMode();
extern void tiva2_onManualMode();


//TIMERS

extern void Setup_DebounceTimer(void);

extern void Setup_PeriodicPulse_Timer (void);

extern void startDebounce(void);

extern void startAutoPulse(); 

extern void stopAutoPulse(void);

//Servo

extern void automatic_mode_loop(int16_t *sweep_angle);

extern void manual_mode_loop(int16_t *sweep_angle, volatile int16_t *target_angle);

extern void set_servo_angle(int16_t *sweep_angle);

extern uint32_t calc_match_value(int16_t *sweep_angle);


//Sensor


extern void calc_pulse_time(uint32_t start, uint32_t end, uint32_t *last_pulse);

extern void pulse_sensor (void);

extern void pulse_to_distance(uint32_t *last_pulse, float *last_distance);


//Potentiometer ADC

extern void startADC(void);

extern void stopADC(void);


//Misc
extern void trigger_piezo(void);

extern void kill_all_flags (void);

//LCD

extern void display_distance(float last_distance);


#endif // MY_LIB_H
