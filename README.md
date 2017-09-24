

![enter image description here](https://github.com/EasySensors/BlindsActuatorNode/blob/master/photos/easyBlindsActuatorNode.png?raw=true)


### The Blinds Actuator Node is actuator node for NEMA17 style steppers used in 3D printers. The board based on [The Button Size Node](https://github.com/EasySensors/ButtonSizeNode) with HopeRF RFM69-HCW radio on board and Pololu DRV8824/5 Stepper driver microcontroller (EBAN-DRV8824-5)
 

<iframe width="480" height="270" src="https://www.youtube.com/embed/Nq-V5lzM5DM" frameborder="0" 
allowfullscreen></iframe>

  Best sutable for automating blinds. easyBlindsActuatorNode.ino  is the Arduino example sketch using [MySensors 2.0 ](https://www.mysensors.org/) API.  This is "work in progress" project although the code and the board are worknig nice. Will add some useful stuff like STL files for printing belt and pulley holders a bit later. 

  
------------------------------------------------------------------------


## Specification: ##

 - Dimensions 39mm x 90mm
 - Supply voltage up to 9-25 Volts depends on the motor specs 
 - FTDI  header for programming
 - Any NEMA17 4-wire motor
  
   **Button Size Node Onboard sensors and features:**
 - Temperature and humidity sensor Si7021 
 - High Accuracy Temperature Sensor ±0.4 °C (max), –10 to 85 °C
 - Precision Relative Humidity Sensor ± 3% RH (max), 0–80% RH
 - Light sensor BH1750,  spectral responsibility is approximately human eye response.
 - Authentication security - Atmel ATSHA204A Crypto Authentication Chip
 - External JDEC EPROM
 - Dualoptiboot bootloader. Implements over the air (OTA) firmware update ability
 - RFM69-HCW (high power version) or CW (low power consumption version) 433 MHz Radio transceiver
 - The Digital and Analog pins are 3.3 volts




------------------------------------------------------


![enter image description here](https://github.com/EasySensors/BlindsActuatorNode/blob/master/photos/setMotor.JPG?raw=true)

![enter image description here](https://github.com/EasySensors/BlindsActuatorNode/blob/master/photos/set1.JPG?raw=true)

![enter image description here](https://github.com/EasySensors/BlindsActuatorNode/blob/master/photos/setPulley.JPG?raw=true)

[**The board schematics Pdf link**](https://github.com/EasySensors/ButtonSizeNode/blob/master/pdf/ButtonSizeNode_ext1.pdf)



