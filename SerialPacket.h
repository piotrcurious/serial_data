
#ifndef SerialPacket_h
#define SerialPacket_h

#include <Arduino.h>

#define RAW_PACKET_SIZE 3
#define ENCODED_PACKET_SIZE 6
#define PACKET_CACHE_SIZE 8

struct Packet {
    byte type;
    byte data;
};

class SerialPacket {
  public:
    SerialPacket(Stream& stream);
    void sendPacket(byte type, byte data);
    bool receivePacket(byte& type, byte& data);

    void sendInt(byte type, int value);
    bool receiveInt(byte& type, int& value);

    void sendLong(byte type, long value);
    bool receiveLong(byte& type, long& value);

    void sendFloat(byte type, float value);
    bool receiveFloat(byte& type, float& value);

    bool available();

  private:
    Stream& _serial;
    byte hammingEncode(byte nibble);
    byte hammingDecode(byte b);

    byte syncBuffer[ENCODED_PACKET_SIZE];
    int syncBufferCount;

    // Fixed-size circular buffer for packet caching
    Packet packetCache[PACKET_CACHE_SIZE];
    int cacheHead;
    int cacheTail;
    int cacheCount;

    void pushCache(Packet p);
    void pushFrontCache(Packet p);
    Packet popCache();
    bool isCacheEmpty();

    bool receivePacketRaw(byte& type, byte& data);
};

#endif
