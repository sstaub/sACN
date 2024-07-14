#include "Ethernet.h"
#include "sACN.h"

uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x14, 0x48}; // MAC Adress of your device
IPAddress ip(10, 101, 1, 201); // IP address of your device
IPAddress dns(10, 101, 1, 100); // DNS address of your device
IPAddress gateway(10, 101, 1, 100); // Gateway address of your device
IPAddress subnet(255, 255, 0, 0); // Subnet mask of your device

EthernetUDP sacn;
Receiver recv(sacn); // universe 1

void dmxReceived() {
	Serial.println("New DMX data received ");
	Serial.print("DMX Slot 1: ");
	Serial.print(recv.dmx(1));
	Serial.print(" DMX Slot 2: ");
	Serial.println(recv.dmx(2));
	}

void newSource() {
	Serial.print("new soure name: ");
	Serial.println(recv.name());
	}

void framerate() {
  Serial.print("Framerate fps: ");
  Serial.println(recv.framerate());
  }

void timeOut() {
	Serial.println("Timeout!");
	}

void setup() {
	Serial.begin(9600);
	delay(2000);
	Ethernet.begin(mac, ip, dns, gateway, subnet);
	recv.callbackDMX(dmxReceived);
	recv.callbackSource(newSource);
	recv.callbackFramerate(framerate);
	recv.callbackTimeout(timeOut);
	recv.begin(1);
	Serial.println("sACN start");
	}

void loop() {
	recv.update();
	}
