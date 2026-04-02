
#include "mock_arduino/Arduino.h"
#include "mock_arduino/SoftwareSerial.h"
#include "SerialPacket.h"
#include <iostream>
#include <vector>
#include <cmath>

SoftwareSerial mySerial(10, 11);

void test_basic() {
    std::cout << "--- Basic Test ---" << std::endl;
    SoftwareSerial::buffer.clear();
    SerialPacket packetHandler(mySerial);
    byte type = 0x01;
    byte data = 0x42;
    packetHandler.sendPacket(type, data);
    byte rT, rD;
    if (packetHandler.receivePacket(rT, rD) && rT == type && rD == data) {
        std::cout << "SUCCESS" << std::endl;
    } else {
        std::cout << "FAILURE" << std::endl;
    }
}

void test_error_correction() {
    std::cout << "--- Error Correction Test ---" << std::endl;
    SerialPacket packetHandler(mySerial);
    byte type = 0x01;
    byte data = 0x42;
    int failures = 0;
    for (int i = 0; i < 6; i++) {
        for (int bit = 0; bit < 7; bit++) {
            SoftwareSerial::buffer.clear();
            packetHandler.sendPacket(type, data);
            SoftwareSerial::buffer[i] ^= (1 << bit);
            byte rT, rD;
            if (!(packetHandler.receivePacket(rT, rD) && rT == type && rD == data)) {
                failures++;
            }
        }
    }
    if (failures == 0) std::cout << "All 1-bit error cases PASSED." << std::endl;
    else std::cout << failures << " error cases FAILED." << std::endl;
}

void test_long_data() {
    std::cout << "--- Long Data Test ---" << std::endl;
    SoftwareSerial::buffer.clear();
    SerialPacket packetHandler(mySerial);
    byte type = 0x03;
    long data = 0x12345678;
    packetHandler.sendLong(type, data);
    byte rT;
    long rV;
    if (packetHandler.receiveLong(rT, rV) && rT == type && rV == data) {
        std::cout << "SUCCESS" << std::endl;
    } else {
        std::cout << "FAILURE: got " << rV << std::endl;
    }
}

void test_float_data() {
    std::cout << "--- Float Data Test ---" << std::endl;
    SoftwareSerial::buffer.clear();
    SerialPacket packetHandler(mySerial);
    byte type = 0x04;
    float data = 3.14159f;
    packetHandler.sendFloat(type, data);
    byte rT;
    float rV;
    if (packetHandler.receiveFloat(rT, rV) && rT == type && std::abs(rV - data) < 0.0001f) {
        std::cout << "SUCCESS" << std::endl;
    } else {
        std::cout << "FAILURE: got " << rV << std::endl;
    }
}

int main() {
    test_basic();
    test_error_correction();
    test_long_data();
    test_float_data();
    return 0;
}
