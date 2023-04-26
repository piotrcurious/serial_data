// This is a code block for the receiver sketch

// Include the SerialPacket library
#include <SerialPacket.h>

// Create a SerialPacket object
SerialPacket serialPacket;

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
  // Initialize the LED pin as output
  pinMode(LED_PIN, OUTPUT);

  // Initialize the serial monitor for debugging
  Serial.begin(9600);
}

// Loop function
void loop() {
  // Define variables for storing the packet type and data
  byte packetType;
  byte packetData;

  // Try to receive a packet from the sender
  bool success = serialPacket.receivePacket(packetType, packetData);

  // Check if the packet was received successfully
  if (success) {
    // Print a message to the serial monitor
    Serial.print("Received packet: ");
    Serial.print(packetType == COMMAND_PACKET ? "COMMAND" : "DATA");
    Serial.print(" ");
    Serial.println(packetData);

    // Check the packet type and act accordingly
    if (packetType == COMMAND_PACKET) {
      // The packet is a command, toggle the LED according to the data
      digitalWrite(LED_PIN, packetData == LED_ON ? HIGH : LOW);
    }
    else if (packetType == DATA_PACKET) {
      // The packet is data, do something with it (e.g. display it on an LCD)
      // For simplicity, we just print it to the serial monitor
      Serial.print("Telemetry data: ");
      Serial.println(packetData);
    }
  }
}
