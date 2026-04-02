
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

    // Basic packet methods
    void sendPacket(byte type, byte data);
    bool receivePacket(byte& type, byte& data);

    // Multi-byte typed methods
    void sendInt(byte type, int value);
    bool receiveInt(byte& type, int& value);
    void sendLong(byte type, long value);
    bool receiveLong(byte& type, long& value);
    void sendFloat(byte type, float value);
    bool receiveFloat(byte& type, float& value);

    // Buffer methods
    void sendBuffer(byte type, const byte* buffer, size_t length);
    bool receiveBuffer(byte& type, byte* buffer, size_t length);

    // Maintenance methods
    bool available();
    void resetStats();
    uint32_t getCorrectedErrors();
    uint32_t getUnrecoverableErrors();

  private:
    Stream& _serial;
    byte hammingEncode(byte nibble);
    byte hammingDecode(byte b, bool& error_corrected);

    byte syncBuffer[ENCODED_PACKET_SIZE];
    int syncBufferCount;

    Packet packetCache[PACKET_CACHE_SIZE];
    int cacheHead, cacheTail, cacheCount;

    uint32_t correctedErrors;
    uint32_t unrecoverableErrors;

    void pushCache(Packet p);
    void pushFrontCache(Packet p);
    Packet popCache();
    bool isCacheEmpty();
    bool receivePacketRaw(byte& type, byte& data);
};

#endif
