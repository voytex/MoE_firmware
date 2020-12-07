#include "MoE_Controller.h"
//#include <SoftwareSerial.h>

Controller controller;
//SoftwareSerial ms(4,5);
void setup()
{
  Serial.begin(9600);
  controller.initialize();
  controller.begin();
  controller.flashBeacon();
}
void loop()
{
  controller.handleMIDI();

  controller.handleUDP();
  
  controller.maintain();
}
