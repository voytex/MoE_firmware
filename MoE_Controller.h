#ifndef MoE_Controller_h
#define MoE_Controller_h

#include <Arduino.h>
#include <EthernetUdp.h>

class Controller {
    private:
        const byte _beacon[4];
        const byte _myMac[6]; 
        IPAddress _myIP;
        IPAddress _broadcastIP;
        const unsigned int _moePort = 50000;
        EthernetUDP eUDP;
    public:
        Controller();
        Controller(IPAddress);
        void beginUDP();
        void flashBeacon();
        void maintain();
        void handleLocalMIDI();
        void handleUDPMIDI();
};


















#endif