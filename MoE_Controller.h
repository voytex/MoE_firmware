#ifndef MoE_Controller_h
#define MoE_Controller_h

#define MAX_SUBS 64

#include <Arduino.h>
#include <EEPROM.h>
#include <EthernetUdp.h>

/*
/ MAC addresses: Arduino UNO #1   0x4A, 0x0E, 0xA3, 0x0A, 0x93, 0x4F
/                Arduino UNO #2   0x88, 0x1D, 0xAC, 0x72, 0xCC, 0xA1
/                Arduino UNO #3   0x1C, 0x97, 0x63, 0x0A, 0xF2, 0x7E
 */

class Patchbay {
  private:
    const byte _maxSubs = 64;
    typedef struct subscription {
      byte sourceChannel;
      byte destIPnib;
      byte destChannel;
    };
    subscription subscriptions[MAX_SUBS];
    byte _numSubs = 0;
  public:
    Patchbay();
    int addSubscription(byte sourceChannel, byte destIPnib, byte destChannel);
    void deleteSubscription(byte sourceChannel, byte destIPnib, byte destChannel);
    void sendAll(byte sourceChannel); 
    void printSubs();
    
};

class Controller {
    private:
        void macRead(byte *mac);  
        
        //ETHERNET_SETTINGS_________________________________________
        
            //Mac address, reading from EEPROM
        byte _myMac[6];
            
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
        
       Patchbay patchbay;

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
