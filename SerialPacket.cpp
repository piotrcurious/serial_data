
// This is a code block for Arduino library

// Include the SoftwareSerial library
#include <SoftwareSerial.h>

// Define the pins for RX and TX
#define RX_PIN 10
#define TX_PIN 11

// Define the baud rate for serial communication
#define BAUD_RATE 9600

// Define the packet types for data and command
#define DATA_PACKET 0x00
#define COMMAND_PACKET 0x01

// Define the packet size in bytes
#define PACKET_SIZE 8

// Define the Hamming code parameters
#define HAMMING_N 7
#define HAMMING_K 4

// Create a SoftwareSerial object
SoftwareSerial mySerial(RX_PIN, TX_PIN);

// Create a class for the library
class SerialPacket {
  public:
    // Constructor
    SerialPacket() {
      // Initialize the serial communication
      mySerial.begin(BAUD_RATE);
    }

    // Method to send a packet
    void sendPacket(byte type, byte data) {
      // Create a packet array
      byte packet[PACKET_SIZE];

      // Set the first byte to the packet type
      packet[0] = type;

      // Set the second byte to the data
      packet[1] = data;

      // Calculate the checksum of the packet using XOR
      byte checksum = packet[0] ^ packet[1];

      // Set the third byte to the checksum
      packet[2] = checksum;

      // Encode the packet using Hamming code
      encodePacket(packet);

      // Write the packet to the serial port
      mySerial.write(packet, PACKET_SIZE);
    }

    // Method to receive a packet
    bool receivePacket(byte& type, byte& data) {
      // Check if there is data available on the serial port
      if (mySerial.available() >= PACKET_SIZE) {
        // Read a packet from the serial port
        byte packet[PACKET_SIZE];
        mySerial.readBytes(packet, PACKET_SIZE);

        // Decode the packet using Hamming code
        decodePacket(packet);

        // Calculate the checksum of the packet using XOR
        byte checksum = packet[0] ^ packet[1];

        // Check if the checksum matches the third byte of the packet
        if (checksum == packet[2]) {
          // The packet is valid, extract the type and data
          type = packet[0];
          data = packet[1];

          // Return true to indicate success
          return true;
        }
        else {
          // The packet is invalid, return false to indicate failure
          return false;
        }
      }
      else {
        // There is not enough data available on the serial port, return false to indicate failure
        return false;
      }
    }

