
#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <chrono>

typedef uint8_t byte;

#define LOW 0x0
#define HIGH 0x1
#define INPUT 0x0
#define OUTPUT 0x1

#define PROGMEM
#define pgm_read_byte(addr) (*(addr))

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
void delay(unsigned long ms);
long random(long min, long max);

unsigned long millis();
unsigned long micros();

class Stream {
public:
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
    virtual size_t readBytes(uint8_t* buf, size_t len) = 0;
    virtual size_t write(const uint8_t* buf, size_t len) = 0;
    virtual ~Stream() {}
};

class SerialMock {
public:
    void begin(unsigned long baud) {}
    void print(const char* s) { printf("%s", s); }
    void print(int n) { printf("%d", n); }
    void println(const char* s) { printf("%s\n", s); }
    void println(int n) { printf("%d\n", n); }
};

extern SerialMock Serial;

#endif
