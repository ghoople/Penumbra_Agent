/* Penumbra_Agent

This code will run on an agent Arduino connected via UART TTL with the clear core.

It will: 
1. Receive data from the clear core
2. Generate signals for the LEDs in the control tower. 
3. Control the brightness of the halogen lights using DMX

*/ 

bool debug = false; // Set to true to print debug messages to the serial monitor.

// Dependencies
#include <Arduino.h>
#include <DmxSimple.h>
#include <SoftwareSerial.h>

#define Top 11732 // Should match the definition in Principal's main.h, but isn't being used now that we got rid of the LEDs. 

// Define Hardware Pins
// 0 Should not be used, reserved for USB Coms
// 1 Sould not be used, reserved for USB Coms
#define dmxDePin 2 // Must be pin 2 based on board.
//#define dmxRxPin 3 // I don't need to use this pin in transmit only mode. Signals from shield might conflict with the debugger.  
#define dmxTxPin 4 // Must be pin 4 based on jumpers on the DMX board
#define serRxPin 7 // Software Serial
#define serTxPin 8 // Software Serial

int position = 0; // Position of the motor, received from clear core
int position_old = 0; // Previous position

// DMX Variables
// Set DMX module to master mode
int channels = 4; // Number of DMX Channels on my system (probably 4)
int halogenA_DmxChan = 1; // Where the Halogen A bulb is plugged in. 
int halogenB_DmxChan = 2; // Where the Halogen B bulb is plugged in. 
int brightA = 0; // Brightness for the halogen A bulb, received from clear core
int brightA_old = 0; // Previous brightness
int brightB = 0; // Brightness for the halogen A bulb, received from clear core
int brightB_old = 0; // Previous brightness

// Serial Coms Variables
#define principalBaudRate 9600 // May need to go faster to keep up with updates from clear core? 
#define usbBaudRate 9600 // This can be slow.  
SoftwareSerial mySerial =  SoftwareSerial(serRxPin, serTxPin);

// Timer Variables
int32_t lastUpdateTime = millis();
int32_t updateInterval = 75; // How often to update the Halogen Lights.

void setup() {
  // DMX Setup
    // Set the DMX to Principal (Master) mode
    pinMode(dmxDePin,OUTPUT);
    digitalWrite(dmxDePin,HIGH);

    // Set the pin the module will use to transmit data to the DMX Cable
    DmxSimple.usePin(dmxTxPin);

    // Set the maximum number of channels I have on my system. 
    DmxSimple.maxChannel(channels);

  // Serial Coms
    mySerial.begin(principalBaudRate); // Software Serial
    Serial.begin(usbBaudRate); // Hardware Serial on USB port
    uint32_t serTimeout = 5000;
    uint32_t serStartTime = millis();
    while (!mySerial && !Serial && millis() - serStartTime < serTimeout) {
            continue;
    }
    if(debug){Serial.println("Setup complete");}
}

void loop() {
  // Check for new data every loop: 
  if (mySerial.available() > 0) {
    // Read the incoming message until a newline character is received
    String message = mySerial.readStringUntil('\n'); 
    // Parse the message into two integers separated by a comma
    sscanf(message.c_str(), "%d,%d,%d", &position, &brightA, &brightB);
    if(debug){
      Serial.print("Rx:");
      Serial.print(position); // Tell the Agent where the light is. 
      Serial.print(","); 
      Serial.print(brightA);// Tell the agent what the intensity should be for halA
      Serial.print(","); 
      Serial.println(brightB);// Tell the agent what the intensity should be for halB
    }
  }

  int32_t currentTime = millis();
// Only process updates every updateInterval ms
 if (currentTime - lastUpdateTime >= updateInterval) {  
    // Update the halogen brightness only when you receive new data:
    if (brightA != brightA_old) {
      DmxSimple.write(halogenA_DmxChan, brightA);
      if(debug){Serial.println("Set halogenA Brightness: " + String(brightA));}
    }

    // Update the halogen brightness only when you receive new data:
    if (brightB != brightB_old) {
      DmxSimple.write(halogenB_DmxChan, brightB);
      if(debug){Serial.println("Set halogenB Brightness: " + String(brightB));}
    }

    brightA_old = brightA;
    brightB_old = brightB;
    position_old = position; 
    
    lastUpdateTime = millis();
  }
}

