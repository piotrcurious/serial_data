
#include "SerialPacket.h"

const byte errorCorrectionTable[] PROGMEM = {
    0x00, 0x08, 0x20, 0x02, 0x40, 0x04, 0x10, 0x01
};

SerialPacket::SerialPacket(Stream& stream)
    : _serial(stream), syncBufferCount(0), cacheHead(0), cacheTail(0), cacheCount(0),
      correctedErrors(0), unrecoverableErrors(0) {
    memset(syncBuffer, 0, sizeof(syncBuffer));
}

void SerialPacket::pushCache(Packet p) {
    if (cacheCount < PACKET_CACHE_SIZE) {
        packetCache[cacheTail] = p;
        cacheTail = (cacheTail + 1) % PACKET_CACHE_SIZE;
        cacheCount++;
    }
}

void SerialPacket::pushFrontCache(Packet p) {
    if (cacheCount < PACKET_CACHE_SIZE) {
        cacheHead = (cacheHead - 1 + PACKET_CACHE_SIZE) % PACKET_CACHE_SIZE;
        packetCache[cacheHead] = p;
        cacheCount++;
    }
}

Packet SerialPacket::popCache() {
    Packet p = {0, 0};
    if (cacheCount > 0) {
        p = packetCache[cacheHead];
        cacheHead = (cacheHead + 1) % PACKET_CACHE_SIZE;
        cacheCount--;
    }
    return p;
}

bool SerialPacket::isCacheEmpty() { return cacheCount == 0; }

void SerialPacket::sendPacket(byte type, byte data) {
    byte raw[RAW_PACKET_SIZE] = {type, data, (byte)(type ^ data)};
    byte encoded[ENCODED_PACKET_SIZE];
    for (int i = 0; i < RAW_PACKET_SIZE; i++) {
        encoded[i * 2] = hammingEncode(raw[i] >> 4);
        encoded[i * 2 + 1] = hammingEncode(raw[i] & 0x0F);
    }
    _serial.write(encoded, ENCODED_PACKET_SIZE);
}

bool SerialPacket::receivePacket(byte& type, byte& data) {
    if (!isCacheEmpty()) {
        Packet p = popCache();
        type = p.type; data = p.data;
        return true;
    }
    return receivePacketRaw(type, data);
}

bool SerialPacket::receivePacketRaw(byte& type, byte& data) {
    while (_serial.available() > 0 || syncBufferCount == ENCODED_PACKET_SIZE) {
        if (syncBufferCount < ENCODED_PACKET_SIZE && _serial.available() > 0) {
            syncBuffer[syncBufferCount++] = _serial.read();
        }
        if (syncBufferCount == ENCODED_PACKET_SIZE) {
            byte raw[RAW_PACKET_SIZE];
            bool packet_had_correction = false;
            for (int i = 0; i < RAW_PACKET_SIZE; i++) {
                bool c1 = false, c2 = false;
                raw[i] = (hammingDecode(syncBuffer[i * 2], c1) << 4) | hammingDecode(syncBuffer[i * 2 + 1], c2);
                if (c1 || c2) packet_had_correction = true;
            }
            if ((raw[0] ^ raw[1]) == raw[2]) {
                if (packet_had_correction) correctedErrors++;
                type = raw[0]; data = raw[1];
                syncBufferCount = 0;
                return true;
            } else {
                for (int i = 0; i < ENCODED_PACKET_SIZE - 1; i++) syncBuffer[i] = syncBuffer[i+1];
                syncBufferCount--;
            }
        }
    }
    return false;
}

void SerialPacket::sendInt(byte type, int value) {
    sendPacket(type, (byte)((value >> 8) & 0xFF));
    sendPacket(type, (byte)(value & 0xFF));
}

bool SerialPacket::receiveInt(byte& type, int& value) {
    byte t1, d1, t2, d2;
    unsigned long start = millis();
    if (receivePacket(t1, d1)) {
        while (millis() - start < 1000) {
            if (receivePacket(t2, d2)) {
                if (t1 == t2) {
                    type = t1;
                    value = (int)((uint16_t)(d1 << 8) | d2);
                    return true;
                } else pushCache({t2, d2});
            }
            if (!available() && syncBufferCount == 0) break;
        }
        pushFrontCache({t1, d1});
    }
    return false;
}

void SerialPacket::sendLong(byte type, long value) {
    sendPacket(type, (byte)((value >> 24) & 0xFF));
    sendPacket(type, (byte)((value >> 16) & 0xFF));
    sendPacket(type, (byte)((value >> 8) & 0xFF));
    sendPacket(type, (byte)(value & 0xFF));
}

