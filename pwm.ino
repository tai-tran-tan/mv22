/*
  Fade

  This example shows how to fade an LED on pin 9 using the analogWrite()
  function.

  The analogWrite() function uses PWM, so if you want to change the pin you're
  using, be sure to use another PWM capable pin. On most Arduino, the PWM pins
  are identified with a "~" sign, like ~3, ~5, ~6, ~9, ~10 and ~11.

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Fade
*/

#include <Servo.h>

// the PWM pin the LED is attached to
const byte D0 = 16;
const byte D5 = 14;
Servo ESC;

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  pinMode(D0, OUTPUT);
  pinMode(D5, OUTPUT);
  ESC.attach(D5,1000,2000);
  Serial.begin(9600);
}
void setSpeed(int speed){
  int angle = map(speed, 0, 100, 0, 180); //Sets servo positions to different speeds
  ESC.write(angle); //angle
}

// the loop routine runs over and over again forever:
void loop() {
  if (Serial.available() > 0) {
    String str = Serial.readString();
    str.trim();
    brightness = str.toInt();
  }
  // set the brightness of pin 9:
  Serial.println(brightness);
  analogWrite(D0, brightness);
  setSpeed(brightness);

  // wait for 30 milliseconds to see the dimming effect
  delay(500);
}
