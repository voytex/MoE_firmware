#include "MoE_Controller.h"


Controller controller;

void setup() {
  controller.initialize();
  controller.beginUDP();
  controller.flashBeacon(); 

}
void loop() {
  controller.handleUDP();
  controller.maintain();

}
