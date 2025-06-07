#ifndef SHIFT_REGISTER_165_H
#define SHIFT_REGISTER_165_H

#include <Arduino.h>
#include <vector>

class ReadShiftInput
{
public:
    ReadShiftInput();
    ReadShiftInput(int dataPin, int clockPin, int latchPin);
    void update();
    int readBit(int bit) const;
    void setRegisterCount(int count);

private:
    int8_t dataPin;
    int8_t clockPin;
    int8_t latchPin;
    int8_t numRegisters = 1;

    std::vector<int8_t> state;
};

#endif
