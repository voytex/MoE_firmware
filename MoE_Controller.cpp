#include <Arduino.h>
#include <EEPROM.h>
#include <EthernetUdp.h>
#include "MoE_Controller.h"

#define DEBUG

byte * Controller::macRead() {
  byte ret[6];
  for (int i = 0; i < 6; i++) {
    ret[i] = EEPROM.read(i);
  }
  return ret;
}

Controller::Controller() {
    
}

Controller::Controller(IPAddress forceIP) {
    //TODO: constructor with forced IP?
}

void Controller::initialize() {
    //_myMac = *macRead();
    #ifdef DEBUG
        Serial.println("Trying to obtain IP from DHCP...");
    #endif
    if (Ethernet.begin(_myMac)) {
    } else {
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            #ifdef DEBUG
                Serial.println("Ethernet shield not found!");
            #endif
            while(true) {
                //do nothing
                delay(1); 
            }  
        }  
    if (Ethernet.linkStatus() == LinkOFF) {
        #ifdef DEBUG
            Serial.println("Ethernet cable not connected!"); 
        #endif
    }
    #ifdef DEBUG
        Serial.println("Failed to obtain IP from DHCP, setting default values.");
    #endif
    Ethernet.begin(_myMac, _safeIP);
    }
    
    #ifdef DEBUG 
        Serial.print("Arduino adress: ");
        Serial.println(Ethernet.localIP());
    #endif
    
    _broadcastIP = Ethernet.localIP();
    _broadcastIP[3] = 255;

}

void Controller::beginUDP() {
    eUDP.begin(_moePort);
}

void Controller::flashBeacon() {
    eUDP.beginPacket(_broadcastIP, _moePort);
    eUDP.write(_beacon, sizeof(_beacon));
    eUDP.endPacket();
    Serial.println("packet sent");
}

void Controller::handleBeacon(IPAddress sender) {
    for (int i = 0; i < _numKD; i++)
    {
        if (_knownDevices[i] == sender[3]) {
            #ifdef DEBUG
                Serial.println("Device already saved");
            #endif
            return;
        }
    }
    _knownDevices[_numKD] = sender[3];
    _numKD++;
}

void Controller::handleUDP() {
    eUDP.parsePacket();
    eUDP.readByte(_incomingUDP, 4);

    switch (_incomingUDP[0])
    {
    case 0xFF:
        handleBeacon(eUDP.remoteIP());
        break;
    //TODO: ...  work in progress ...
    default:
        break;
    }
}

void Controller::maintain() {
    Ethernet.maintain();
}

void Controller::handleLocalMIDI() {
    //TODO: Handle local serial MIDI data
}

void Controller::handleUDPMIDI() {
    //TODO: handle incoming UDP MIDI data
}
