//#include <SD.h>

// Including Arduino_Due_SD_HSCMI library also creates SD object (MassStorage class)
#include <Arduino_Due_SD_HSMCI.h>                       // This creates the object SD
#include "variant.h"
#include <due_can.h>

#include <Arduino.h>
#include <pins_arduino.h>
#include <stdint.h>

#include <M2_12VIO.h>

M2_12VIO M2IO;                                          //Constructor for the M2_12Vio library

// We need to create FileStore object, we will be using it to open/create file and to close file.
FileStore FS;
byte initial_brake_position;

// Do Setup.   Set Pins as outputs and note as Source (Positive) or Sink (Negative)
void setup() {

  M2IO.Init_12VIO();                                    // Initialise the M2I/O library;
  M2IO.Setpin_12VIO(1, OFF, SOURCE, PWM_PIN, 1000, 0);  // Set 12V Pin 1 as 12V source, 10000hz, duty cycle XX; Mapped to XXX 
  M2IO.Setpin_12VIO(2, OFF, SOURCE, PWM_PIN, 1000, 0);  // Set 12V Pin 2 as 12V source, 10000hz, duty cycle XX; Mapped to XXX
  M2IO.Setpin_12VIO(3, OFF, SOURCE, PWM_PIN, 1000, 0);  // Set 12V Pin 3 as 12V source, 10000hz, duty cycle XX; Mapped to XXX
  M2IO.Setpin_12VIO(4, OFF, SINK);                      // SET 12V Pin 4 as Ground Sink; Mapped to XXX
  M2IO.Setpin_12VIO(5, OFF, SINK);                      // SET 12V Pin 5 as Ground Sink; Mapped to XXX
  M2IO.Setpin_12VIO(6, OFF, SINK);                      // SET 12V Pin 6 as Ground Sink; Mapped to XXX

  pinMode(LED_BUILTIN, OUTPUT);                         // Set onboard LED as output

  // SD Card Setup
  InitalizeSD();
  ReadSD();

  // Setup CANbus
  CANAutoBaud();
}

//Initialize SD Card
void InitalizeSD () {
  SD.Init();                                            // Initialization of HSCMI protocol and SD socket switch GPIO (to adjust pin number go to library source file - check Getting Started Guide)
  FS.Init();                                            // Initialization of FileStore class for file manipulation
}

// Read data from SD card
void ReadSD () {
  FS.Open("0:","can_status.txt",false);                 // Open file named "can_status.txt" in directory "0:" in read only mode. 
  SerialUSB.print("Reading from file can_status.txt");  // Print Status
  SerialUSB.print('\n');                                // New line
  FS.Seek(0);                                           // Go to start of file
  char can_initial_status;                              // Define var for can_initial_status
  FS.Read(can_initial_status);                          // Read one byte from file and store it in "can_initial_status"
  initial_brake_position = can_initial_status;
  String initial_brake_status; 
  if ( can_initial_status == 0 ) {
    initial_brake_status = "released";
    M2IO.Setpin_12VIO(4, OFF);    
  } else if ( can_initial_status == 1 ) {
    initial_brake_status = "set";
    M2IO.Setpin_12VIO(4, ON);    
  } else {
    initial_brake_status = "errored";
  }
  SerialUSB.print("Initial brake status is ");          // Format output
  SerialUSB.print(initial_brake_status);                // Print can_initial_status to serial
  SerialUSB.print('\n');                                // New line
  FS.Close();                                           // File is saved, when it is closed. When closed, you can rename it or delete it.  
}

// Write data to SD card
void WriteSD(byte can_status) {
  String brake_status; 
  if ( can_status == 0 ) {
    brake_status = "released";
  } else if ( can_status == 1 ) {
    brake_status = "set";
  } else {
    brake_status = "errored";
  }
  FS.Open("0:","can_status.txt",true);                  // Open file in directory "0:"; third atribute is read (false) / write (true) 
  SerialUSB.print("Writing to file can_status.txt");
  SerialUSB.print('\n');
  FS.Seek(0);                                           // Go to start of file
  SerialUSB.print("Current brake status is ");
  SerialUSB.print(brake_status);
  SerialUSB.print('\n');
  FS.Write(can_status);
  FS.Close();                                           // File is saved, when it is closed. When closed, you can rename it or delete it. 
  initial_brake_position = can_status;
}


