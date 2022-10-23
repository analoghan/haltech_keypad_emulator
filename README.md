# haltech_keypad_emulator
Emulator to simulate a Haltech Canbus Keypad for input expansion.

This sketch is deisgned to run on an Macchina M2 and EMULATE a Haltech 3x5 Keypad (Keypad "B").

### The following functions are performed:
* Automatically determine CANbus speed on CAN "0" bus
* Negotiate with Haltech ECU as 3x5 Haltech keypad and manage keepalives
* Output to keypad with status based on other inputs.

## Dependencies:
* M2_12VIO - https://github.com/TDoust/M2_12VIO
* due_can - https://github.com/collin80/due_can

This will eventually get merged with functions from my keypad translator program to allow it to manage my ECUMaster H-Bridge for Tesla parking brake control and Haltech IC7 Dash status.
