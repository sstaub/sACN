#include "Ethernet.h"
#include "sACN.h"

uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x14, 0x48}; // MAC Adress of your device
IPAddress ip(10, 101, 1, 201); // IP address of your device
IPAddress dns(10, 101, 1, 100); // DNS address of your device
IPAddress gateway(10, 101, 1, 100); // Gateway address of your device
IPAddress subnet(255, 255, 0, 0); // Subnet mask of your device

EthernetUDP sacn;

// CID fd32aedc-7b94-11e7-bb31-be2e44b06b34
uint8_t cid[16] {0xFD, 0x32, 0xAE, 0xDC, 0x7B, 0x94, 0x11, 0xE7, 0xBB, 0x31, 0xBE, 0x2E, 0x44, 0xB0, 0x6B, 0x34};
Source sender(sacn);

void setup() {
	Serial.begin(9600);
	delay(2000);
	Ethernet.begin(mac, ip, dns, gateway, subnet);
	deviceCID(cid);
  deviceName("Arduino");
	sender.begin(1);
	Serial.println("sACN start");
	sender.dmx(1, 255); // send value 255 to DMX slot 1 
	}

void loop() {
	sender.idle();
	}
