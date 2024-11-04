# ServoRadar-TIVA-Embedded-Systems
University assignment for embedded systems.

This project aimed to configure two tiva microcontroller boards to allow the user to control an ultrasonic sensor mounted on a servo motor. 
The project features:
-  2 tiva boards configured to use SPI communication. TIVA 1 controls the user actions through terminal commands and the operation of the potentiometer. TIVA 2 operates the servo motor and ultrasonic sensor.
-  ADC conversion of a voltage read from a pin on TIVA 1 connected to an external potentiometer used to configure the angular position of the servo motor during manual mode.
-  UART subsystem used to provide a terminal interface for the user to issue commands via TERATERM.
-  A SG90 servo motor with a 120 degree sweep range driven by a PWM signal from TIVA 2.
-  A SRF05 ultrasonic sensor triggered from TIVA 2 using a PWM pulse and input capture/timer subsystems to measure the response. 

The user will enter commands in a terminal prompt to interact with the program configured through the TIVA's UART subsystem and a computer connection with TERATERM.
| Key | Action |
|----|----------|
|<esc>| Escape. Leave the current measurement mode and wait for new command. A piezo buzzer is used to emit a beep when the <ESC> key is pressed. |
| < | Request the minimum angle of the range (default 0) |
| > | Request the maximum angle of the range (maximum 180) |
| A | Enter automatic mode |
| M | Enter manual mode and use a potentiometer to set the angle between 0 and 180 whilst cotninually displaying the distance in centimetres. |

