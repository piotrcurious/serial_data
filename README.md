# SerialPacket Library

A robust Arduino library for serial data communication with Hamming(7,4) error correction and automatic packet synchronization.

## Features

- **Error Correction**: Uses Hamming(7,4) code to detect and correct single-bit errors in every nibble.
- **Packet Synchronization**: Automatically recovers from stream offsets using a sliding-window buffer.
- **Interleaved Packet Support**: Handled via an internal packet cache, allowing multi-byte data (like `int`) to be received even if other packets are interleaved.
- **Dependency Injection**: Works with any `Stream` implementation (HardwareSerial, SoftwareSerial, etc.).
- **Multi-byte Support**: Helpers for sending and receiving 16-bit integers.
- **Mock Environment**: Includes a C++ mock Arduino environment for local testing and development.

## API

### Constructor
```cpp
SerialPacket(Stream& stream);
```
Initializes the library with a serial stream.

### Methods
```cpp
void sendPacket(byte type, byte data);
```
Sends a single-byte data packet with a specified type.

```cpp
bool receivePacket(byte& type, byte& data);
```
Attempts to receive a single-byte data packet. Returns `true` if a valid packet was successfully decoded.

```cpp
void sendInt(byte type, int value);
```
Sends a 16-bit integer as two consecutive packets of the same type.

```cpp
bool receiveInt(byte& type, int& value);
```
Attempts to receive a 16-bit integer. It can handle interleaved packets of different types by caching them for subsequent `receivePacket` calls.

```cpp
bool available();
```
Returns `true` if there are cached packets or enough data in the serial stream to potentially form a packet.

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

The synchronization logic in `receivePacket` scans the incoming stream byte-by-byte until a valid 6-byte sequence passing the Hamming decoding and checksum is found.
