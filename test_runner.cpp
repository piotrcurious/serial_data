
#include "mock_arduino/Arduino.h"
#include "mock_arduino/SoftwareSerial.h"
#include "SerialPacket.h"
#include <iostream>
#include <vector>

SoftwareSerial mySerial(10, 11);

void test_basic() {
    std::cout << "--- Basic Test ---" << std::endl;
    SoftwareSerial::buffer.clear();
    SerialPacket packetHandler(mySerial);
    byte type = 0x01;
    byte data = 0x42;

    packetHandler.sendPacket(type, data);
    std::cout << "Sent packet. Buffer size: " << SoftwareSerial::buffer.size() << std::endl;

    byte receivedType, receivedData;
    if (packetHandler.receivePacket(receivedType, receivedData)) {
        std::cout << "Packet received! Type: " << (int)receivedType << ", Data: " << (int)receivedData << std::endl;
        if (receivedType == type && receivedData == data) {
            std::cout << "SUCCESS" << std::endl;
        } else {
            std::cout << "FAILURE: mismatch" << std::endl;
        }
    } else {
        std::cout << "FAILURE: not received" << std::endl;
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

            // Injected 1-bit error in byte i at bit 'bit'
            SoftwareSerial::buffer[i] ^= (1 << bit);

            byte receivedType, receivedData;
            if (packetHandler.receivePacket(receivedType, receivedData)) {
                if (receivedType != type || receivedData != data) {
                    std::cout << "FAILURE: mismatch after correcting error at byte " << i << ", bit " << bit << std::endl;
                    failures++;
                }
            } else {
                std::cout << "FAILURE: not received after error at byte " << i << ", bit " << bit << std::endl;
                failures++;
            }
        }
    }
    if (failures == 0) {
        std::cout << "All 1-bit error cases PASSED." << std::endl;
    } else {
        std::cout << failures << " error cases FAILED." << std::endl;
    }
}

int main() {
    test_basic();
    test_error_correction();
    return 0;
}
