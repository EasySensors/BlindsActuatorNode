/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
**/

// Enable debug prints to serial monitor
#define MY_DEBUG

// Comment it out for Auto Node ID #
#define MY_NODE_ID 0xEE 
// Avoid battery drain if Gateway disconnected and the node sends more than MY_TRANSPORT_STATE_RETRIES times message.
#define MY_TRANSPORT_UPLINK_CHECK_DISABLED
#define MY_PARENT_NODE_IS_STATIC
#define MY_PARENT_NODE_ID 0

// Enable and select radio type attached
#define MY_RADIO_RFM69
#define MY_IS_RFM69HW
#define MY_RFM69_FREQUENCY RF69_433MHZ

//#define MY_RFM69_FREQUENCY   RFM69_868MHZ

#define MY_RFM69_NEW_DRIVER

//Enable OTA feature
//#define MY_OTA_FIRMWARE_FEATURE
//#define MY_OTA_FLASH_JDECID 0 //0x2020

//Enable Crypto Authentication to secure the node
//#define MY_SIGNING_ATSHA204
//#define  MY_SIGNING_REQUEST_SIGNATURES


#include <avr/wdt.h>
#ifdef  __AVR__
  #include <avr/power.h>
#endif

#include <MySensors.h>
#include <stdlib.h>

#define percentage 1 
#define calibrationButton 2 

#define LEDpin 4

//#define SPIFLASH_BLOCKERASE_32K   0x52
#define SPIFLASH_BLOCKERASE_32K   0xD8 // Redefine erase block and CHIPERASE commands here. so please keep these two lines AFTER #include <MySensors.h>
#define SPIFLASH_CHIPERASE        0x60

MyMessage msgPerc(percentage, V_PERCENTAGE);
MyMessage msgCalibrationButton(calibrationButton, V_LIGHT);

#include <Bounce2.h>
Bounce debouncerSw3 = Bounce(), debouncerSw2 = Bounce(), debouncerSw1 = Bounce(); 

// ----------------------- ----------------- Stepper setup
// http://www.airspayce.com/mikem/arduino/AccelStepper/classAccelStepper.html#a5dce13ab2a1b02b8f443318886bf6fc5 
 #include <AccelStepper.h>

// All the wires needed for full functionality
#define DIR 6
#define STEP 7
//Uncomment line to use enable/disable functionality
#define ENBL 5

// Define a stepper and the pins it will use
AccelStepper stepper(1,STEP,DIR); //AccelStepper::DRIVER

static unsigned long calibrationStartedMillis = 0;
int motorMaxSpeed = 900, motorAcceleration  = 9000;
char calibrationString[11]; 
const char calibrated[11] = "calibrated";

long stepsToRotateWhileCalibrating = 200; // keep it  even and more than 100
long calibrationOpenedPosition = 0, calibrationClosedPosition = 0, calibrationCurrentPosition = 0;
// ----------------------- ----------------- Stepper setup END

#include <EEPROM.h>

// http://playground.arduino.cc/Code/EEPROMWriteAnything
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
  //return 1;
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)  
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
  //return 1;
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}

