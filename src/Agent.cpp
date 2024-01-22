/* Penumbra_Agent

This code will run on an agent Arduino connected via UART TTL with the clear core.

It will: 
1. Receive data from the clear core
2. Generate signals for the LEDs in the control tower. 
3. Control the brightness of the halogen lights using DMX

*/ 

// Dependencies
#include <Arduino.h>
#include <FastLED.h>
#include <DmxSimple.h>
#include <SoftwareSerial.h>

// Define Hardware Pins
// 0 Should not be used, reserved for USB Coms
// 1 Sould not be used, reserved for USB Coms
#define dmxDePin 2 // Must be pin 2 based on board.
//#define dmxRxPin 3 // Must be pin 2 based on board. (I don't need to use this one in transmit only mode, but can't use for other stuff)
#define dmxTxPin 4 // Must be pin 4 based on jumpers on the DMX board
#define ledPin 6  // Defines the pin to which the LED strip is connected
#define serRxPin 7 // Software Serial
#define serTxPin 8 // Software Serial


// Initialize LED Variables
#define numLeds 144 // Define the number of LEDs on the strip
CRGB leds[numLeds]; // An array of CRGB colors to pass to the strip. 
int position = 0; // Position of the motor, received from clear core
int position_old = 0; // Previous position

// DMX Variables
int channels = 4; // Number of DMX Channels on my system (probably 4)
int HalogenA_DmxChan = 2; // Where the Halogen A bulb is plugged in. 
int brightnessA = 0; // Brightness for the halogen A bulb, received from clear core
int brightnesA_old = 0; // Previous brightness
// Probably will want to add brightness B here. 

// Serial Coms Variables
#define principalBaudRate 28800 // May need to go faster to keep up with updates from clear core? 
#define usbBaudRate 9600 // This can be slow.  
SoftwareSerial mySerial =  SoftwareSerial(serRxPin, serTxPin);

void setup() {

  // LED Setup
    FastLED.addLeds<WS2812, ledPin, GRB>(leds, numLeds); // Set up FastLED, need to adapt to whatever LED string I buy. 

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
    Serial.println("<Setup complete>");
}

void loop() {
  
  // Check for new data: 

  if (mySerial.available() > 0) {
    // Read the incoming message until a newline character is received
    String message = mySerial.readStringUntil('\n'); 
    Serial.println("Received Message");
    // Parse the message into two integers separated by a comma
    sscanf(message.c_str(), "%d,%d", &position, &brightnessA);
    Serial.print(position);
    Serial.print(" and ");
    Serial.println(brightnessA);
  }
  
  // Update the led show only when you receive data:
  if (position != position_old) {
    
    fill_solid(leds, numLeds, CRGB::Black); // Set the LED array to all black
    leds[position] = CRGB::White; // Set the LED at the current position to white

    // Show the updated LED colors
    FastLED.show();

    Serial.print("New Position Received: ");
    Serial.println(brightnessA);
  }

  // Update the halogen brightness only when you receive new data:
  if (brightnessA != brightnesA_old) {
    
    DmxSimple.write(HalogenA_DmxChan, brightnessA);

    Serial.print("New Brightness Received: ");
    Serial.println(brightnessA);

  }

  // Add Code for brightness2 if we go that route. 

  brightnesA_old = brightnessA;
  position_old = position; 

}

