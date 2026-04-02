// This is a code block for the receiver sketch

// Include the SerialPacket library
#include <SerialPacket.h>
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11);

// Create a SerialPacket object
SerialPacket serialPacket(mySerial);

// Define the pin for the LED
#define LED_PIN 13

// Define the packet types for data and command
#define DATA_PACKET 0x00
#define COMMAND_PACKET 0x01

// Define the command values for toggling LEDs
#define LED_ON 0x01
#define LED_OFF 0x00

// Setup function
void setup() {
  mySerial.begin(9600);
  // Initialize the LED pin as output
  pinMode(LED_PIN, OUTPUT);

  // Initialize the serial monitor for debugging
  Serial.begin(9600);
}

// Loop function
void loop() {
  // Try to receive telemetry as long
  byte packetType;
  long longValue;
  if (serialPacket.receiveLong(packetType, longValue)) {
      if (packetType == DATA_PACKET) {
          Serial.print("Received telemetry (long): ");
          Serial.println((int)longValue);
      }
  }

  // Try to receive command
  byte cmdType;
  byte cmdData;
  if (serialPacket.receivePacket(cmdType, cmdData)) {
      if (cmdType == COMMAND_PACKET) {
          Serial.print("Received command: ");
          Serial.println(cmdData == LED_ON ? "LED ON" : "LED OFF");
          digitalWrite(LED_PIN, cmdData == LED_ON ? HIGH : LOW);
      }
  }
}
