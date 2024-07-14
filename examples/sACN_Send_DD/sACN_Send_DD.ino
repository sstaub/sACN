// Example for sending with priority by channel and use of the ID Tools

#include "Ethernet.h"
#include "sACN.h"
#include "IDTools.h"

//uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x14, 0x48}; // MAC Adress of your device
uint8_t mac[6]; // for use with generator
IPAddress ip(10, 101, 1, 201); // IP address of your device
IPAddress dns(10, 101, 1, 100); // DNS address of your device
IPAddress gateway(10, 101, 1, 100); // Gateway address of your device
IPAddress subnet(255, 255, 0, 0); // Subnet mask of your device

EthernetUDP sacn;

// CID fd32aedc-7b94-11e7-bb31-be2e44b06b34
//uint8_t id[16] {0xFD, 0x32, 0xAE, 0xDC, 0x7B, 0x94, 0x11, 0xE7, 0xBB, 0x31, 0xBE, 0x2E, 0x44, 0xB0, 0x6B, 0x34};
uint8_t cid[16]; // for use with ID generator
Source sender(sacn); // universe 1, priority 100, with DD priority

void setup() {
	generateUUID(cid, micros());
	deviceCID(cid);
  deviceName("Arduino");
	generateMAC(mac, micros());
	Serial.begin(9600);
	delay(2000);
	Ethernet.begin(mac, ip, dns, gateway, subnet);
	sender.begin(1, 100, true);
	Serial.println("sACN start");
	sender.dd(2, 0); // set priority of channel 2 to 0
	sender.dmx(1, 255); // send value 255 to DMX slot 1 
	}

void loop() {
	sender.idle();
	sender.idleDD();
	}
