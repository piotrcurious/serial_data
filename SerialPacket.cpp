
#include "SerialPacket.h"

// 3 bytes of raw data (type, data, checksum)
// Each byte becomes 2 nibbles, each nibble becomes 1 byte (Hamming 7,4)
// Total 6 bytes for the encoded packet
#define RAW_PACKET_SIZE 3
#define ENCODED_PACKET_SIZE 6

// Hamming(7,4) error correction table
// Syndrome = (s1, s2, s3)
// s1 = p1 ^ d1 ^ d2 ^ d4
// s2 = p2 ^ d1 ^ d3 ^ d4
// s3 = p3 ^ d2 ^ d3 ^ d4
// Bits in byte: 0 p1 p2 d1 p3 d2 d3 d4
const byte errorCorrectionTable[] PROGMEM = {
    0x00, // 000: No error
    0x08, // 001: p3 (bit 3) -> syndrome s3=1
    0x20, // 010: p2 (bit 5) -> syndrome s2=1
    0x02, // 011: d3 (bit 1) -> syndrome s2=1, s3=1
    0x40, // 100: p1 (bit 6) -> syndrome s1=1
    0x04, // 101: d2 (bit 2) -> syndrome s1=1, s3=1
    0x10, // 110: d1 (bit 4) -> syndrome s1=1, s2=1
    0x01  // 111: d4 (bit 0) -> syndrome s1=1, s2=1, s3=1
};

SerialPacket::SerialPacket(Stream& stream) : _serial(stream) {
}

void SerialPacket::sendPacket(byte type, byte data) {
    byte raw[RAW_PACKET_SIZE];
    raw[0] = type;
    raw[1] = data;
    raw[2] = type ^ data;

    byte encoded[ENCODED_PACKET_SIZE];
    for (int i = 0; i < RAW_PACKET_SIZE; i++) {
        encoded[i * 2] = hammingEncode(raw[i] >> 4);
        encoded[i * 2 + 1] = hammingEncode(raw[i] & 0x0F);
    }
    _serial.write(encoded, ENCODED_PACKET_SIZE);
}

bool SerialPacket::receivePacket(byte& type, byte& data) {
    if (_serial.available() >= ENCODED_PACKET_SIZE) {
        byte encoded[ENCODED_PACKET_SIZE];
        _serial.readBytes(encoded, ENCODED_PACKET_SIZE);

        byte raw[RAW_PACKET_SIZE];
        for (int i = 0; i < RAW_PACKET_SIZE; i++) {
            raw[i] = (hammingDecode(encoded[i * 2]) << 4) | hammingDecode(encoded[i * 2 + 1]);
        }

        if ((raw[0] ^ raw[1]) == raw[2]) {
            type = raw[0];
            data = raw[1];
            return true;
        }
    }
    return false;
}

byte SerialPacket::hammingEncode(byte nibble) {
    // nibble: 0 0 0 0 d1 d2 d3 d4
    byte d1 = (nibble >> 3) & 0x01;
    byte d2 = (nibble >> 2) & 0x01;
    byte d3 = (nibble >> 1) & 0x01;
    byte d4 = nibble & 0x01;

    byte p1 = d1 ^ d2 ^ d4;
    byte p2 = d1 ^ d3 ^ d4;
    byte p3 = d2 ^ d3 ^ d4;

    // Output byte: 0 p1 p2 d1 p3 d2 d3 d4
    return (p1 << 6) | (p2 << 5) | (d1 << 4) | (p3 << 3) | (d2 << 2) | (d3 << 1) | d4;
}

byte SerialPacket::hammingDecode(byte b) {
    // b: 0 p1 p2 d1 p3 d2 d3 d4
    byte p1 = (b >> 6) & 0x01;
    byte p2 = (b >> 5) & 0x01;
    byte d1 = (b >> 4) & 0x01;
    byte p3 = (b >> 3) & 0x01;
    byte d2 = (b >> 2) & 0x01;
    byte d3 = (b >> 1) & 0x01;
    byte d4 = b & 0x01;

    byte s1 = p1 ^ d1 ^ d2 ^ d4;
    byte s2 = p2 ^ d1 ^ d3 ^ d4;
    byte s3 = p3 ^ d2 ^ d3 ^ d4;

    byte syndrome = (s1 << 2) | (s2 << 1) | s3;
    if (syndrome != 0) {
        b ^= pgm_read_byte(&errorCorrectionTable[syndrome]);
        d1 = (b >> 4) & 0x01;
        d2 = (b >> 2) & 0x01;
        d3 = (b >> 1) & 0x01;
        d4 = b & 0x01;
    }

    return (d1 << 3) | (d2 << 2) | (d3 << 1) | d4;
}
