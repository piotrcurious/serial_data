
#include "mock_arduino/Arduino.h"
#include "mock_arduino/SoftwareSerial.h"
#include "SerialPacket.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>

SoftwareSerial mySerial(10, 11);

void test_basic() {
    std::cout << "--- Basic Test ---" << std::endl;
    SoftwareSerial::buffer.clear();
    SerialPacket p(mySerial);
    p.sendPacket(0x01, 0x42);
    byte t, d;
    if (p.receivePacket(t, d) && t == 0x01 && d == 0x42) std::cout << "SUCCESS" << std::endl;
    else std::cout << "FAILURE" << std::endl;
}

void test_error_stats() {
    std::cout << "--- Error Stats Test ---" << std::endl;
    SoftwareSerial::buffer.clear();
    SerialPacket p(mySerial);
    p.resetStats();
    p.sendPacket(0x01, 0x42);
    SoftwareSerial::buffer[0] ^= 0x01; // 1-bit error
    byte t, d;
    if (p.receivePacket(t, d) && p.getCorrectedErrors() == 1) std::cout << "SUCCESS" << std::endl;
    else std::cout << "FAILURE: corrected=" << p.getCorrectedErrors() << std::endl;
}

void test_buffer() {
    std::cout << "--- Buffer Test ---" << std::endl;
    SoftwareSerial::buffer.clear();
    SerialPacket p(mySerial);
    byte data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    p.sendBuffer(0x05, data, 4);
    byte t, r[4];
    if (p.receiveBuffer(t, r, 4) && t == 0x05 && memcmp(data, r, 4) == 0) std::cout << "SUCCESS" << std::endl;
    else std::cout << "FAILURE" << std::endl;
}

int main() {
    test_basic();
    test_error_stats();
    test_buffer();
    return 0;
}
