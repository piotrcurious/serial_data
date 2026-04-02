
// This is a code block for the sender sketch

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

// Define a variable for storing the LED state
bool ledState = false;

// Define a variable for storing the telemetry data
long telemetryData = 0;

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
  // Toggle the LED state every second
  ledState = !ledState;
  digitalWrite(LED_PIN, ledState);

  // Send a command packet to toggle the LED on the receiver
  serialPacket.sendPacket(COMMAND_PACKET, ledState ? LED_ON : LED_OFF);

  // Print a message to the serial monitor
  Serial.print("Sent command: ");
  Serial.println(ledState ? "LED ON" : "LED OFF");

  // Increment the telemetry data by a random value
  telemetryData += random(10, 100);

  // Send a data packet with the telemetry data as a LONG
  serialPacket.sendLong(DATA_PACKET, telemetryData);

  // Print a message to the serial monitor
  Serial.print("Sent telemetry (long): ");
  Serial.println((int)telemetryData);

  // Wait for one second
  delay(1000);
}
