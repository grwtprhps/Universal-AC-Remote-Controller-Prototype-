
/*
  Debounce

  Each time the input pin goes from LOW to HIGH (e.g. because of a push-button
  press), the output pin is toggled from LOW to HIGH or HIGH to LOW. There's a
  minimum delay between toggles to debounce the circuit (i.e. to ignore noise).

  The circuit:
  - LED attached from pin 13 to ground through 220 ohm resistor
  - pushbutton attached from pin 2 to +5V
  - 10 kilohm resistor attached from pin 2 to ground

  - Note: On most Arduino boards, there is already an LED on the board connected
    to pin 13, so you don't need any extra components for this example.

  created 21 Nov 2006
  by David A. Mellis
  modified 30 Aug 2011
  by Limor Fried
  modified 28 Dec 2012
  by Mike Walters
  modified 30 Aug 2016
  by Arturo Guadalupi

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce
*/
#include <PanasonicHeatpumpIR.h>
#include <DaikinTHCHeatpumpIR.h>

IRSenderBitBang irSender(3);  // IR led on Wemos D1 mini, connect between D2 and G
HeatpumpIR* heatpumpIR[] = { new PanasonicDKEHeatpumpIR(),
                             new DaikinTHCHeatpumpIR(),
                             NULL };
// constants won't change. They're used here to set pin numbers:
const int buttonPin = 4;  // the number of the pushbutton pin
const int ledPin = 13;    // the number of the LED pin

// Variables will change:
int ledState = HIGH;        // the current state of the output pin
int buttonState;            // the current reading from the input pin
int lastButtonState = LOW;  // the previous reading from the input pin

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  Serial.begin(115200);
  // pinMode(5, OUTPUT);
  pinMode(5, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  // set initial LED state
  // digitalWrite(ledPin, ledState);
}
bool lastbtnmode, btnmodenow;
unsigned long lastdebouncemode;

int i = 1;
void loop() {
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);
  bool btnmode = digitalRead(5);
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:
  if (btnmode != lastbtnmode) {
    lastdebouncemode = millis();
  }
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if (millis() - lastdebouncemode >= 50) {
    if (btnmode != btnmodenow) {
      btnmodenow = btnmode;
      if (btnmodenow == LOW) {
        i = (i==0) ? i=1: i=0;
        (i==0) ? Serial.println("mode AC Panasonic"): Serial.println("mode AC Daikin");
      }
    }
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;
      Serial.println("button: " + String(buttonState));
      // only toggle the LED if the new button state is HIGH
      if (buttonState == LOW) {
        ledState = !ledState;
        // Send the same IR command to all supported heatpumps
        Serial.print(F("Sending IR to "));

        const char* buf;
        buf = heatpumpIR[i]->model();
        // 'model' is a PROGMEM pointer, so need to write a byte at a time
        while (char modelChar = pgm_read_byte(buf++)) {
          Serial.print(modelChar);
        }
        Serial.print(F(", info: "));

        buf = heatpumpIR[i]->info();
        // 'info' is a PROGMEM pointer, so need to write a byte at a time
        while (char infoChar = pgm_read_byte(buf++)) {
          Serial.print(infoChar);
        }
        Serial.println();
        int swingcon = (i==0)? VDIR_AUTO:VDIR_ON;
        if (ledState == true) {
          Serial.println("menyalakan AC");
          heatpumpIR[i]->send(irSender, POWER_ON, MODE_COOL, FAN_AUTO, 23, swingcon, HDIR_AUTO);
          delay(50);
          if(i==0)heatpumpIR[i]->send(irSender, POWER_ON, MODE_COOL, FAN_AUTO, 23, swingcon, HDIR_AUTO);
          // tone(5, 523);
        } else {
          Serial.println("menonaktifkan AC");
          heatpumpIR[i]->send(irSender, POWER_OFF, MODE_COOL, FAN_AUTO, 23, swingcon, HDIR_AUTO);
          delay(50);
          if (i==0) heatpumpIR[i]->send(irSender, POWER_OFF, MODE_COOL, FAN_AUTO, 23, swingcon, HDIR_AUTO);

          // heatpumpIR[i]->send(irSender, POWER_OFF, MODE_COOL, FAN_AUTO, 23, VDIR_ON, HDIR_AUTO);
          // noTone(5);
        }
      }
    }
  }

  // set the LED:
  lastbtnmode = btnmode;
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}
