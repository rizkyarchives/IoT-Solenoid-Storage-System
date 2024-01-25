# IoT-Solenoid-Storage-System
An IoT Storage System designed for enhanced security and convenience. This system integrates a solenoid lock with multiple authentication options (PIN using 3x4 membrane keypad, RFID using RC522 module) and IoT capabilities, allowing remote access through a Webserver with HTML5, CSS, and Javascript. Our aim is to provide an affordable yet robust solution for protecting valuable items, addressing the limitations of commercial storage systems. A data logging feature is also present using ThingSpeak, allowing user to check when the lock is open or tried to be open but failed.

This project also uses two microcontrollers, which are ESP32 and Arduino UNO R3, for the sole reason of adding complexity into the system to enhance our learning experience. ESP32 will be communicating with the Webserver and send/receive any data via Serial Communication with Arduino. While the Arduino is the microcontroller that have direct access to the sensors and actuators and will also send/receive data with ESP32.

## Materials and Components
* ESP 32
* Arduino UNO R3
* MFRC522 RFID Reader
* Keypad Membrane 3x4
* Relay 5V
* LCD I2C 1602
* 12V Solenoid Lock
* Breadboard (Optional)

## Schematics


## Output
This is a video that will demonstrate + explain this project:

## License
This project is licensed under MIT License.
