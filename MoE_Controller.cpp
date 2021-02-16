#include <Arduino.h>
#include <EEPROM.h>
#include <EthernetUdp.h>
#include <SoftwareSerial.h>
#include "MoE_Controller.h"


//#define DEBUG

#ifndef DEBUG
#define Serial NoOperation
static class {
public:
    void write(...) {}
    void begin(...) {}
    void print(...) {}
    void println(...) {}
} Serial;
#endif

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
    Serial.println("Trying to obtain IP from DHCP...");
    if (Ethernet.begin(_myMac))
    {
    }
    else
    {
        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
            Serial.println("Ethernet shield not found!");
            while (true)
            {
                //do nothing
                delay(1);
            }
        }
        if (Ethernet.linkStatus() == LinkOFF)
        {
            Serial.println("Ethernet cable not connected!");
        }
        Serial.println("Failed to obtain IP from DHCP, setting default values.");
        Ethernet.begin(_myMac, _safeIP);
    }
    Serial.print("Arduino address: ");
    Serial.println(Ethernet.localIP());

    _broadcastIP = Ethernet.localIP();
    _broadcastIP[3] = 255;

    //preparation
    _destinationIP = Ethernet.localIP();
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
    if (midiSerial.available())
        _incoming = midiSerial.read();
    
    //normal status:
    if (_incoming >= 0b10000000) {
        _data0 = _incoming;
        _recSB = true;
    } else if ((_recSB) && (!_recDB)) {
        _data1 = _incoming;
        _recDB = true;
    } else if ((_recSB) && (_recDB)) {
        _data2 = _incoming;
        sendUDP3();
        _recSB = false;
        _recDB = false;
    }

    //running status:
    if ((_incoming < 0b10000000) && (!_recSB)) {
        if (!_recDB) {
            _data1 = _incoming;
            _recDB = true;
        } else {
            _data2 = _incoming;
            sendUDP2();
            _recDB = false;
        }
    }

    //if (midiSerial.available() == 3)
    //{
    //    _data0 = midiSerial.read();
    //    _data1 = midiSerial.read();
    //    _data2 = midiSerial.read();
    //    sendUDP(_data0, _data1, _data2);
    //
    //}
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

void Controller::sendUDP3()
{
    _srcCh = _data0 & 0x0F;
    for (byte i = 0; i < _numSubs; i++)
    {
        if (_srcCh == (_subscriptions[i].srcdstChannel & 0xF0) >> 4)
        {
            _data0 = _data0 & 0xF0;
            _data0 = _data0 | (_subscriptions[i].srcdstChannel & 0x0F);
            _destinationIP[3] = _subscriptions[i].dstIPnib;
            eUDP.beginPacket(_destinationIP, MOE_PORT);
            eUDP.write(0xA3);
            eUDP.write(_data0);
            eUDP.write(_data1);
            eUDP.write(_data2);
            eUDP.endPacket();
        }
    }
}

void Controller::sendUDP2()
{
    for (byte i = 0; i < _numSubs; i++)
    {
        if (_srcCh == (_subscriptions[i].srcdstChannel & 0xF0) >> 4)
        {
            _destinationIP[3] = _subscriptions[i].dstIPnib;
            eUDP.beginPacket(_destinationIP, MOE_PORT);
            eUDP.write(0xA2);
            eUDP.write(_data1);
            eUDP.write(_data2);
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

void Controller::sendSubs(IPAddress editorPC)
{
    if (_numSubs == 0) 
    {
        const byte placeholder[4] = {0x80, 0xFF, 0x00, 0xFF};
        eUDP.beginPacket(editorPC, MOE_PORT);
        eUDP.write(placeholder, 4);
        eUDP.endPacket();
    }
    for (byte i = 0; i < _numSubs; i++)
    {
        eUDP.beginPacket(editorPC, MOE_PORT);
        eUDP.write(0x80);
        eUDP.write((_subscriptions[i].srcdstChannel & 0xF0) >> 4);
        eUDP.write(_subscriptions[i].dstIPnib);
        eUDP.write(_subscriptions[i].srcdstChannel & 0x0F);
        eUDP.endPacket();
    }
    
}