bool SerialPacket::receiveLong(byte& type, long& value) {
    byte t1, t2, d[4];
    int count = 0;
    unsigned long start = millis();
    if (receivePacket(t1, d[0])) {
        count = 1;
        while (millis() - start < 1000 && count < 4) {
            if (receivePacket(t2, d[count])) {
                if (t1 == t2) count++;
                else pushCache({t2, d[count]});
            }
            if (!available() && syncBufferCount == 0) break;
        }
        if (count == 4) {
            type = t1;
            value = ((long)d[0] << 24) | ((long)d[1] << 16) | ((long)d[2] << 8) | (long)d[3];
            return true;
        } else {
            for (int i = count - 1; i >= 0; i--) pushFrontCache({t1, d[i]});
        }
    }
    return false;
}

void SerialPacket::sendFloat(byte type, float value) {
    union { float f; byte b[4]; } v; v.f = value;
    for (int i = 0; i < 4; i++) sendPacket(type, v.b[3-i]);
}

bool SerialPacket::receiveFloat(byte& type, float& value) {
    byte t1, t2, d[4];
    int count = 0;
    unsigned long start = millis();
    if (receivePacket(t1, d[0])) {
        count = 1;
        while (millis() - start < 1000 && count < 4) {
            if (receivePacket(t2, d[count])) {
                if (t1 == t2) count++;
                else pushCache({t2, d[count]});
            }
            if (!available() && syncBufferCount == 0) break;
        }
        if (count == 4) {
            type = t1;
            union { float f; byte b[4]; } v;
            for (int i = 0; i < 4; i++) v.b[3-i] = d[i];
            value = v.f;
            return true;
        } else {
            for (int i = count - 1; i >= 0; i--) pushFrontCache({t1, d[i]});
        }
    }
    return false;
}

void SerialPacket::sendBuffer(byte type, const byte* buffer, size_t length) {
    for (size_t i = 0; i < length; i++) sendPacket(type, buffer[i]);
}

bool SerialPacket::receiveBuffer(byte& type, byte* buffer, size_t length) {
    byte t1, t2, d;
    size_t count = 0;
    unsigned long start = millis();
    if (receivePacket(t1, d)) {
        buffer[count++] = d;
        while (millis() - start < 1000 && count < length) {
            if (receivePacket(t2, d)) {
                if (t1 == t2) buffer[count++] = d;
                else pushCache({t2, d});
            }
            if (!available() && syncBufferCount == 0) break;
        }
        if (count == length) {
            type = t1;
            return true;
        } else {
            for (int i = count - 1; i >= 0; i--) pushFrontCache({t1, buffer[i]});
        }
    }
    return false;
}

bool SerialPacket::available() { return !isCacheEmpty() || _serial.available() >= ENCODED_PACKET_SIZE; }
void SerialPacket::resetStats() { correctedErrors = 0; unrecoverableErrors = 0; }
uint32_t SerialPacket::getCorrectedErrors() { return correctedErrors; }
uint32_t SerialPacket::getUnrecoverableErrors() { return unrecoverableErrors; }

byte SerialPacket::hammingEncode(byte nibble) {
    byte d1 = (nibble >> 3) & 0x01, d2 = (nibble >> 2) & 0x01, d3 = (nibble >> 1) & 0x01, d4 = nibble & 0x01;
    byte p1 = d1 ^ d2 ^ d4, p2 = d1 ^ d3 ^ d4, p3 = d2 ^ d3 ^ d4;
    return (p1 << 6) | (p2 << 5) | (d1 << 4) | (p3 << 3) | (d2 << 2) | (d3 << 1) | d4;
}

byte SerialPacket::hammingDecode(byte b, bool& error_corrected) {
    byte p1 = (b >> 6) & 0x01, p2 = (b >> 5) & 0x01, d1 = (b >> 4) & 0x01;
    byte p3 = (b >> 3) & 0x01, d2 = (b >> 2) & 0x01, d3 = (b >> 1) & 0x01, d4 = b & 0x01;
    byte s1 = p1 ^ d1 ^ d2 ^ d4, s2 = p2 ^ d1 ^ d3 ^ d4, s3 = p3 ^ d2 ^ d3 ^ d4;
    byte syndrome = (s1 << 2) | (s2 << 1) | s3;
    if (syndrome != 0) {
        b ^= pgm_read_byte(&errorCorrectionTable[syndrome]);
        error_corrected = true;
        d1 = (b >> 4) & 0x01; d2 = (b >> 2) & 0x01; d3 = (b >> 1) & 0x01; d4 = b & 0x01;
    } else error_corrected = false;
    return (d1 << 3) | (d2 << 2) | (d3 << 1) | d4;
}
