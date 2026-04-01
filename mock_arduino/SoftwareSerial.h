
#ifndef SOFTWARE_SERIAL_H
#define SOFTWARE_SERIAL_H

#include "Arduino.h"
#include <deque>

class SoftwareSerial : public Stream {
public:
    SoftwareSerial(uint8_t rx, uint8_t tx) {}
    void begin(unsigned long baud) {}
    int available() override { return buffer.size(); }
    int read() override {
        if (buffer.empty()) return -1;
        uint8_t val = buffer.front();
        buffer.pop_front();
        return val;
    }
    size_t readBytes(uint8_t* buf, size_t len) override {
        size_t count = 0;
        while (count < len && !buffer.empty()) {
            buf[count++] = buffer.front();
            buffer.pop_front();
        }
        return count;
    }
    size_t write(const uint8_t* buf, size_t len) override {
        for (size_t i = 0; i < len; i++) {
            buffer.push_back(buf[i]);
        }
        return len;
    }

    // Static buffer to simulate shared serial line
    static std::deque<uint8_t> buffer;
};

#endif