  private:
    // Method to encode a packet using Hamming code
    void encodePacket(byte* packet) {
      // Create an array for storing the encoded bytes
      byte encoded[PACKET_SIZE];

      // Loop through each byte of the packet
      for (int i = 0; i < PACKET_SIZE; i++) {
        // Get the current byte of the packet
        byte b = packet[i];

        // Split the byte into two nibbles (4 bits each)
        byte highNibble = b >> 4;
        byte lowNibble = b & 0x0F;

        // Encode each nibble using Hamming code and store them in the encoded array
        encoded[i * 2] = hammingEncode(highNibble);
        encoded[i * 2 + 1] = hammingEncode(lowNibble);
      }

      // Copy the encoded array back to the packet array
      memcpy(packet, encoded, PACKET_SIZE);
    }

// Method to decode a packet using Hamming code
void decodePacket(byte* packet) {
  // Create an array for storing the decoded bytes
  byte decoded[PACKET_SIZE / 2];

  // Loop through each pair of bytes in the packet
  for (int i = 0; i < PACKET_SIZE / 2; i++) {
    // Get the current pair of bytes from the packet
    byte highByte = packet[i * 2];
    byte lowByte = packet[i * 2 + 1];

    // Decode each byte using Hamming code and combine them into a nibble
    byte highNibble = hammingDecode(highByte);
    byte lowNibble = hammingDecode(lowByte);

    // Merge the nibbles into a byte and store it in the decoded array
    decoded[i] = (highNibble << 4) | lowNibble;
  }

  // Copy the decoded array back to the packet array
  memcpy(packet, decoded, PACKET_SIZE / 2);
}

// Method to encode a nibble using Hamming code
byte hammingEncode(byte nibble) {
  // Create a variable for storing the encoded byte
  byte encoded = 0;

  // Set the data bits of the encoded byte according to the nibble
  encoded |= (nibble & 0x01) << 2; // D1 -> P3
  encoded |= (nibble & 0x02) << 1; // D2 -> P4
  encoded |= (nibble & 0x04) << 1; // D3 -> P5
  encoded |= (nibble & 0x08) << 1; // D4 -> P6

  // Calculate the parity bits of the encoded byte using XOR
  byte p1 = (encoded & 0x10) ^ (encoded & 0x08) ^ (encoded & 0x04); // P1 = D4 ^ D3 ^ D1
  byte p2 = (encoded & 0x10) ^ (encoded & 0x08) ^ (encoded & 0x02); // P2 = D4 ^ D3 ^ D2
  byte p3 = (encoded & 0x04) ^ (encoded & 0x02) ^ (encoded & 0x01); // P3 = D3 ^ D2 ^ D1

  // Set the parity bits of the encoded byte according to the calculated values
  encoded |= p1 << 6; // P1 -> P7
  encoded |= p2 << 5; // P2 -> P6
  encoded |= p3;      // P3 -> P1

  // Return the encoded byte
  return encoded;
}

// Method to decode a byte using Hamming code
byte hammingDecode(byte b) {
   // Create a variable for storing the decoded nibble
   byte decoded = 0;

   // Calculate the syndrome bits using XOR
   byte s1 = (b & 0x80) ^ (b & 0x40) ^ (b & 0x20) ^ (b & 0x10); // S1 = P7 ^ P6 ^ P5 ^ P4
   byte s2 = (b & 0x80) ^ (b & 0x40) ^ (b & 0x10) ^ (b & 0x08); // S2 = P7 ^ P6 ^ P4 ^ P3
   byte s3 = (b & 0x20) ^ (b & 0x10) ^ (b & 0x08) ^ (b & 0x04); // S3 = P5 ^ P4 ^ P3 ^ P2

   // Combine the syndrome bits into a value
   byte syndrome = s1 | s2 | s3;

   // Check if there is an error in the received byte
   if (syndrome != 0) {
     // There is an error, find and correct it using a lookup table
     b ^= pgm_read_byte(&errorCorrectionTable[syndrome]);
   }

// Get the data bits from the corrected byte and store them in the decoded nibble
decoded |= (b & 0x04) >> 2; // P3 -> D1
decoded |= (b & 0x08) >> 1; // P4 -> D2
decoded |= (b & 0x10) >> 1; // P5 -> D3
decoded |= (b & 0x20) >> 1; // P6 -> D4

// Return the decoded nibble
return decoded;
}

// A lookup table for error correction using Hamming code
// The index is the syndrome value, the value is the bit to flip
const byte errorCorrectionTable[] PROGMEM = {
0x00, // 000: No error
0x01, // 001: Error in P1
0x02, // 010: Error in P2
0x10, // 011: Error in D4
0x04, // 100: Error in P3
0x08, // 101: Error in D3
0x20, // 110: Error in D2
0x40, // 111: Error in D1
};
};


//Source: Conversation with Bing, 4/26/2023
//(1) Errors when uploading a sketch – Arduino Help Center. https://support.arduino.cc/hc/en-us/articles/4403365313810-Errors-when-uploading-a-sketch.
//(2) SoftwareSerial Library | Arduino Documentation. https://docs.arduino.cc/learn/built-in-libraries/software-serial/.
//(3) Arduino Serial Error Correction - Electrical Engineering Stack Exchange. https://electronics.stackexchange.com/questions/159583/arduino-serial-error-correction.
//(4) Troubleshooting 10 Common Errors in the Arduino IDE - MUO. https://www.makeuseof.com/arduino-ide-troubleshooting-common-errors/.
//(5) How to do serial communication between two boards?. https://arduinojson.org/v6/how-to/do-serial-communication-between-two-boards/.
