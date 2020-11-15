#include <Arduino.h>
#include <EEPROM.h>
#include <EthernetUdp.h>
#include "MoE_Controller.h"

#define DEBUG

void Controller::macLoad(byte *mac) {
  //since I know I will read Mac Address, I can hard-write 6 in there
  for (int i = 0; i < 6; i++) {
    mac[i] = EEPROM.read(i);
  }
}

Controller::Controller() {
    
}

Controller::Controller(IPAddress forceIP) {
    //TODO: constructor with forced IP?
}

void Controller::initialize() {
    macLoad(_myMac);
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
}

void Controller::handleBeacon(IPAddress sender) {
    /*/??????????????????????????????????????????
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
    //??????????????????????????????????????????*/

    patchbay.addSubscription(17, sender[3], 42);
    patchbay.printSubs();   
}

void Controller::handleUDP() {
    if (eUDP.parsePacket()) {        
      eUDP.readByte(_incomingUDP, 4);
  
      switch (_incomingUDP[0])
      {
      case 'A':
          handleBeacon(eUDP.remoteIP());
          break;
      //TODO: ...  work in progress ...
      default:
          break;
      }
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


Patchbay::Patchbay() {

}

int Patchbay::addSubscription(byte srcCh, byte dstIPnib, byte dstCh) {
    if (_numSubs >= MAX_SUBS) {
        return -1;
    }

    byte srcdstCh;
    if ((srcCh > 15) || (dstCh > 15)) {
        return -2;
    } else {
         srcdstCh = (srcCh << 4) | dstCh;
    }

    for (byte i = 0; i < MAX_SUBS; i++)
    {
        if ((dstIPnib == _subscriptions[i].dstIPnib) && (srcdstCh == _subscriptions[i].srcdstChannel));
    }
    
    _subscriptions[_numSubs].dstIPnib = dstIPnib;
    _subscriptions[_numSubs].srcdstChannel = srcdstCh;
    _numSubs++;
    return 1;
}

void Patchbay::printSubs() {
    for (byte i = 0; i < _numSubs; i++)
    {
        Serial.print((_subscriptions[i].srcdstChannel & 0xF0) >> 4, DEC);
        Serial.print("\t -> \t");
        Serial.print("192.168.1.");
        Serial.print(_subscriptions[i].dstIPnib, DEC);
        Serial.print(": channel = ");
        Serial.println((_subscriptions[i].srcdstChannel & 0x0F), DEC);
    }
    
}
