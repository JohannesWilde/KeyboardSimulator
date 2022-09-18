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

namespace Internal
{

static size_t constexpr maximumMessageLength = 50;

char const string_0[] PROGMEM = "String 0";
char const string_1[] PROGMEM = "String 1";
char const string_2[] PROGMEM = "String 2";
char const string_3[] PROGMEM = "String 3";
char const string_4[] PROGMEM = "String 4";
char const string_5[] PROGMEM = "String 5";

char const * const array[] PROGMEM = {string_0, string_1, string_2, string_3, string_4, string_5};

} // namespace Internal

static size_t constexpr count = sizeof(Internal::array) / sizeof(Internal::array[0]);

char const * getMessage(size_t const index)
{
    static char buffer[Internal::maximumMessageLength + 1] = {0};
    strncpy_P(buffer, (char *)pgm_read_word(&(Internal::array[index])), Internal::maximumMessageLength);
    return buffer;
}

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
        enterSleepMode();
        buttonPressed = (LOW == digitalRead(Pins::button));

        if (buttonPressed)
        {
            bool longPress = false;
            {
                unsigned long const timePressed = millis();
                delay(50);
                while (LOW == digitalRead(Pins::button))
                {
                    // wait for the button to be released
                    unsigned long const now = millis();
                    if (250 < (now - timePressed))
                    {
                        longPress = true;
                        digitalWrite(Pins::led, HIGH);
                    }
                }
            }

            if (longPress)
            {
                // change the selected message

                unsigned long constexpr timeout = 2000;
                bool timedOut = false;
                size_t buttonPresses = 0;

                do
                {
                    {
                        // check whether pressed again
                        unsigned long const timeReleased = millis();
                        delay(50);
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
                        digitalWrite(Pins::led, LOW);
                        delay(50);
                        while ((LOW == digitalRead(Pins::button)))
                        {
                            if (500 < (millis() - timePressed))
                            {
                                digitalWrite(Pins::led, HIGH);
                            }
                        }
                        digitalWrite(Pins::led, HIGH);

                        buttonPresses = (buttonPresses + 1) % Messages::count;
                    }
                }
                while (!timedOut);

                digitalWrite(Pins::led, LOW);

                if (0 < buttonPresses)
                {
                    messageIndex = buttonPresses - 1;
                    EEPROM.put(EepromAddresses::selectedMessageIndex, messageIndex);
                }
            }
            else
            {
                // write out the message for a short press of the button
                slowKeyboard.print(Messages::getMessage(messageIndex));
                Keyboard.write(KEY_RETURN);
            }

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
