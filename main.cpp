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

void setup()
{
    // initialize control over the keyboard:
    Keyboard_ & slowKeyboard = Keyboard;
//    slowKeyboard.minimumReportDelayUs = 8000;

    slowKeyboard.begin(KeyboardLayout_de_DE);

    // Wait for Arduino to be up and running.
//    delay(2000);
    for (unsigned index = 0; index < 2; ++index)
    {
        pinMode(Pins::led, OUTPUT);
        digitalWrite(Pins::led, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(500);                       // wait for a second
        digitalWrite(Pins::led, LOW);    // turn the LED off by making the voltage LOW
        delay(500);                       // wait for a second
        pinMode(Pins::led, INPUT);
    }

    // Type "Hello World!".
    slowKeyboard.print("Hello, World!");

    Keyboard.write(KEY_RETURN);

    // finalize
    slowKeyboard.end();
}

void loop()
{
    // do nothing
}
