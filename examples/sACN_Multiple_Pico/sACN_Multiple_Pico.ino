#include <Arduino.h>
#include <Ethernet.h>
#include "sACN.h"
#include "IDToolsPico.h"
#include "Ticker.h"

#define ETH_RST     20
#define ETH_CS      17

//uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x14, 0x48}; // MAC Adress of your device
uint8_t mac[6]; // for use with generator
IPAddress ip(10, 101, 1, 201); // IP Adress of your device
IPAddress dns(10, 101, 1, 100); // IP Adress of your device
IPAddress gateway(10, 101, 1, 100); // IP Adress of your device
IPAddress subnet(255, 255, 0, 0); // Subnet of your device

IPAddress ipUnicast(10, 101, 1, 100); // IP Adress of your device


// UDP instances
EthernetUDP sacn1;
EthernetUDP sacn2;
EthernetUDP sacn3;
EthernetUDP sacn4;

// CID fd32aedc-7b94-11e7-bb31-be2e44b06b34
// uint8_t id[16] {0xFD, 0x32, 0xAE, 0xDC, 0x7B, 0x94, 0x11, 0xE7, 0xBB, 0x31, 0xBE, 0x2E, 0x44, 0xB0, 0x6B, 0x34};
uint8_t cid[16]; // for use with ID generator

Receiver recv1(sacn1);
Receiver recv2(sacn2);
Source sender3(sacn3);
Source sender4(sacn4);

uint8_t recvBuffer1[512];
uint8_t recvBuffer2[512];
uint8_t dmxBuffer3[512];
uint8_t dmxBuffer4[512];

uint8_t i, j;
void maintain() {
  sender3.dmx(1, i++);
  sender3.dmx(512, j--);
  sender4.dmx(1, i++);
  sender4.dmx(512, j--);
  Serial.print("timestamp ");
  Serial.print(millis());
  Serial.println(" sACN send data");
  }
Ticker tick(maintain, 1000);

void framerate1() {
  Serial.print("Universe 1 DMX framerate ");
  Serial.println(recv1.framerate());
  }

void framerate2() {
  Serial.print("Universe 2 DMX framerate ");
  Serial.println(recv2.framerate());
  }

void dmxReceived1() {
  Serial.println("Universe 1 new DMX received ");
  Serial.print("DMX 1: ");
  Serial.print(recv1.dmx(1));
  Serial.print(" DMX 2: ");
  Serial.println(recv1.dmx(512));
  }

void dmxReceived2() {
  Serial.println("Universe 2 new DMX received ");
  Serial.print("DMX 1: ");
  Serial.print(recv2.dmx(1));
  Serial.print(" DMX 2: ");
  Serial.println(recv2.dmx(512));
  }

void newSource1() {
  Serial.print("Universe 1 new soure name: ");
  Serial.println(recv1.name());
  }

void newSource2() {
  Serial.print("Universe 2 new soure name: ");
  Serial.println(recv2.name());
  }

void timeOut1() {
  Serial.println("Universe 1 Timeout!");
  }

void timeOut2() {
  Serial.println("Universe 2 Timeout!");
  }

void hardreset(uint8_t pinRST) {
    pinMode(pinRST, OUTPUT);
    digitalWrite(pinRST, HIGH);
    digitalWrite(pinRST, LOW);
    delay(1);
    digitalWrite(pinRST, HIGH);
    delay(150);
  }

void setup() {
  Serial.begin(9600);
  delay(2000);
  generateUUID(cid);
  deviceCID(cid);
  deviceName("Arduino");
  generateMAC(mac);
  Ethernet.init(ETH_CS);
  hardreset(ETH_RST);
  Ethernet.begin(mac, ip, dns, gateway, subnet);
  recv1.callbackDMX(dmxReceived1);
  recv1.callbackSource(newSource1);
  recv1.callbackTimeout(timeOut1);
  recv1.callbackFramerate(framerate1);
  recv1.begin(1);
  recv2.callbackDMX(dmxReceived2);
  recv2.callbackSource(newSource2);
  recv2.callbackTimeout(timeOut2);
  recv2.callbackFramerate(framerate2);
  recv2.begin(2);
  sender3.begin(3, 100, true);
  sender3.dd(2, 0);
  sender4.begin(4, 100, true);
  sender4.dd(2, 0);
  tick.start();
  Serial.println("sACN start");
  dmxBuffer3[0] = 128;
  dmxBuffer3[511] = 255;
  dmxBuffer4[0] = 64;
  dmxBuffer4[511] = 128;
  sender3.dmx(dmxBuffer3);
  sender4.dmx(dmxBuffer4);
  }

void loop() {
  tick.update();
  recv1.update();
  recv2.update();
  sender3.idle();
  sender3.idleDD();
  sender4.idle();
  sender4.idleDD();
  }

