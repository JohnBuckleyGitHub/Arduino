 /*
  Arduino Starter Kit example
 Project 10  - Zoetrope

 This sketch is written to accompany Project 10 in the
 Arduino Starter Kit

 Parts required:
 two 10 kilohm resistors
 2 momentary pushbuttons
 one 10 kilohm potentiometer
 motor
 9V battery
 H-Bridge

 Created 13 September 2012
 by Scott Fitzgerald
 Thanks to Federico Vanzati for improvements

 http://www.arduino.cc/starterKit

 This example code is part of the public domain
 */

const int controlPin1 = 2; // connected to pin 7 on the H-bridge
const int controlPin2 = 3; // connected to pin 2 on the H-bridge
const int enablePin = 9;   // connected to pin 1 on the H-bridge
const int potPin = A0;  // connected to the potentiometer's output

int motorSpeed = 0; // speed of the motor


void setup() {
  Serial.begin(9600);
  Serial.println("Setup started");
  // intialize the inputs and outputs
   pinMode(controlPin1, OUTPUT);
   pinMode(controlPin2, OUTPUT);
   pinMode(enablePin, OUTPUT);

  // pull the enable pin LOW to start
   digitalWrite(enablePin, HIGH);
   digitalWrite(controlPin1, LOW);
   digitalWrite(controlPin2, HIGH);
   Serial.println("Setup finished");
}

void loop() {

  delay(1);


  // read the value of the pot and divide by 4 to get
  // a value that can be used for PWM
  motorSpeed = analogRead(potPin) / 4;
  Serial.println(motorSpeed);


  analogWrite(enablePin, motorSpeed);

}




