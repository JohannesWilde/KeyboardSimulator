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


#include "SlowKeyboard.h"

#include <Arduino.h>
#include <Wire.h>

#include <avr/sleep.h>


namespace Pins
{

static uint8_t constexpr button = 7; // PE6, Int4
static uint8_t constexpr led = 11;

} // namespace Pin

static Keyboard_ & slowKeyboard = Keyboard;

static bool volatile buttonPressed = false;

static void isrAwake()
{
    detachInterrupt(digitalPinToInterrupt(Pins::button));
    buttonPressed = true;
}

void enterSleepMode(void)
{
    attachInterrupt(digitalPinToInterrupt(Pins::button), isrAwake, LOW);
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
    sleep_mode();
    sleep_disable();
}

void setup()
{
    pinMode(Pins::button, INPUT_PULLUP);
    pinMode(Pins::led, OUTPUT);

    // initialize control over the keyboard:
//    slowKeyboard.minimumReportDelayUs = 8000;

    slowKeyboard.begin(KeyboardLayout_de_DE);

    delay(600);

    while (true)
    {
        enterSleepMode();

        if (buttonPressed)
        {
            slowKeyboard.print("Hello, World! or something other very extensive one should know about...");
            Keyboard.write(KEY_RETURN);

            buttonPressed = false;
        }
        else
        {
            // intentionally empty
        }
    }

    // finalize
    slowKeyboard.end();
}

void loop()
{
    // notify error condition
    digitalWrite(Pins::led, HIGH);
    delay(500);
    digitalWrite (Pins::led, LOW);
    delay(500);
}
