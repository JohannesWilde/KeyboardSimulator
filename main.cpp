/*
  Keaboard Simulator

  Sends one of the supplied messages as keyboard-inputs to the
  connected USB host.

  The message to be sent can be selected by long-pressing the button
  and afterwards cycling through them with normal button presses [starting
  from the first one].
  The selected message number will be remembered even when powered off.

  The circuit:
  Lily TTGO USB with button between MISO and GND [see drawing.svg].

  This code is in the public domain.
*/


#include "SlowKeyboard.h"

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>

#include <avr/pgmspace.h>
#include <avr/sleep.h>

#include <string.h>


namespace Messages
{

typedef void (*KeyboardFunction)(Keyboard_ & keyboard);

void keyboardFunction0(Keyboard_ & keyboard)
{
    keyboard.print(F("String 0"));
    keyboard.write(KEY_RETURN);
}

void keyboardFunction1(Keyboard_ & keyboard)
{
    keyboard.print(F("String 1"));
    keyboard.write(KEY_RETURN);
}

void keyboardFunction2(Keyboard_ & keyboard)
{
    keyboard.print(F("String 2"));
    keyboard.write(KEY_RETURN);
}

void keyboardFunction3(Keyboard_ & keyboard)
{
    keyboard.print(F("String 3"));
    keyboard.write(KEY_RETURN);
}

void keyboardFunction4(Keyboard_ & keyboard)
{
    keyboard.print(F("String 4"));
    keyboard.write(KEY_RETURN);
}

void keyboardFunction5(Keyboard_ & keyboard)
{
    keyboard.print(F("String 5"));
    keyboard.write(KEY_RETURN);
}

KeyboardFunction const array[] = {keyboardFunction0,
                                  keyboardFunction1,
                                  keyboardFunction2,
                                  keyboardFunction3,
                                  keyboardFunction4,
                                  keyboardFunction5};

static size_t constexpr count = sizeof(array) / sizeof(array[0]);

} // namespace Messages

namespace Pins
{

static uint8_t constexpr button = PIN_SPI_MISO; // PE6, Int4
static uint8_t constexpr led = LED_BUILTIN;

} // namespace Pin

namespace EepromAddresses
{

static uint8_t constexpr selectedMessageIndex = 0;

} // namespace EepromAddresses


static Keyboard_ & slowKeyboard = Keyboard;

static bool volatile buttonPressed = false;

static size_t messageIndex = 0;


ISR(PCINT0_vect)
{
    PCICR &= ~PCIE0; // Disable pin change interrupt for PCINT7..0 of Atmega 32u4.
}

void enterSleepMode(void)
{
    static_assert(PIN_SPI_MISO == Pins::button); // Make sure we are talking about the same pin here and everywhere else.
    PCIFR |= PCIF0; // Clear interrupt flag for PCINT7..0 of Atmega 32u4.
    PCICR |= PCIE0; // Enable pin change interrupt for PCINT7..0 of Atmega 32u4.
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();
    sleep_mode();
    sleep_disable();
}

void setup()
{
    PCMSK0 = PCINT3; // Enable pin change interrupt for PB3 [Arduino Pin PIN_SPI_MISO].
    pinMode(Pins::button, INPUT_PULLUP);
    pinMode(Pins::led, OUTPUT);

    EEPROM.get(EepromAddresses::selectedMessageIndex, messageIndex);
    if (Messages::count <= messageIndex)
    {
        messageIndex = 0;
    }

    // initialize control over the keyboard:
//    slowKeyboard.minimumReportDelayUs = 8000;

    slowKeyboard.begin(KeyboardLayout_de_DE);

    digitalWrite(Pins::led, HIGH);
    // Wait for the USB connection to become operational.
    delay(600);
    digitalWrite(Pins::led, LOW);

    while (true)
    {
        buttonPressed = (LOW == digitalRead(Pins::button));

        if (buttonPressed)
        {
            bool longPress = false;
            {
                unsigned long const timePressed = millis();
                delay(50); // debounce

                // Wait for the button to be released.
                while (LOW == digitalRead(Pins::button))
                {
                    unsigned long const now = millis();
                    // If the button is held low for more than 250ms consider it a long press.
                    if (250 < (now - timePressed))
                    {
                        longPress = true;
                        digitalWrite(Pins::led, HIGH);
                    }
                }
            }

            if (longPress)
            {
                // Update the selected message index.

                unsigned long constexpr timeout = 2000; // milliseconds
                bool timedOut = false;
                size_t buttonPresses = 0;

                do
                {
                    {
                        // check whether pressed again
                        unsigned long const timeReleased = millis();
                        delay(50); // debounce
                        while ((HIGH == digitalRead(Pins::button)) && !timedOut)
                        {
                            if (timeout < (millis() - timeReleased))
                            {
                                timedOut = true;
                            }
                        }
                    }

                    if (!timedOut)
                    {
                        // wait to be released again
                        unsigned long const timePressed = millis();
                        digitalWrite(Pins::led, LOW); // Turn off LED temporarily when pressed.
                        delay(50); // debounce
                        while ((LOW == digitalRead(Pins::button)))
                        {
                            // Turn LED back on after 500ms.
                            if (500 < (millis() - timePressed))
                            {
                                digitalWrite(Pins::led, HIGH);
                            }
                        }
                        digitalWrite(Pins::led, HIGH);

                        // Note button press.
                        // Please note that this will overflow ungracefully, so don't press the button more
                        // than std::numeric_limits<size_t>::max() times, i.e. 65535 for 16-bit.
                        buttonPresses += 1;
                    }
                }
                while (!timedOut);

                digitalWrite(Pins::led, LOW); // Selection finished, so turn off LED again.

                if (0 < buttonPresses)
                {
                    // Change to zero-based index and confine to available number of messages..
                    messageIndex = (buttonPresses - 1) % Messages::count;
                    // Remember messageIndex even after power off.
                    EEPROM.put(EepromAddresses::selectedMessageIndex, messageIndex);
                }
            }
            else
            {
                // write out the message for a short press of the button
                Messages::array[messageIndex](slowKeyboard);
            }

            buttonPressed = false;
        }
        else
        {
            // intentionally empty
        }
        
        // Conserve power by going to sleep now.
        enterSleepMode();
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