void before() {
    // watchdog sets to 8 secs
    wdt_enable(WDTO_8S);     //
    //wdt_disable();

    //in case watchdog resets node - we do RFM69 reset here since VDD (power) is not disconnected while watchdog resets the node. Just in case!
    pinMode(9,OUTPUT);
    digitalWrite(9,HIGH);
    delay(10);
    digitalWrite(9,LOW);
    delay(10);

    pinMode(ENBL,OUTPUT);
    stepper.setMaxSpeed(motorMaxSpeed);
    stepper.setAcceleration(motorAcceleration);
    stepper.setEnablePin(ENBL);
    //digitalWrite(5,LOW);  // energize
    //digitalWrite(5,HIGH); // Off
    //stepper.disableOutputs(); // actually it is enable...
    stepper.setPinsInverted(false,false,true); //  ( bool  directionInvert = false,bool  stepInvert = false,bool  enableInvert = false ) 
    stepper.disableOutputs(); // deenergize coils

    // LED
    pinMode(LEDpin,OUTPUT);
    digitalWrite(LEDpin,HIGH); 
    delay(100);
    digitalWrite(LEDpin,LOW); 

    // External buttons
    pinMode(A0, INPUT_PULLUP);
    pinMode(A1, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    
    // Debouncing the button
    debouncerSw3.attach(A0);
    debouncerSw3.interval(20);
    debouncerSw2.attach(A1);
    debouncerSw2.interval(20);
    debouncerSw1.attach(3);
    debouncerSw1.interval(20);

    // read stored actuator Position values if calibrated before
    EEPROM_readAnything(240,calibrationString);
Serial.println("Read calibrationString"); Serial.println(calibrationString);

    if (strcmp(calibrationString, calibrated) == 0){
      EEPROM_readAnything(200,calibrationOpenedPosition);
      EEPROM_readAnything(204,calibrationClosedPosition);
      EEPROM_readAnything(210,calibrationCurrentPosition);
      stepper.setCurrentPosition(calibrationCurrentPosition);
    }
Serial.println("Read calibrationClosedPosition"); Serial.println(calibrationClosedPosition);
Serial.println("Read calibrationOpenedPosition"); Serial.println(calibrationOpenedPosition);
Serial.println("Read calibrationCurrentPosition"); Serial.println(calibrationCurrentPosition);

}

void setup() {

}

void presentation() {  
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Blinds Actuator node","1.0");

  // Register all sensors to gw (they will be created as child devices)
  present(percentage, S_LIGHT);
  present(calibrationButton, S_LIGHT);
}


void openPressed(){
  digitalWrite(LEDpin,1); 
  stepper.enableOutputs(); // energize coils
  if ((bool)loadState(calibrationButton)) {
    calibrationOpenedPosition = stepper.currentPosition() + stepsToRotateWhileCalibrating;
    stepper.moveTo(calibrationOpenedPosition);
    stepper.setSpeed(motorMaxSpeed);
    } 
    else if (stepper.distanceToGo() != 0){
      stepper.moveTo(stepper.currentPosition());
      EEPROM_writeAnything(210,stepper.currentPosition());
      Serial.println("Read calibrationCurrentPosition"); Serial.println(stepper.currentPosition());
    }
    else {
      stepper.moveTo(calibrationOpenedPosition);
      stepper.setSpeed(motorMaxSpeed);
      EEPROM_writeAnything(210,calibrationOpenedPosition);
      Serial.println("Read calibrationCurrentPosition"); Serial.println(stepper.currentPosition());
      }
}

void closePressed(){ // revolutions is the number of rotations for motor. 0 is full close
  digitalWrite(LEDpin,1); 
  stepper.enableOutputs(); // energize coils
  if ((bool)loadState(calibrationButton)) {
    //stepper.runToNewPosition(stepper.currentPosition()- stepsToRotateWhileCalibrating);
    //calibrationClosedPosition = stepper.currentPosition();
    calibrationClosedPosition = stepper.currentPosition() - stepsToRotateWhileCalibrating;
    stepper.moveTo(calibrationClosedPosition);
    stepper.setSpeed(motorMaxSpeed);
    } 
    else if (stepper.distanceToGo() != 0) {
      stepper.moveTo(stepper.currentPosition());
      EEPROM_writeAnything(210,stepper.currentPosition());
      Serial.println("Read calibrationCurrentPosition"); Serial.println(stepper.currentPosition());
    }
    else {
      stepper.moveTo(calibrationClosedPosition);
      stepper.setSpeed(motorMaxSpeed);
      EEPROM_writeAnything(210,calibrationClosedPosition);
      Serial.println("Read calibrationCurrentPosition"); Serial.println(stepper.currentPosition());
      }
}


void calibrationPressed(int buttonValue){
  // calibration button pressed.
  saveState(calibrationButton, buttonValue);
  if (buttonValue == 0){//calibration process finished - button is off
      EEPROM_writeAnything(200,calibrationOpenedPosition);
      EEPROM_writeAnything(204,calibrationClosedPosition);
      EEPROM_writeAnything(210,stepper.currentPosition());
      EEPROM_writeAnything(240,calibrated);
      digitalWrite(LEDpin,0); 
    } else       calibrationStartedMillis = millis();

Serial.println("Read calibrationClosedPosition"); Serial.println(calibrationClosedPosition);
Serial.println("Read calibrationOpenedPosition"); Serial.println(calibrationOpenedPosition);
Serial.println("Read calibrationCurrentPosition"); Serial.println(stepper.currentPosition());

}

static uint8_t   lastValueSw3 = HIGH, lastValueSw1 = HIGH, lastValueSw2 = HIGH;

void loop(){
  wdt_reset();

  debouncerSw3.update();
  if (debouncerSw3.read() == 0   && lastValueSw3 == 1) {
      closePressed(); //stepper.moveTo(0); //
      lastValueSw3 = 0;
    }
    else if (debouncerSw3.read() == 1){
      lastValueSw3 = 1;
    }

  debouncerSw2.update();
  if (debouncerSw2.read() == 0   && lastValueSw2 == 1) {
      openPressed(); //stepper.moveTo(0); //
      lastValueSw2 = 0;
    }
    else if (debouncerSw2.read() == 1){
      lastValueSw2 = 1;
    }

  debouncerSw1.update();
  if (debouncerSw1.read() == 0  && lastValueSw1 == 1) {
      calibrationPressed(loadState(calibrationButton)?0:1);
      //send(msgCalibrationButton.set(loadState(calibrationButton)),true);
      lastValueSw1 = 0;
    }
    else if (debouncerSw1.read() == 1){
      lastValueSw1 = 1;
    }
    
  // blink LED if calibration pressed
  // Check if we reach the max millis of 4,294,967,295
  static unsigned long lastBlinkMillis = 0, BlinkInterval = 100;
  if (millis() < lastBlinkMillis)  lastBlinkMillis = 0;
  if (millis() > (lastBlinkMillis + BlinkInterval)  && (bool)loadState(calibrationButton)){
    lastBlinkMillis = millis();
    digitalWrite(LEDpin,!digitalRead(LEDpin));    
  }


  // Disable Calibration mode after 1 minute since it keeps coils energized
  // Check if we reach the max millis of 4,294,967,295
  static unsigned long CalibrationResetInterval = 60000;
  if (millis() < calibrationStartedMillis) calibrationStartedMillis = 0;
  if (millis() > (calibrationStartedMillis + CalibrationResetInterval)  && (bool)loadState(calibrationButton)){
    send(msgCalibrationButton.set(0),true);
    calibrationPressed(0);
  }

  // deenergize coils 
  if (stepper.distanceToGo() == 0 && !(bool)loadState(calibrationButton)) {
    stepper.disableOutputs(); // deenergize coils
    digitalWrite(LEDpin,0); 
  }

  if (stepper.distanceToGo() != 0) stepper.runSpeedToPosition();   //stepper.run(); 

}


void receive(const MyMessage &message) {
  
    if (message.type == V_LIGHT && message.sensor == percentage) {
      if (message.getBool() == 1) closePressed(); //getUInt
      else if (message.getBool() == 0) openPressed(); //getUInt
    }
    
    if (message.type == V_PERCENTAGE && message.sensor == percentage) {
      int perc = message.getUInt();
      long Gap =  calibrationOpenedPosition - calibrationClosedPosition;
      long targetPostion = (long)(float(calibrationOpenedPosition - perc*(Gap/100) ) ); //  could be + 0.5 for faster "rounding" before casting to long
      digitalWrite(LEDpin,1); 
      stepper.enableOutputs(); // energize coils
      stepper.moveTo(targetPostion);
      stepper.setSpeed(motorMaxSpeed);
    }
  
    if (message.type == V_LIGHT && message.sensor == calibrationButton) calibrationPressed(message.getUInt());

Serial.print("Incoming change for sensor:");
Serial.print(message.sensor);
Serial.print(", New status: ");
Serial.println(message.getBool());

}
