//#include <SD.h>

// Including Arduino_Due_SD_HSCMI library also creates SD object (MassStorage class)
#include <Arduino_Due_SD_HSMCI.h>                       // This creates the object SD
#include "variant.h"
#include <due_can.h>

#include <Arduino.h>
#include <pins_arduino.h>
#include <stdint.h>

// We need to create FileStore object, we will be using it to open/create file and to close file.
byte initial_brake_position;

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);                         // Set onboard LED as output

  // Setup CANbus
  CANAutoBaud();
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



void CANButtonFrame(int ControlCase) {
  CAN_FRAME outgoing1 ;
    outgoing1.id = 397;
    outgoing1.extended = false;
    outgoing1.length = 3;
    if (ControlCase == 0) {
      outgoing1.data.byte[0] = 0x01;
      outgoing1.data.byte[1] = 0;
      outgoing1.data.byte[2] = 0;
    }
    else if (ControlCase == 1) {
      outgoing1.data.byte[0] = 0x02;
      outgoing1.data.byte[1] = 1;
      outgoing1.data.byte[2] = 0;
    }
    else if (ControlCase == 2) {
      outgoing1.data.byte[0] = 0x04;
      outgoing1.data.byte[1] = 1;
      outgoing1.data.byte[2] = 0;
    }
    else if (ControlCase == 3) {
      outgoing1.data.byte[0] = 0x08;
      outgoing1.data.byte[1] = 0;
      outgoing1.data.byte[2] = 0;
    }    
    else if (ControlCase == 4) {
      outgoing1.data.byte[0] = 0;
      outgoing1.data.byte[1] = 0;
      outgoing1.data.byte[2] = 0;
    }
    SerialUSB.print("Sending Button Frame ");
    SerialUSB.print("\r\n");  
  Can0.sendFrame(outgoing1);
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

// Freewheel Wireless Steering Controls Section

if (frame.id == 1280) {
      //SerialUSB.print("Steering Control Frame ID'd ");
      //SerialUSB.print("\r\n");  
    if (bitRead(frame.data.bytes[0], 7) == 1 ) {
      SerialUSB.print("Left Turn Button Pressed ");
      SerialUSB.print("\r\n");
      CANButtonFrame(0);
    }
    else if ((bitRead(frame.data.bytes[0], 6)) == 1 ) {
      SerialUSB.print("Right Turn Button Pressed ");
      SerialUSB.print("\r\n");
      CANButtonFrame(1);
    }
    else if ((bitRead(frame.data.bytes[0], 5)) == 1 ) {
      SerialUSB.print("Headlight Button Pressed ");
      SerialUSB.print("\r\n");
      CANButtonFrame(2);
    }
    else if ((bitRead(frame.data.bytes[0], 4)) == 1 ) {
      SerialUSB.print("Horn Button Pressed ");
      SerialUSB.print("\r\n");
      CANButtonFrame(3);
    }
    else
      CANButtonFrame(4);         
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
