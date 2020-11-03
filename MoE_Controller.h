#ifndef MoE_Controller_h
#define MoE_Controller_h

#include <Arduino.h>
#include <EthernetUdp.h>

class Controller {
    private:
        const byte _beacon[4];
        const byte _myMac[6];
        byte _knownDevices[32];
        unsigned int _numKD = 0; 
        IPAddress _myIP;
        IPAddress _broadcastIP;
        const unsigned int _moePort = 50000;
        EthernetUDP eUDP;

        byte _incomingUDP[4];
    public:
        Controller();
        Controller(IPAddress);
        void initialize();
        void beginUDP();
        void flashBeacon();
        void handleBeacon(IPAddress);
        void maintain();
        void handleLocalMIDI();
        void handleUDPMIDI();

        void handleUDP();
        void handleSerial();
};


















#endif