// Automatically setup CAN
void CANAutoBaud() {
  SerialUSB.println("Doing Auto Baud scan on CAN0");
  Can0.beginAutoSpeed();
  
  //By default there are 7 mailboxes for each device that are RX boxes
  //This sets each mailbox to have an open filter that will accept extended 
  //or standard frames
  int filter;
  //extended
  for (filter = 0; filter < 3; filter++) {
  Can0.setRXFilter(filter, 0, 0, true);
  }  
  //standard
  for (int filter = 3; filter < 7; filter++) {
  Can0.setRXFilter(filter, 0, 0, false);
  }
}


// Send CAN Frames emulating Haltech Keypad.   Cases per combo instead of math because I'm lazy and we hae excess speed and storage anyway.

void CANButtonFrame(int ControlCase, int state) {
  CAN_FRAME outgoing ;
  if (ControlCase < 16) {
    outgoing.id = 397;
    outgoing.extended = false;
    outgoing.length = 3;
    if (ControlCase == 0) { // HL
      outgoing.data.byte[0] = 4;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 1) { // HL + L
      outgoing.data.byte[0] = 5;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 2) { // HL + R
      outgoing.data.byte[0] = 6;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 3) { // HL + L + R
      outgoing.data.byte[0] = 7;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }        
    else if (ControlCase == 4) { // HL + Horn
      outgoing.data.byte[0] = 12;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 5) { // HL + L + Horn
      outgoing.data.byte[0] = 13;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 6) { // HL + R + Horn
      outgoing.data.byte[0] = 14;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }        
    else if (ControlCase == 7) { // Headlight + L + R + Horn
      outgoing.data.byte[0] = 15;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 8) { // L
      outgoing.data.byte[0] = 1;
      outgoing.data.byte[1] = 0;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 9) { // R
      outgoing.data.byte[0] = 2;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 10) { // L + R
      outgoing.data.byte[0] = 3;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 11) { // L + Horn
      outgoing.data.byte[0] = 9;
      outgoing.data.byte[1] = 0;
      outgoing.data.byte[2] = 0;
    }        
    else if (ControlCase == 12) { // R + Horn
      outgoing.data.byte[0] = 10;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 13) { // L + R + Horn
      outgoing.data.byte[0] = 11;
      outgoing.data.byte[1] = 1;
      outgoing.data.byte[2] = 0;
    }
    else if (ControlCase == 14) { // Horn Only
      outgoing.data.byte[0] = 8;
      outgoing.data.byte[1] = 0;
      outgoing.data.byte[2] = 0;
    }    
    else if (ControlCase == 15) { // No buttons pressed
      outgoing.data.byte[0] = 0;
      outgoing.data.byte[1] = 0;
      outgoing.data.byte[2] = 0;
    }
  }
  else if (ControlCase == 16) {
    if ( ControlCase == 16 ) {
      outgoing.id = 0x773;
      outgoing.extended = false;
      outgoing.length = 8;
      if ( state == 1 ) {
        outgoing.data.byte[0] = 0x04;
        outgoing.data.byte[1] = 0xFF;
        outgoing.data.byte[2] = 0x00;
        outgoing.data.byte[3] = 0x00;
        outgoing.data.byte[4] = 0x00;
        outgoing.data.byte[5] = 0x00;
        outgoing.data.byte[6] = 0x00;
        outgoing.data.byte[7] = 0x00;
      } else if( state == 0 ) {
        outgoing.data.byte[0] = 0xFF;
        outgoing.data.byte[1] = 0x04;
        outgoing.data.byte[2] = 0x00;
        outgoing.data.byte[3] = 0x00;
        outgoing.data.byte[4] = 0x00;
        outgoing.data.byte[5] = 0x00;
        outgoing.data.byte[6] = 0x00;
        outgoing.data.byte[7] = 0x00;   
      }
    }  
  }
//    SerialUSB.print("Sending Button Frame ");
//    SerialUSB.print("\r\n");  
  Can0.sendFrame(outgoing);
}


