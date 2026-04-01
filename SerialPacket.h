
#ifndef SerialPacket_h
#define SerialPacket_h

#include <Arduino.h>

#define ENCODED_PACKET_SIZE 6

class SerialPacket {
  public:
    // Take a Stream reference to allow any serial interface
    SerialPacket(Stream& stream);
    void sendPacket(byte type, byte data);
    bool receivePacket(byte& type, byte& data);

    // Helpers for multi-byte data
    void sendInt(byte type, int value);
    bool receiveInt(byte& type, int& value);

  private:
    Stream& _serial;
    byte hammingEncode(byte nibble);
    byte hammingDecode(byte b);

    byte syncBuffer[ENCODED_PACKET_SIZE];
    int syncBufferCount;
};

#endif
