#include <Stepper.h>

const int stepsPerRevolution = 6400;

Stepper myStepper(stepsPerRevolution, 2, 3);
const int enPin = 4;
uint32_t f = 1000;  //Частота следования микрошагов
int motorSpeed = 0;
int stepCount = 0;  // number of steps the motor has taken

void setup() {
  // nothing to do inside the setup
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, 0);
  Serial.begin(9600);
}

uint32_t t = 1000000 / f / 2;

void loop() {

  for (motorSpeed = 1; motorSpeed <= 10; motorSpeed++) {
    //int sensorReading = analogRead(A0);
    //int motorSpeed = map(sensorReading, 0, 1023, 0, 100);
    Serial.println(motorSpeed);
    //Serial.println(sensorReading);
    delay(500);

    myStepper.setSpeed(abs(motorSpeed*10));

    myStepper.step(200);
  }
}
