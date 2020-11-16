#include <Arduino.h>
#include <EEPROM.h>
#include <EthernetUdp.h>
#include <SoftwareSerial.h>
#include "MoE_Controller.h"

#define DEBUG

void Controller::macLoad(byte *mac)
{
    //since I know I will read Mac Address, I can hard-write 6 in there
    for (byte i = 0; i < 6; i++)
    {
        mac[i] = EEPROM.read(i);
    }
}

Controller::Controller() : midiSerial(2, 3)
{
}

Controller::Controller(IPAddress forceIP) : midiSerial(2, 3)
{
    //TODO: constructor with forced IP?
}

void Controller::initialize()
{
    macLoad(_myMac);
#ifdef DEBUG
    Serial.println("Trying to obtain IP from DHCP...");
#endif
    if (Ethernet.begin(_myMac))
    {
    }
    else
    {
        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
#ifdef DEBUG
            Serial.println("Ethernet shield not found!");
#endif
            while (true)
            {
                //do nothing
                delay(1);
            }
        }
        if (Ethernet.linkStatus() == LinkOFF)
        {
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

void Controller::begin()
{
    eUDP.begin(MOE_PORT);

    midiSerial.begin(31250);
}

void Controller::flashBeacon()
{
    eUDP.beginPacket(_broadcastIP, MOE_PORT);
    eUDP.write(_beacon, sizeof(_beacon));
    eUDP.endPacket();
}

void Controller::handleBeacon(IPAddress sender)
{
    addSubscription(2, sender[3], 4);
    printSubs();
}

void Controller::handleUDP()
{
    if (eUDP.parsePacket())
    {
        eUDP.readByte(_incomingUDP, 4);
        for (byte i = 0; i < 4; i++)
        {
            Serial.println(_incomingUDP[i], BIN);
        }
        switch (_incomingUDP[0])
        {
        case 'A':
            handleBeacon(eUDP.remoteIP());
            break;
        case 0xAA:
            Serial.println(_incomingUDP[1], BIN);
            Serial.println(_incomingUDP[2], BIN);
            Serial.println(_incomingUDP[3], BIN);
            break;
        
        //TODO: ...  work in progress ...
        default:
            break;
        }
    }
}

void Controller::maintain()
{
    Ethernet.maintain();
}

void Controller::handleMIDI()
{
    int numIncBytes = midiSerial.available();
    if (numIncBytes)
    {
        switch (numIncBytes)
        {
        case 2:
        {
            byte data0, data1;
            data0 = midiSerial.read();
            data1 = midiSerial.read();
            sendUDP(data0, data1, false, 0);
            break;
        }
        case 3:
        {
            byte data0, data1, data2;
            data0 = midiSerial.read();
            data1 = midiSerial.read();
            data2 = midiSerial.read();
            // Serial.println(data0, BIN);
            // Serial.println(data1, BIN);
            // Serial.println(data2, BIN);
            sendUDP(data0, data1, true, data2);
            break;
        }
        }
    }
}

void Controller::sendUDP(byte data0, byte data1, bool threeByte, byte data2)
{
    byte srcCh = data0 & 0x0F;
    for (byte i = 0; i < MAX_SUBS; i++)
    {
        if (srcCh == (_subscriptions[i].srcdstChannel & 0xF0) >> 4)
        {
            Serial.println(data0, BIN);
            data0 = data0 & 0xF0;
            Serial.println(data0, BIN);
            data0 = data0 | (_subscriptions[i].srcdstChannel & 0x0F);
            IPAddress remoteIP = Ethernet.localIP();
            remoteIP[3] = _subscriptions[i].dstIPnib;
            eUDP.beginPacket(remoteIP, MOE_PORT);
            eUDP.write(0xAA);
            eUDP.write(data0);
            Serial.println(data0, BIN);
            eUDP.write(data1);
            if (threeByte)
                eUDP.write(data2);
            eUDP.endPacket();
            Serial.println("sent MIDI mesage!");
        }
    }
}

int Controller::addSubscription(byte srcCh, byte dstIPnib, byte dstCh)
{
    if (_numSubs >= MAX_SUBS)
    {
        return -1;
    }

    byte srcdstCh;
    if ((srcCh > 15) || (dstCh > 15))
    {
        return -2;
    }
    else
    {
        srcdstCh = (srcCh << 4) | dstCh;
    }

    for (byte i = 0; i < MAX_SUBS; i++)
    {
        if ((dstIPnib == _subscriptions[i].dstIPnib) && (srcdstCh == _subscriptions[i].srcdstChannel))
        {
            return 0;
        }
    }

    _subscriptions[_numSubs].dstIPnib = dstIPnib;
    _subscriptions[_numSubs].srcdstChannel = srcdstCh;
    _numSubs++;
    return 1;
}

int Controller::delSubscription(byte srcCh, byte dstIPnib, byte dstCh)
{
    byte srcdstCh;
    if ((srcCh > 15) || (dstCh > 15))
    {
        return -2;
    }
    else
    {
        srcdstCh = (srcCh << 4) | dstCh;
    }

    for (byte i = 0; i < _numSubs; i++)
    {
        if ((dstIPnib == _subscriptions[i].dstIPnib) && (srcdstCh == _subscriptions[i].srcdstChannel))
        {
            _subscriptions[i].dstIPnib = 0x00;
            _subscriptions[i].srcdstChannel = 0x00;
            _numSubs--;
        }
    }
    //not sure if I am 100% able to prevent multiple same subscriptions... so just for sure not putting return 1; in the if scope
    return 1;
}

void Controller::printSubs()
{
    for (byte i = 0; i < _numSubs; i++)
    {
        Serial.print((_subscriptions[i].srcdstChannel & 0xF0) >> 4, DEC);
        Serial.print("\t -> \t");
        Serial.print("Local IP .");
        Serial.print(_subscriptions[i].dstIPnib, DEC);
        Serial.print(": channel = ");
        Serial.println((_subscriptions[i].srcdstChannel & 0x0F), DEC);
    }
}
