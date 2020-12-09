#ifndef MoE_Controller_h
#define MoE_Controller_h

#define MAX_SUBS 64
#define MOE_PORT 50000

#include <Arduino.h>
#include <EEPROM.h>
#include <EthernetUdp.h>         //This library is slightly edited (added int readByte())
#include <SoftwareSerial.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
/ MAC addresses: Arduino UNO #1   0x4A, 0x0E, 0xA3, 0x0A, 0x93, 0x4F  /
/                Arduino UNO #2   0x88, 0x1D, 0xAC, 0x72, 0xCC, 0xA1  /
/                Arduino UNO #3   0x1C, 0x97, 0x63, 0x0A, 0xF2, 0x7E  /
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

class Controller
{
private:
  void macLoad(byte *mac);

  //ETHERNET_SETTINGS_________________________________________

  //Mac address, reading from EEPROM
  byte _myMac[6];

  //TODO: IP to use when DHCP fails, or when we want to use custom IP
  byte _safeIP[4] = {192, 168, 1, 100};

  //Broadcast beacon for auto-device-discover
  const byte _beacon[4] = {0xFF, 0xFF, 0xFF, 0xFF};

  IPAddress _broadcastIP;
 
  EthernetUDP eUDP;
  
  //__________________________________________________________

  
  byte _incomingUDP[4];

  //SUBSCRIPTION_Database____________________________________
  
  typedef struct subscription
  {
    //first nibble is source channel, second nibble is destination
    byte srcdstChannel;
    byte dstIPnib;
  };
  subscription _subscriptions[MAX_SUBS];
  
  byte _numSubs = 0;

  int addSubscription(byte, byte, byte);
  int delSubscription(byte, byte, byte);
  void sendSubs(IPAddress);
  void printSubs();
  
  //_________________________________________________________

  
  //UDP_sending______________________________________________
  void sendUDP(byte, byte, byte);
  void sendUDP(byte, byte);
  //_________________________________________________________
  
  byte _data0, _data1, _data2;          //will come handy when implementing running status

  SoftwareSerial midiSerial;            //MIDI interface

public:
  
  Controller();
  
  Controller(IPAddress);

  void initialize();

  void begin();

  void flashBeacon();

  void maintain();

  void handleMIDI();

  void handleUDP();
};

#endif
