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

Controller::Controller() : midiSerial(4, 5)
{
}

Controller::Controller(IPAddress forceIP) : midiSerial(4, 5)
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
    Serial.print("Arduino address: ");
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
    Serial.println("[beacon]");
}

void Controller::handleUDP()
{
    if (eUDP.parsePacket())
    {
        eUDP.readByte(_incomingUDP, 4);
        switch (_incomingUDP[0])
        {
        case 0x08:
            Serial.print("[0x08] - sending subs >> ");
            Serial.println(eUDP.remoteIP());
            sendSubs(eUDP.remoteIP());
            break;

        case 0x0E:
            Serial.println("[0x0E] - erasing subscription.");
            delSubscription(_incomingUDP[1], _incomingUDP[2], _incomingUDP[3]);
            break;

        case 0x0F:
            Serial.println("[0x0F] - adding subscription.");
            addSubscription(_incomingUDP[1], _incomingUDP[2], _incomingUDP[3]);
            break;

        case 0xA2:
            Serial.println("[0xA2] - writing 2MIDI");
            midiSerial.write(_incomingUDP[1]);
            midiSerial.write(_incomingUDP[2]);
            break;
        
        case 0xA3:
            Serial.println("[0xA3] - writing 3MIDI");
            midiSerial.write(_incomingUDP[1]);
            midiSerial.write(_incomingUDP[2]);
            midiSerial.write(_incomingUDP[3]);
            break;
        
        case 0xFF:
            Serial.println("[0xFF] - adding subscription");
            addSubscription(0, eUDP.remoteIP()[3], 0);
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
    if (midiSerial.available() == 3)
    {
        _data0 = midiSerial.read();
        _data1 = midiSerial.read();
        _data2 = midiSerial.read();
        //midiSerial.write(_data0);
        //midiSerial.write(_data1);
        //midiSerial.write(_data2);
        sendUDP(_data0, _data1, _data2);

    }


   // do
   // {
   //     if (midiSerial.available())
   //     {
   //         Serial.print("available");
   //         _data0 = midiSerial.read();
   //         _data1 = midiSerial.read();
   //         _data2 = midiSerial.read();
   //     }
   // } while (midiSerial.available() > 2);
   // sendUDP(_data0, _data1, true, _data2);
   // Serial.print("Sent");
    
    /*if (numIncBytes)
    {
        Serial.println(numIncBytes);
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
            sendUDP(data0, data1, true, data2);
            Serial.println("Sent MIDI!");
            break;
        }
        }
    }*/
}

void Controller::sendUDP(byte data0, byte data1, byte data2)
{
    byte srcCh = data0 & 0x0F;
    for (byte i = 0; i < _numSubs; i++)
    {
        Serial.println(i, DEC);
        if (srcCh == (_subscriptions[i].srcdstChannel & 0xF0) >> 4)
        {
            data0 = data0 & 0xF0;
            data0 = data0 | (_subscriptions[i].srcdstChannel & 0x0F);
            IPAddress remoteIP = Ethernet.localIP();
            remoteIP[3] = _subscriptions[i].dstIPnib;
            eUDP.beginPacket(remoteIP, MOE_PORT);
            eUDP.write(0xA3);
            eUDP.write(data0);
            eUDP.write(data1);
            eUDP.write(data2);
            eUDP.endPacket();
            Serial.println("just sent");
        }
    }
}

void Controller::sendUDP(byte data0, byte data1)
{
    Serial.println("twobyte");
    byte srcCh = data0 & 0x0F;
    for (byte i = 0; i < _numSubs; i++)
    {
        Serial.println(i, DEC);
        if (srcCh == (_subscriptions[i].srcdstChannel & 0xF0) >> 4)
        {
            data0 = data0 & 0xF0;
            data0 = data0 | (_subscriptions[i].srcdstChannel & 0x0F);
            IPAddress remoteIP = Ethernet.localIP();
            remoteIP[3] = _subscriptions[i].dstIPnib;
            eUDP.beginPacket(remoteIP, MOE_PORT);
            eUDP.write(0xA2);
            eUDP.write(data0);
            eUDP.write(data1);
            eUDP.endPacket();
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
            for (byte j = i; j < _numSubs; j++)
            {
                _subscriptions[j].dstIPnib = _subscriptions[j+1].dstIPnib;
                _subscriptions[j].srcdstChannel = _subscriptions[j+1].srcdstChannel;
            }
            break;
        }
    }
    _numSubs--;
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

void Controller::sendSubs(IPAddress remotePC)
{
    if (_numSubs == 0) 
    {
        const byte placeholder[4] = {0x80, 0xFF, 0x00, 0xFF};
        eUDP.beginPacket(remotePC, MOE_PORT);
        eUDP.write(placeholder, 4);
        eUDP.endPacket();
    }
    for (byte i = 0; i < _numSubs; i++)
    {
        eUDP.beginPacket(remotePC, MOE_PORT);
        eUDP.write(0x80);
        eUDP.write((_subscriptions[i].srcdstChannel & 0xF0) >> 4);
        eUDP.write(_subscriptions[i].dstIPnib);
        eUDP.write(_subscriptions[i].srcdstChannel & 0x0F);
        eUDP.endPacket();
    }
    
}
