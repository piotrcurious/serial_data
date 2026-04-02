
#ifndef SerialPacket_h
#define SerialPacket_h

#include <Arduino.h>
#include <deque>

#define ENCODED_PACKET_SIZE 6

struct Packet {
    byte type;
    byte data;
};

class SerialPacket {
  public:
    // Take a Stream reference to allow any serial interface
    SerialPacket(Stream& stream);
    void sendPacket(byte type, byte data);
    bool receivePacket(byte& type, byte& data);

    // Helpers for multi-byte data
    void sendInt(byte type, int value);
    bool receiveInt(byte& type, int& value);

    // Returns true if enough bytes are available for at least one encoded packet
    bool available();

  private:
    Stream& _serial;
    byte hammingEncode(byte nibble);
    byte hammingDecode(byte b);

    byte syncBuffer[ENCODED_PACKET_SIZE];
    int syncBufferCount;

    // A small queue for packets to allow look-ahead/interleaving handling
    std::deque<Packet> packetCache;

    // Internal raw receive (no cache)
    bool receivePacketRaw(byte& type, byte& data);
};

#endif
