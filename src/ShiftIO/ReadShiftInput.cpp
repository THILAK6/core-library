#include "ShiftIO/ReadShiftInput.h"

ReadShiftInput::ReadShiftInput(int dataPin, int clockPin, int latchPin)
    : dataPin(dataPin), clockPin(clockPin), latchPin(latchPin), state{} {
    pinMode(dataPin, INPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(latchPin, OUTPUT);
}

ReadShiftInput::ReadShiftInput(){}

void ReadShiftInput::update() {
    digitalWrite(latchPin, LOW);
    delayMicroseconds(5);
    digitalWrite(latchPin, HIGH);
    delayMicroseconds(5);

    state.clear();

    for (int i = 0; i < numRegisters * 8; i++) {
        // Read one bit at a time
        bool bitVal = digitalRead(dataPin);
        Serial.print(bitVal ? "1" : "0");
        state.push_back(bitVal ? HIGH : LOW);

        digitalWrite(clockPin, HIGH);
        delayMicroseconds(5);
        digitalWrite(clockPin, LOW);
        delayMicroseconds(5);
    }
}

int ReadShiftInput::readBit(int bit) const {
    if (bit < 0 || bit >= state.size()) return LOW;
    return state[bit];
}

void ReadShiftInput::setRegisterCount(int count) {
    numRegisters = count;
    state.reserve(numRegisters * 8);
}
