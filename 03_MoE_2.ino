#include "MoE_Controller.h"


Controller controller;

void setup() {
  Serial.begin(9600);
  controller.initialize();
  controller.beginUDP();
  controller.flashBeacon();
}
void loop() {
  //controller.handleUDP();
  //controller.maintain();

}
