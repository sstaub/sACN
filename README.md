# Arduino library for sending and receiving sACN DMX streams following ANSI E1.31-2018

This library send and receives sACN DMX streams following the ANSI E1.31-2018 standard.

## !!! Attention !!! Version 1.1 changes
- Source: DD packet are initialized with priority of the source not the default sACN priority (100)
- Source: The CID and the source name are globaly defined by new helper functions ```deviceCID()``` and ```deviceName()```
- Rceiver: method name ```receive()``` changed to ```update()```
- Receiver / Source: Class initialisation is changed
- Receiver / Source: ```begin()``` now contains the init data
- new IDToolsPico for RaspberryPiPico
- API names for the ID tools are changed


## Limitations
### Merging received data
Merging data from more than one source is not implemented. The goal of the E1.31 standard is to replace it with the priority concept.<br>
Maybe *priority per channel* (ETC start code DD) merge will implemented in a later version.<br>
In the moment, if there is more than one source with the same priority, the first source win the race all other sources are ignored.

### Protocol limitations
Following parts of the ANSI E1.31 protocol are not supported yet:
- Universe Discovery
- Synchronization Packets
- Preview Data
- No Shutdown process when no more source data received 

There is also no support for RDM ANSI E1.20 and RDMNET ANSI E1.33

### Ethernet library problems
While writing this library many problems occurs with different ethernet libraries.
- Teensy 4.1 with FNET does not work because of blocking the sockets
- Teensy 4.1 with QNEthernet needs to test.
- STM32duino needs to test.
- Arduino Ethernet library have unexpected problems with sending unicast sACN.
- WizNet W5500 with Ethernet3 library,
  unexpected problems occurs with the W5500-EVB-Pico board, which is the best solution because of price (about € 10.-) and the power of the Raspberry Pi Pico. Sending works but not receiving. The Arduino Ethernet library is recommended for use with Pico and W5500
- WizNet W5500 a hard reset should performed to avoid connection problems with switches, if this can't done by board hardware. This must done before `Ethernet.begin()`
    - for Ethernet3 library use `Ethernet.hardreset()`
    - Arduino Ethernet library should do this with following code 

```cpp
void hardreset(uint8_t pinRST) {
  pinMode(pinRST, OUTPUT);
  digitalWrite(pinRST, HIGH);
  digitalWrite(pinRST, LOW);
  delay(1);
  digitalWrite(pinRST, HIGH);
  delay(150);
  }
```
Feedback from users will helpful.

## Example
Here is a simple example for receiving. You find also a receive and sending examples in the dedicated folder. The examples are tested with an Arduino MEGA with Ethernet Shield 2. For work with other boards and libraries you need to modify them.
```cpp
#include "Ethernet.h"
#include "sACN.h"

uint8_t mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x14, 0x48}; // MAC Adress of your device
IPAddress ip(10, 101, 1, 201); // IP 
IPAddress dns(10, 101, 1, 100); // IP 
IPAddress gateway(10, 101, 1, 100); // IP 
IPAddress subnet(255, 255, 0, 0); // 

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
  Serial.print("Universe 1 DMX framerate ");
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
  recv.callbackTimeout(timeOut);
  recv.callbackFramerate(framerate);
  recv.begin(1);
  Serial.println("sACN start");
  }

void loop() {
  recv.update();
  }
```

# Documentation

## ID Tools
In `IDTools.h` you find some useful tools to generate an individual serial number (UUID/CID) and MAC address, also for formatted printing. The MAC address is only valid in local networks. All tools needs a random start number to generate, this can e.g. noise values from Analog inputs, RTC ...<br>
The functions must call inside ```setup()```

### UUID/CID serial number Tools
```cpp
void generateUUID(uint8_t uuid[], unsigned int srnd)
uint8_t* generateUUID(unsigned int srnd)
```

### MAC address Tools
```cpp
void generateMAC(uint8_t mac[], unsigned int srnd)
uint8_t* generateMAC(unsigned int srnd)
```

## ID Tools for RaspberryPiPico
In `IDToolsPico.h` you find some useful tools to generate an individual serial number (UUID/CID) and MAC address, also for formatted printing. The MAC address is only valid in local networks. There is no random init necessary.
The functions must call inside ```setup()```

### UUID/CID serial number Tools
```cpp
void generateUUID(uint8_t uuid[])
uint8_t* generateUUID()
```

### MAC address Tools
```cpp
void generateMAC(uint8_t mac[])
uint8_t* generateMAC()
```

## Receiver API

### Constructor
```cpp
Receiver(UDP& udp)
```
- **udp** UDP socket instance


Create a Receiver object.

**Example**
```cpp
EthernetUDP sacn1;
Receiver recv1(sacn1); // Universe 1, no Unicast
```

## Methods

### **begin()**
```cpp
void begin(uint16_t universe, bool unicastMode = false)
```
- **universe** the sACN universe you want to receive
- **uniCastMode** `true` if you want receive unicast streams

Start the UDP connection of the receiver, this should happen in `setup()`.

**Example**
```cpp
recv1.begin(1);
```

### **stop()**
```cpp
void stop()
```

Stop UDP connection of the receiver.

**Example**
```cpp
recv1.stop();
```

### **update()**
```cpp
bool update()
```

Proceed the sACN data of the UDP connection, return true if there is a valid sACN packet received. This must done inside `loop()`.

**Example**
```cpp
recv1.update();
```

### **dmx()**
```cpp
uint8_t* dmx()
void dmx(uint8_t *data)
uint8_t dmx(uint16_t slot);
```

- ***data** pointer to the whole dmx universe
- **slot** slot number (DMX address)

Get the DMX data, you can get the whole universe or a single DMX slot.

