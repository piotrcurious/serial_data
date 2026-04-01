
#ifndef SerialPacket_h
#define SerialPacket_h

#include <Arduino.h>

class SerialPacket {
  public:
    // Take a Stream reference to allow any serial interface
    SerialPacket(Stream& stream);
    void sendPacket(byte type, byte data);
    bool receivePacket(byte& type, byte& data);

  private:
    Stream& _serial;
    byte hammingEncode(byte nibble);
    byte hammingDecode(byte b);
};

#endif
