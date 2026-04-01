
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

void test_sync_loss() {
    std::cout << "--- Synchronization Loss Test ---" << std::endl;
    SoftwareSerial::buffer.clear();
    SerialPacket packetHandler(mySerial);
    byte type = 0x01;
    byte data = 0x42;

    // Inject a junk byte to misalign the stream
    SoftwareSerial::buffer.push_back(0xFF);

    packetHandler.sendPacket(type, data);
    std::cout << "Sent junk byte + valid packet. Buffer size: " << SoftwareSerial::buffer.size() << std::endl;

    byte receivedType, receivedData;
    if (packetHandler.receivePacket(receivedType, receivedData)) {
        std::cout << "Packet received! Type: " << (int)receivedType << ", Data: " << (int)receivedData << std::endl;
        if (receivedType == type && receivedData == data) {
            std::cout << "SUCCESS: Recovered from synchronization loss." << std::endl;
        } else {
            std::cout << "FAILURE: Data mismatch." << std::endl;
        }
    } else {
        std::cout << "FAILURE: Could not recover from synchronization loss." << std::endl;
    }
}

void test_int_data() {
    std::cout << "--- Int Data Test ---" << std::endl;
    SoftwareSerial::buffer.clear();
    SerialPacket packetHandler(mySerial);
    byte type = 0x02;
    int data = 12345;

    packetHandler.sendInt(type, data);
    std::cout << "Sent int. Buffer size: " << SoftwareSerial::buffer.size() << std::endl;

    byte receivedType;
    int receivedValue;
    if (packetHandler.receiveInt(receivedType, receivedValue)) {
        std::cout << "Int received! Type: " << (int)receivedType << ", Value: " << receivedValue << std::endl;
        if (receivedType == type && receivedValue == data) {
            std::cout << "SUCCESS" << std::endl;
        } else {
            std::cout << "FAILURE: mismatch" << std::endl;
        }
    } else {
        std::cout << "FAILURE: not received" << std::endl;
    }
}

int main() {
    test_basic();
    test_error_correction();
    test_sync_loss();
    test_int_data();
    return 0;
}
