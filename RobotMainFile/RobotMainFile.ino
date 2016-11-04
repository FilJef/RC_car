#include <Utility.h> //for protothreading
#include <TimedAction.h>

#include <Servo.h> //for controlling the servo
#define CHNL1_IN 5 //input pins for RC remote
#define CHNL2_IN 6

#define BASE_INPUT 1500 //this is what the RC controller outputs when idling
#define MAX_INPUT 2000 //the highest output of the RC remote
#define MIN_INPUT 1000 //lowest RC output

#define MA_DIR 12 //Motor A pins
#define MA_PWM 3
#define MA_BRAKE 9

#define MB_DIR 13 // Motor B pins
#define MB_PWM 11
#define MB_BRAKE 8

Servo flipper;
bool flipState = LOW;
int channel1;
int channel2;

unsigned long timer = 0;

void checkFlip();

TimedAction flipperThread = TimedAction(200, *checkFlip);

void setup() {
  pinMode(CHNL2_IN, INPUT);// RC controller input pins
  pinMode(CHNL1_IN, INPUT);
  pinMode(MA_DIR, OUTPUT); //directional control pins
  pinMode(MB_DIR, OUTPUT);
  flipper.attach(A2);
  flipper.write(0);
  Serial.begin(9600);
}

void loop() {
  channel1 = pulseIn(CHNL1_IN, HIGH, 25000); //get the PWM signal for channels
  channel2 = pulseIn(CHNL2_IN, HIGH, 25000);
  movement();
  flipperThread.check();
}

void forward(bool dir, int spd) {
  digitalWrite(MA_DIR, dir); //direction
  digitalWrite(MB_DIR, !dir);
  motorSpeed(spd);
}

void turn(bool dir, int spd) {
  digitalWrite(MA_DIR, dir); //direction
  digitalWrite(MB_DIR, dir);

  motorSpeed(spd);
  brake(LOW);
}

void motorSpeed(int spd) { //sets the motor speed
  analogWrite(MA_PWM, spd);
  analogWrite(MB_PWM, spd);
}

void brake(bool set) { //sets the brakes
  digitalWrite(MA_BRAKE, set);
  digitalWrite(MB_BRAKE, set);
}

void movement() {
  if ((channel1 > BASE_INPUT + 50) || (channel1 < BASE_INPUT - 50)) { //check deadzone
    if (channel1 > BASE_INPUT) { //if its above turn left
      turn(HIGH, map(channel1, BASE_INPUT, MAX_INPUT, 0, 255));
    }
    else if (channel1 < BASE_INPUT && (channel1 != 0)) { //otherwise its below turn right
      turn(LOW, map(channel1, BASE_INPUT, MIN_INPUT,  0, 255));
    }
  }
  else { //if not turning then check if moving forward
    if ((channel2 > BASE_INPUT + 50) || (channel2 < BASE_INPUT - 50)) {//check deadzone
      if (channel2 > BASE_INPUT) { //if forwards then map the speed
        forward(HIGH, map(channel2, BASE_INPUT, MAX_INPUT, 0, 255));
      }
      else if (channel2 < 1050 && channel2 != 0){
        useFlip();
      }
      else if (channel2 < BASE_INPUT && (channel2 != 0)) { //else backwards and map speed, the != 0 is due to a hardware issue where input polls as 0
        forward(LOW, map(channel1, BASE_INPUT, MIN_INPUT, 0, 255));
      }
    }
    else { //if it is within +-50 of base then do nothing and brake motors
      forward(HIGH, 0);
    }
  }
}

void checkFlip(){
  if (flipState) { //reset the flipper
    flipper.write(0);
    flipState = LOW;
  }
}

void useFlip() { //use the servo
  if ((timer < (millis() - 200)) && (channel2 < 1100 && channel2 != 0)) {
    brake(HIGH);
    flipper.write(90);
    flipState = HIGH;
    timer = millis();
    brake(LOW);
  }

}

