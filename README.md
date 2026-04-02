# SerialPacket Library

A robust Arduino library for serial data communication with Hamming(7,4) error correction and automatic packet synchronization.

## Features

- **Error Correction**: Uses Hamming(7,4) code to detect and correct single-bit errors in every nibble.
- **Packet Synchronization**: Automatically recovers from stream offsets using a sliding-window buffer.
- **Interleaved Packet Support**: Handled via an internal packet cache, allowing multi-byte data (like `int`, `long`, `float`, `buffer`) to be received even if other packets are interleaved.
- **Dependency Injection**: Works with any `Stream` implementation (HardwareSerial, SoftwareSerial, etc.).
- **Multi-type Support**: Helpers for sending and receiving `int` (16-bit), `long` (32-bit), `float` (32-bit), and raw byte buffers.
- **Error Statistics**: Track corrected and unrecoverable errors during communication.
- **Mock Environment**: Includes a C++ mock Arduino environment for local testing and development.

## API

### Constructor
```cpp
SerialPacket(Stream& stream);
```
Initializes the library with a serial stream.

### Basic Methods
```cpp
void sendPacket(byte type, byte data);
bool receivePacket(byte& type, byte& data);
bool available();
```

### Multi-byte Methods
```cpp
// 16-bit Integer
void sendInt(byte type, int value);
bool receiveInt(byte& type, int& value);

// 32-bit Long
void sendLong(byte type, long value);
bool receiveLong(byte& type, long& value);

// 32-bit Float
void sendFloat(byte type, float value);
bool receiveFloat(byte& type, float& value);

// Byte Buffer
void sendBuffer(byte type, const byte* buffer, size_t length);
bool receiveBuffer(byte& type, byte* buffer, size_t length);
```

### Maintenance and Stats
```cpp
void resetStats();
uint32_t getCorrectedErrors();
uint32_t getUnrecoverableErrors();
```

## Testing Locally

This repository includes a mock Arduino environment that allows you to run tests on your host machine without hardware.

### Prerequisites
- A C++ compiler (e.g., `g++`)

### Running Tests
To compile and run the included test suite:
```bash
g++ -I. -Imock_arduino test_runner.cpp SerialPacket.cpp mock_arduino/mock_arduino.cpp -o test_runner
./test_runner
```

## Protocol Details

Each byte of raw data (including the type and a XOR checksum) is split into two nibbles. Each nibble is encoded into a 7-bit Hamming code and stored in a full byte.
- **Raw Packet**: `[Type] [Data] [Checksum (Type ^ Data)]` (3 bytes)
- **Encoded Packet**: 6 bytes (2 bytes per raw byte)

The synchronization logic in `receivePacket` scans the incoming stream byte-by-byte until a valid 6-byte sequence passing the Hamming decoding and checksum is found. Multi-byte types are sent as a series of single-byte packets of the same `type`.
