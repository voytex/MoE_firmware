#include <Arduino.h>
#include <EthernetUdp.h>
#include "MoE_Controller.h"


const byte _myMac[] = {0x00, 0xAA, 0xBB, 0xCD, 0xDE, 0x01};
const byte _beacon[] = {0xFF, 0x00, 0x00, 0x00};
IPAddress _myIP = IPAddress(192, 168, 1, 100);


Controller::Controller() {
    
}

Controller::Controller(IPAddress forceIP) {
    //TODO: constructor with forced IP?
}

void Controller::initialize() {
    #ifdef DEBUG
        Serial.println("Trying to obtain IP from DHCP...");
    #endif
    if (Ethernet.begin(_myMac)) {
        _myIP = Ethernet.localIP(); 
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
            Serial.println("Ethernet cable not connected!"); //never got this message, even with cable unplugged
        #endif
    }
    #ifdef DEBUG
        Serial.println("Failed to obtain IP from DHCP, setting default values.");
    #endif
    Ethernet.begin(_myMac, _myIP);
    }
    
    #ifdef DEBUG 
        Serial.print("Arduino adress: ");
        Serial.println(Ethernet.localIP());
    #endif

    _broadcastIP = _myIP;
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
    int packetSize = eUDP.parsePacket();
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

