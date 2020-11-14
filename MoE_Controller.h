#ifndef MoE_Controller_h
#define MoE_Controller_h

#include <Arduino.h>
#include <EEPROM.h>
#include <EthernetUdp.h>

/*
/ MAC addresses: Arduino UNO   byte mac[] = {0xB9, 0xDF, 0x48, 0x5B, 0x33, 0xAF};
/                Arduino UNO
 */

class Controller {
    private:
        byte * macRead();  
        
        //ETHERNET_SETTINGS_________________________________________
        
            //Mac address, reading from EEPROM
        const byte * _myMac = * macRead();
            
            //IP to use when DHCP fails, or when we want to use custom IP
        byte _safeIP[4] = {192, 168, 1, 100};

            //Broadcast beacon for auto-device-discover
        const byte _beacon[4] = {'A', 'H', 'O', 'J'};

          //MIDI over Ethernet protocol port
        const unsigned int _moePort = 50000;
        
        //__________________________________________________________
        
        byte _knownDevices[32];
        unsigned int _numKD = 0; 
        IPAddress _forceIP;
        IPAddress _broadcastIP;
        
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