**Example**
```cpp
uint8_t buffer[512];
recv1.dmx(buffer); // copy the whole universe

uint8_t dmx1; 
dmx1 = recv1.dmx(1); // get data from slot 1
```

### **name()**
```cpp
char* name()
void name(char *sourceName)
```
- **sourceNAme** pointer to a buffer for the source name

Get the name of the active source.

**Example**
```cpp
Serial.print("Source name: ");
Serial.println(recv.name());
```

### **framerate()**
```cpp
uint8_t framerate()
```

Get the frame rate in fps.

**Example**
```cpp
uint8_t framerate = recv1. framerate();
```

### **sources()**
```cpp
bool sources()
```

Get the state of active sources, Return `true` if a source is active.

**Example**
```cpp
bool state = recv1.sources();
```

### **callbackDMX()**
```cpp
void callbackDMX(fptr callDMX)
```
- **callDMX** function name which should executed

Set a callback when receiving only changed DMX data. This should configured in `setup()`.

**Example**
```cpp
// before setup()
void dmxReceived() {
  Serial.println("New DMX received ");
  }

// in setup()
recv1.callbackDMX(dmxReceived);
```

### **callbackSource()**

```cpp
void callbackSource(fptr callSource)
```
- **callSource** function name which should executed

Set a callback when there is a new active Source. This should configured in `setup()`.

### **callbackFramerate()**

```cpp
void callbackFramerate(fptr callFramerate)
```
- **callFramerate** function name which should executed

Set a callback every second the current framerate. This should configured in `setup()`.

**Example**
```cpp
// before setup()
void framerate() {
  Serial.print("Universe framerate: ");
  Serial.println(recv1.framerate());
  }

// in setup()
recv1.callbackFramerate(framerate);
```

### **callbackTimeout()**
```cpp
void callbackTimeout(fptr callTimeout)
```
- **callTimeout** function name which should executed

Set a callback when a timeout occurs. This happens after 2500 ms without receiving any valid sACN packet. This should configured in `setup()`.

**Example**
```cpp
// before setup()
void timeOut() {
  Serial.println("Timeout!");
  }

// in setup()
recv1.timeOut(timeOut);
```

## Source API

### Source helper functions

### deviceCID
```cpp
void deviceCID(uint8_t cid[16])
```
- **cid** common universal UUID/CID of the device

Set the device CID common for all sources. This must done in `setup()`) before `begin()`.
You can generate the CID with the IDTools.

**Example**
```cpp
// before setup
uint8_t id[16] {0xFD, 0x32, 0xAE, 0xDC, 0x7B, 0x94, 0x11, 0xE7, 0xBB, 0x31, 0xBE, 0x2E, 0x44, 0xB0, 0x6B, 0x34};

// in setup()
deviceCID(id);
```

### deviceName
```cpp
void deviceName(const char name[64])
```
- **name** common name of the device

Set the device name common for all sources. This must done in `setup()`) before `begin()`.

**Example**
```cpp
// in setup()
deviceName("Arduino");
```

### Constructor
```cpp
Source(UDP& udp)
```
- **udp** UDP socket instance

Create a Source object.

**Example**
```cpp
EthernetUDP sacn1;
Source send1(sacn1); // universe 1 with priority 100
```

## Methods

### **begin()**
```cpp
void begin(uint16_t universe, uint16_t priority, bool priorityDD = false)
void begin(IPAddress unicastIp ,uint16_t universe, uint16_t priority, bool priorityDD = false)
```
- **universe** the sACN universe you want to send
- **priority** sACN priority 0...200
- **priorityDD** allows to send also *priority per channel* packets (ETC startcode DD)
- **unicastIp** optional IP address for sending packets as unicast instead of multicast

Start the UDP connection of the source, this should happen in `setup()`.

**Example**
```cpp
send1.begin(1); // universe 1, default priority
send2.begin(4, 101, true); // universe 1, priority 101, with DD
```

### **stop()**
```cpp
void stop()
```

Stop UDP connection of the source.

**Example**
```cpp
send1.stop();
```

### **dmx()**
```cpp
void dmx(uint8_t *data)
void dmx(uint16_t slot, uint8_t data)
```
- ***data** pointer to the whole dmx universe, or DMX value for a single slot
- **slot** slot number (DMX address)

Set DMX values.

**Example**
```cpp
dmxBuffer[512];
dmxBuffer[0] = 128;
dmxBuffer[511] = 255;
send1.dmx(dmxBuffer);
```

### **dd()**
```cpp
void dd(uint8_t *priorityData)
void dd(uint16_t slot, uint8_t priorityData)
```
- ***priorityData** pointer to the whole priority universe, or priority value for a single slot
- **slot** slot number (DMX address)

Set the *priority per channel* values.

**Example**
```cpp
send1.dd(2, 0); // set the priority for DMX slot 2 to zero
```

### **send()**
```cpp
void send()
```

Send a sACN packet.

**Example**
```cpp
send1.send();
```

### **idle()**
```cpp
void idle()
```

Send sACN packets regularly (each 800ms), this must done in `loop()`. Also avoid long delays inside `loop()` or use RTOS instead.

**Example**
```cpp
loop() {
  send1.idle();
  }
```

### **sendDD()**
```cpp
void sendDD()
```

Send a sACN DD *priority per channel* packet.

**Example**
```cpp
send1.sendDD();
```

### **idleDD()**
```cpp
void idleDD()
```

Send sACN DD *priority per channel* packets regularly (each 800ms), this must done in `loop()`. Also avoid long delays inside `loop()` or use RTOS instead.

**Example**
```cpp
loop() {
  send1.idleDD();
  }
```
