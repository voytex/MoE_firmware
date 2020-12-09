#include "MoE_Controller.h"


Controller controller;

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
