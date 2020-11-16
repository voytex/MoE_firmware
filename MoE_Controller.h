#ifndef MoE_Controller_h
#define MoE_Controller_h

#define MAX_SUBS 64
#define MOE_PORT 50000

#include <Arduino.h>
#include <EEPROM.h>
#include <EthernetUdp.h>
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

  //IP to use when DHCP fails, or when we want to use custom IP
  byte _safeIP[4] = {192, 168, 1, 100};

  //Broadcast beacon for auto-device-discover
  const byte _beacon[4] = {'A', 'H', 'O', 'J'};

  //__________________________________________________________

  byte _knownDevices[32];
  unsigned int _numKD = 0;
  IPAddress _forceIP;
  IPAddress _broadcastIP;
  byte _incomingUDP[4];

  EthernetUDP eUDP;
  SoftwareSerial midiSerial;

  typedef struct subscription
  {
    //first nibble is source channel, second nibble is destination
    byte srcdstChannel;
    byte dstIPnib;
  };
  subscription _subscriptions[MAX_SUBS];
  byte _numSubs = 0;
  int addSubscription(byte srcCh, byte dstIPnib, byte dstCh);
  int delSubscription(byte srcCh, byte dstIPnib, byte dstCh);
  void printSubs();
  void sendUDP(byte data0, byte data1, bool threeByte, byte data2);

public:
  Controller();
  Controller(IPAddress);

  void initialize();

  void begin();

  void flashBeacon();
  void handleBeacon(IPAddress);

  void maintain();

  void handleMIDI();

  void handleUDP();

};

#endif
