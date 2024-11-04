# ServoRadar-TIVA-Embedded-Systems
University assignment for embedded systems for a servomotor controlled ultrasonic sensor.

This project aimed to configure two tiva boards to allow the user to control an ultrasonic sensor mounted on a servo motor.

The user will enter commands in a terminal prompt to interact with the program configured through the TIVA's UART subsystem and a computer connection with TERATERM.
| Key | Action |
|----|----------|
|<esc>| Escape. Leave the current measurement mode and wait for new command. A piezo buzzer is used to emit a beep when the <ESC> key is pressed. |
| < | Request the minimum angle of the range (default 0) |
| > | Request the maximum angle of the range (maximum 180) |
| A | Enter automatic mode |
| M | Enter manual mode and use a potentiometer to set the angle between 0 and 180 whilst cotninually displaying the distance in centimetres. |