void CANSyncFrames(int b0, int b1, int b2, int b3, int b4, int b5, int b6, int b7) {
  CAN_FRAME ka1;
    ka1.id = 1421;
    ka1.extended = false;
    ka1.length = 8;
    ka1.data.byte[0] = b0;
    ka1.data.byte[1] = b1;
    ka1.data.byte[2] = b2;
    ka1.data.byte[3] = b3;
    ka1.data.byte[4] = b4;
    ka1.data.byte[5] = b5;
    ka1.data.byte[6] = b6;
    ka1.data.byte[7] = b7;        
//    SerialUSB.print("Sending Sync Frame");
//    SerialUSB.print("\r\n");  
  Can0.sendFrame(ka1);
}


void CANKeepAliveFrames() {
  CAN_FRAME ka2;
    ka2.id = 1805;
    ka2.extended = false;
    ka2.length = 1;
    ka2.data.byte[0] = 5;       
//    SerialUSB.print("Sending Keepalive Frame");
//    SerialUSB.print("\r\n");  
  Can0.sendFrame(ka2);
}


// CAN Input from Haltech Keypad
void processFrame(CAN_FRAME &frame) {
int b0;
int b1;
int b2;
int b3;
int b4;
int b5;
int b6;
int b7;
int call;
byte brake_position;

// Keypad Configuration Section

  if (frame.id == 1549) {
    if ((frame.data.bytes[0]) == 34 ) {
      b0 = 96;
      b1 = (frame.data.bytes[1]);
      b2 = (frame.data.bytes[2]);
      b3 = (frame.data.bytes[3]);
      b4 = 0;
      b5 = 0;
      b6 = 0;
      b7 = 0;  
//      SerialUSB.print("Type 34 frame identified ");
//      SerialUSB.print("\r\n");
      CANSyncFrames(b0, b1, b2, b3, b4, b5, b6, b7);
    }
    else if ((frame.data.bytes[0]) == 66 ) {
      b0 = 67;
      b1 = (frame.data.bytes[1]);
      b2 = (frame.data.bytes[2]);
      b3 = (frame.data.bytes[3]);
      if ((b1 == 24) && (b2 == 16) && (b3 == 1)) {
        b4 = 7;
        b5 = 4;
        b6 = 0;
        b7 = 0;
      }
      else if ((b1 == 24) && (b2 == 16) && (b3 == 2)) {
        b4 = 75;
        b5 = 51;
        b6 = 0;
        b7 = 0;
      }
      else if ((b1 == 24) && (b2 == 16) && (b3 == 3)) {
        b4 = 1;
        b5 = 0;
        b6 = 0;
        b7 = 0;
      }
      else if ((b1 == 24) && (b2 == 16) && (b3 == 4)) {
        b4 = 166;
        b5 = 184;
        b6 = 25;
        b7 = 12;
      }
      else if ((b1 == 0) && (b2 == 24) && (b3 == 1)) {
        b4 = 141;
        b5 = 1;
        b6 = 0;
        b7 = 64;
      }      
      else {
      b4 = 0;
      b5 = 0;
      b6 = 0;
      b7 = 0;
      }      
//      SerialUSB.print("Type 66 Frame Identified");
//      SerialUSB.print("\r\n");
      CANSyncFrames(b0, b1, b2, b3, b4, b5, b6, b7);
    }    
    else if (((frame.data.bytes[0]) == 0) && ((frame.data.bytes[7]) == 200)) {
      b0 = 128; 
      b1 = 0;
      b2 = 0; 
      b3 = 0;
      b4 = 1;
      b5 = 0;
      b6 = 4;
      b7 = 5; 
//      SerialUSB.print("Type 0 Frame Identified");
//      SerialUSB.print("\r\n");
      CANSyncFrames(b0, b1, b2, b3, b4, b5, b6, b7);
    }            
  }  

// CAN Inputs Section

  if (frame.id == 1280) {  //Freewheel CAN Output  
      //SerialUSB.print("Steering Wheel Button Frame ID'd ");
      //SerialUSB.print("\r\n");  
    if (((bitRead(frame.data.bytes[0], 5)) == 1 ) && ((bitRead(frame.data.bytes[0], 7)) == 0 ) && ((bitRead(frame.data.bytes[0], 6)) == 0 ) && ((bitRead(frame.data.bytes[0], 4)) == 0 ) ) {
        SerialUSB.print("Headlight Button Only ");
        SerialUSB.print("\r\n");
        CANButtonFrame(0, 3);
    }
    else if (((bitRead(frame.data.bytes[0], 5)) == 1 ) && ((bitRead(frame.data.bytes[0], 7)) == 1 ) && ((bitRead(frame.data.bytes[0], 6)) == 0 ) && ((bitRead(frame.data.bytes[0], 4)) == 0 ) ) {
        SerialUSB.print("Headlight Button + Left Turn ");
        SerialUSB.print("\r\n");
        CANButtonFrame(1, 3);
    }
    else if (((bitRead(frame.data.bytes[0], 5)) == 1 ) && ((bitRead(frame.data.bytes[0], 7)) == 0 ) && ((bitRead(frame.data.bytes[0], 6)) == 1 ) && ((bitRead(frame.data.bytes[0], 4)) == 0 ) ) {
        SerialUSB.print("Headlight Button + Right Turn ");
        SerialUSB.print("\r\n");
        CANButtonFrame(2, 3);
    }    
    else if (((bitRead(frame.data.bytes[0], 5)) == 1 ) && ((bitRead(frame.data.bytes[0], 7)) == 1 ) && ((bitRead(frame.data.bytes[0], 6)) == 1 ) && ((bitRead(frame.data.bytes[0], 4)) == 0 ) ) {
        SerialUSB.print("Headlight Button + Both Turn Signals ");
        SerialUSB.print("\r\n");
        CANButtonFrame(3, 3);
    }        
    else if (((bitRead(frame.data.bytes[0], 5)) == 1 ) && ((bitRead(frame.data.bytes[0], 7)) == 0 ) && ((bitRead(frame.data.bytes[0], 6)) == 0 ) && ((bitRead(frame.data.bytes[0], 4)) == 1 ) ) {
        SerialUSB.print("Headlight Button + Horn ");
        SerialUSB.print("\r\n");
        CANButtonFrame(4, 3);
    }
    else if (((bitRead(frame.data.bytes[0], 5)) == 1 ) && ((bitRead(frame.data.bytes[0], 7)) == 1 ) && ((bitRead(frame.data.bytes[0], 6)) == 0 ) && ((bitRead(frame.data.bytes[0], 4)) == 1 ) ) {
        SerialUSB.print("Headlight Button + Left Turn + Horn ");
        SerialUSB.print("\r\n");
        CANButtonFrame(5, 3);
    }    
    else if (((bitRead(frame.data.bytes[0], 5)) == 1 ) && ((bitRead(frame.data.bytes[0], 7)) == 0 ) && ((bitRead(frame.data.bytes[0], 6)) == 1 ) && ((bitRead(frame.data.bytes[0], 4)) == 1 ) ) {
        SerialUSB.print("Headlight Button + Right Turn + Horn ");
        SerialUSB.print("\r\n");
        CANButtonFrame(6, 3);
    }        
    else if (((bitRead(frame.data.bytes[0], 5)) == 1 ) && ((bitRead(frame.data.bytes[0], 7)) == 1 ) && ((bitRead(frame.data.bytes[0], 6)) == 1 ) && ((bitRead(frame.data.bytes[0], 4)) == 1 ) ) {
        SerialUSB.print("Headlight Button + Both Turn Signals + Horn ");
        SerialUSB.print("\r\n");
        CANButtonFrame(7, 3);
    }            
    else if (((bitRead(frame.data.bytes[0], 5)) == 0 ) && ((bitRead(frame.data.bytes[0], 7)) == 1 ) && ((bitRead(frame.data.bytes[0], 6)) == 0 ) && ((bitRead(frame.data.bytes[0], 4)) == 0 ) ) {
        SerialUSB.print("Left Turn Only ");
        SerialUSB.print("\r\n");
        CANButtonFrame(8, 3);
    }
    else if (((bitRead(frame.data.bytes[0], 5)) == 0 ) && ((bitRead(frame.data.bytes[0], 7)) == 0 ) && ((bitRead(frame.data.bytes[0], 6)) == 1 ) && ((bitRead(frame.data.bytes[0], 4)) == 0 ) ) {
        SerialUSB.print("Right Turn Only ");
        SerialUSB.print("\r\n");
        CANButtonFrame(9, 3);
    }
    else if (((bitRead(frame.data.bytes[0], 5)) == 0 ) && ((bitRead(frame.data.bytes[0], 7)) == 1 ) && ((bitRead(frame.data.bytes[0], 6)) == 1 ) && ((bitRead(frame.data.bytes[0], 4)) == 0 ) ) {
        SerialUSB.print("Both Turn Signals ");
        SerialUSB.print("\r\n");
        CANButtonFrame(10, 3);
    }
    else if (((bitRead(frame.data.bytes[0], 5)) == 0 ) && ((bitRead(frame.data.bytes[0], 7)) == 1 ) && ((bitRead(frame.data.bytes[0], 6)) == 0 ) && ((bitRead(frame.data.bytes[0], 4)) == 1 ) ) {
        SerialUSB.print("Left Turn + Horn ");
        SerialUSB.print("\r\n");
        CANButtonFrame(11, 3);
    }
    else if (((bitRead(frame.data.bytes[0], 5)) == 0 ) && ((bitRead(frame.data.bytes[0], 7)) == 0 ) && ((bitRead(frame.data.bytes[0], 6)) == 1 ) && ((bitRead(frame.data.bytes[0], 4)) == 1 ) ) {
        SerialUSB.print("Right Turn + Horn ");
        SerialUSB.print("\r\n");
        CANButtonFrame(12, 3);
    }
    else if (((bitRead(frame.data.bytes[0], 5)) == 0 ) && ((bitRead(frame.data.bytes[0], 7)) == 1 ) && ((bitRead(frame.data.bytes[0], 6)) == 1 ) && ((bitRead(frame.data.bytes[0], 4)) == 1 ) ) {
        SerialUSB.print("Both Turn Signals + Horn");
        SerialUSB.print("\r\n");
        CANButtonFrame(13, 3);
    }    
    else if (((bitRead(frame.data.bytes[0], 5)) == 0 ) && ((bitRead(frame.data.bytes[0], 7)) == 0 ) && ((bitRead(frame.data.bytes[0], 6)) == 0 ) && ((bitRead(frame.data.bytes[0], 4)) == 1 ) ) {
      SerialUSB.print("Horn Only ");
      SerialUSB.print("\r\n");
      CANButtonFrame(14, 3);
    }
    else
      // SerialUSB.print("No Steering Buttons Pressed ");
      // SerialUSB.print("\r\n");
      CANButtonFrame(15, 3);
  }
  if (frame.id == 396) {    
    if (bitRead(frame.data.bytes[1], 1) == 1) {      // Parking Brake Set 
        SerialUSB.print("Set Parking Brake\r\n");
        brake_position = 1;
        M2IO.Setpin_12VIO(4, ON);                     // Turn off brake indicator on dash
        CANButtonFrame(16, brake_position);          // Send status to CAN output function           
        if (brake_position != initial_brake_position) {
          SerialUSB.print("Brake status changed to set.\r\n");
          WriteSD(brake_position);
        }
    }
    if (bitRead(frame.data.bytes[1], 6) == 1) {        // Parking Brake Release
        SerialUSB.print("Release Parking Brake\r\n");
        brake_position = 0;
        M2IO.Setpin_12VIO(4, OFF);                    // Turn off brake indicator on dash
        CANButtonFrame(16, brake_position);          // Send status to CAN output function        
        if (brake_position != initial_brake_position) {
          SerialUSB.print("Brake status changed to released.\r\n");    
          WriteSD(brake_position);
      }
    }
  }
}

void loop() {
  
  static unsigned long lastTime = 0;

  CAN_FRAME incoming;
  if (Can0.available() > 0) {
    CANKeepAliveFrames();
    Can0.read(incoming);
    processFrame(incoming);
  }

}
