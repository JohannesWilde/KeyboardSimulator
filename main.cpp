/*
  Keyboard test

  For the Arduino Leonardo, Micro or Due

  Reads a byte from the serial port, sends a keystroke back.
  The sent keystroke is one higher than what's received, e.g. if you send a,
  you get b, send A you get B, and so forth.

  The circuit:
  - none

  created 21 Oct 2011
  modified 27 Mar 2012
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/KeyboardSerial
*/

// WIP: Only for development purposes, so that QtCreator correctly finds Serial and Keyboard instances.
// Look into arduino-1.8.19/hardware/tools/avr/avr/include/avr/io.h to find the correct macro.
#define __AVR_ATmega32U4__

#include <Arduino.h>
#include <Wire.h>

#include "SlowKeyboard.h"

namespace Pins
{

uint8_t constexpr button = 7; // PE6, Int4
uint8_t constexpr led = 11;

} // namespace Pin


static void buttonPressed()
{
    if (digitalRead(Pins::button) == HIGH)
    {
        digitalWrite(Pins::led, HIGH);
    }
    else
    {
        digitalWrite (Pins::led, LOW);
    }
}


void setup()
{
    pinMode(Pins::button, INPUT_PULLUP);
    pinMode(Pins::led, OUTPUT);

    attachInterrupt(digitalPinToInterrupt(Pins::button), buttonPressed, CHANGE);

    // initialize control over the keyboard:
    Keyboard_ & slowKeyboard = Keyboard;
//    slowKeyboard.minimumReportDelayUs = 8000;

    slowKeyboard.begin(KeyboardLayout_de_DE);

    // Wait for Arduino to be up and running.
//    delay(2000);
    for (unsigned index = 0; index < 2; ++index)
    {
        digitalWrite(Pins::led, HIGH);
        delay(500);
        digitalWrite(Pins::led, LOW);
        delay(500);
    }

    // Type "Hello World!".
    slowKeyboard.print("Hello, World!");

    Keyboard.write(KEY_RETURN);

    // finalize
    slowKeyboard.end();

//    interrupts();
}

void loop()
{
    // do nothing
}
