# ServoRadar-TIVA-Embedded-Systems
University assignment for embedded systems.

## Aim

This project's aim was to configure two tiva microcontroller boards to allow a user to control an ultrasonic sensor mounted on a servo motor and then read object distances from a terminal. 
It was coded in Keil µVision for the Tiva TM4C129NCPDT.

## Features

The project features:
-  2 tiva boards configured to use SPI communication. TIVA 1 controls the user actions through terminal commands and the operation of the potentiometer. TIVA 2 operates the servo motor and ultrasonic sensor.
-  ADC conversion of a voltage read from a pin on TIVA 1 connected to an external potentiometer used to configure the angular position of the servo motor during manual mode.
-  UART subsystem used to provide a terminal interface for the user to issue commands via TERATERM.
-  A SG90 servo motor with a 120 degree sweep range driven by a PWM signal from TIVA 2.
-  A SRF05 ultrasonic sensor triggered from TIVA 2 using a PWM pulse and input capture/timer subsystems to measure the response. 

The user terminal commands are as follows:
| Key | Action |
|----|----------|
|<esc>| Escape. Leave the current measurement mode and wait for new command. A piezo buzzer is used to emit a beep when the <ESC> key is pressed. |
| < | Request the minimum angle of the range (default 0) |
| > | Request the maximum angle of the range (maximum 180) |
| A | Enter automatic mode |
| M | Enter manual mode and use a potentiometer to set the angle between 0 and 180 whilst cotninually displaying the distance in centimetres. |

## Operation

1. The user enters a command in the terminal and presses enter.
2. If manual mode is enabled a pin connected to a potentiometer is read for a voltage value that is converted to an angular position, else, automatic mode causes the servo motor to sweep back and forth within the configured angle ranges.
3. The SPI protocol packages the setting variables stored on tiva 1 and sends them to tiva 2 via a periodic timer interrupt.
4. Tiva 2 unpacks the packet and updates the current settings for the motor/sensor operation.
5. The ultrasonic sensor periodically pulses and the return pulse width response is measured and mapped to a distance value which is stored locally.
6. THe most recent distance value and/or any error messages are sent back to tiva 1 to be displayed on the terminal.
