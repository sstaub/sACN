/* Arduino library for sending and receiving sACN lighting protocoll ANSI E1.31
 *
 * (c) 2022 stefan staub
 * Released under the MIT License
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SACN_H
#define SACN_H

#include "Arduino.h"
#include "Udp.h"

/*
TODO for v1.1
- [x] CID / Name as global function
- [x] Sender DD priority should init with global priority of the sender
- [x] Class init only socket, the rest in begin()
- [ ] Layer check as functions
*/

void deviceCID(uint8_t cid[16]);
void deviceName(const char name[64]);

/**
 * @brief Receiver class for sACN ANSI E1.31
 * 
 */
class Receiver {
	typedef void (*fptr)();
	public:
	/**
	 * @brief Construct a new Receiver object
	 * 
	 * @param udp socket for receiving
	 * @param universe DMX universe to receive
	 * @param unicastMode allows to receive from unicast source
	 */
	Receiver(UDP& udp);

	/**
	 * @brief Destroy the Receiver object
	 * 
	 */
	~Receiver();

	/**
	 * @brief Begin the socket connection
	 * 
	 */
	void begin(uint16_t universe, bool unicastMode = false);

	/**
	 * @brief Stop the socket connection
	 * 
	 */
	void stop();

	/**
	 * @brief Receive and proceed incoming data, must inside of loop()
	 * 
	 * @return true if valid data received
	 * @return false if there is no valid data
	 */
	bool update();

	/**
	 * @brief Callback when receiving changed DMX data
	 * 
	 * @param callDMX function name to call
	 */
	void callbackDMX(fptr callDMX);

	/**
	 * @brief Callback for receiving a new source
	 * 
	 * @param callSource function name to call
	 */
	void callbackSource(fptr callSource);

	/**
	 * @brief Callback for data timeout
	 * 
	 * @param callTimeout function name to call
	 */
	void callbackTimeout(fptr callTimeout);

/**
	 * @brief Callback for updated framerate
	 * 
	 * @param callTimeout function name to call
	 */
	void callbackFramerate(fptr callFramerate);

	/**
	 * @brief Get DMX data
	 * 
	 * @return uint8_t* DMX universe content
	 */
	uint8_t* dmx();

	/**
	 * @brief Get DMX data
	 * 
	 * @param data DMX universe content
	 */
	void dmx(uint8_t *data);

	/**
	 * @brief Get a single DMX slot
	 * 
	 * @param slot DMX slot 1...512
	 * @return uint8_t DMX data 
	 */
	uint8_t dmx(uint16_t slot);

	/**
	 * @brief Get the source name 
	 * 
	 * @return char* source name
	 */
	char* name();

	/**
	 * @brief Get the source name
	 * 
	 * @param sourceName 
	 */
	void name(char *sourceName);

	/**
	 * @brief Get the framerate
	 * 
	 * @return uint8_t fps
	 */
	uint8_t framerate();

	/**
	 * @brief Get the state of the source
	 * 
	 * @return true if source is active
	 * @return false if no source available
	 */
	bool sources();

	private:
	bool parse();
	uint16_t flagAndLength(uint8_t highByte, uint8_t lowByte, uint16_t startAddress);
	UDP *udp;
	uint16_t universe;
	uint8_t mcastIP[4] = {239, 255, 0, 0}; // change to IPAddress
	bool unicastMode;
	uint8_t *sacnPacket;
	uint16_t packetSize;
	uint32_t receiverTimeout;
	fptr callDMXFunction;
	fptr callSourceFunction;
	fptr callTimeoutFunction;
	fptr callFramerateFunction;
	uint16_t rootFlagAndLength;
	uint16_t framingFlagAndLength;
	uint16_t dmpFlagAndLength;
	uint8_t cid[16];
	uint8_t seqNumber;
	uint8_t priority;
	uint16_t propertyValueCount;
	struct Sources {
		uint8_t cid[16];
		char name[64];
		uint8_t priority;
		int16_t seqNumber;
		uint8_t startcode;
		uint32_t timestamp;
		uint8_t dmx[512];
		uint8_t frameRate;
		uint8_t terminateCount;
		uint32_t frameRateTimestamp;
		uint8_t frameRateCount;
		bool active;
		bool newSource;
		};
	Sources source;
	};

class Source {
	public:
	/**
	 * @brief Construct a new Source object
	 * 
	 * @param udp socket for sending
	 */
	Source(UDP& udp);

	/**
	 * @brief Destroy the Source object
	 * 
	 */
	~Source();

	/**
	 * @brief Begin the socket connection
	 * 
	 * @param unicastIp optional IP address for unicast sending
	 * @param universe DMX universe to send
	 * @param priority sACN priority
	 * @param cid ID of the source, set to 0 if you need to do it later
	 * @param name source name
	 * @param priorityDD flag for sending optional priority per channel mode
	 */
	void begin(uint16_t universe, uint16_t priority = 100, bool priorityDD = false);
	void begin(IPAddress unicastIp, uint16_t universe, uint16_t priority = 100, bool priorityDD = false);

	/**
	 * @brief Stop the socket connection
	 * 
	 */
	void stop();

	/**
	 * @brief Set the DMX data
	 * 
	 * @param data DMX universe content
	 */
	void dmx(uint8_t *data);

	/**
	 * @brief Set DMX slot
	 * 
	 * @param slot DMX slot 1...512
	 * @param data DMX data
	 */
	void dmx(uint16_t slot, uint8_t data);

	/**
	 * @brief Set the DMX priority universe
	 * 
	 * @param priorityData DMX priority universe
	 */
	void dd(uint8_t *priorityData);

	/**
	 * @brief Set DMX priority slot
	 * 
	 * @param slot DMX priority slot 1...512
	 * @param priorityData DMX priority
	 */
	void dd(uint16_t slot, uint8_t priorityData);

	/**
	 * @brief Send the sACN packet
	 * 
	 */
	void send();

	/**
	 * @brief Idle mode for sACN packets, must inside of loop()
	 * 
	 */
	void idle();

	/**
	 * @brief Send the sACN packet with priority data
	 * 
	 */
	void sendDD();

	/**
	 * @brief Idle mode for priority data packets, must inside of loop()
	 * 
	 */
	void idleDD();

	private:
	void initPacket(uint8_t *packet);
	UDP *udp;
	uint8_t mcastIP[4] = {239, 255, 0, 0};
	IPAddress ip;
	IPAddress unicastIp;
	bool unicastMode;
	uint16_t universe;
	uint8_t priority;
	bool priorityDD;
	uint8_t *sacnPacket; // sacnPacket = new uint8_t [SACN_BUFFER_MAX];
	uint8_t *sacnPacketDD; // sacnDDPacket = new uint8_t [SACN_BUFFER_MAX];
	uint32_t timestamp;
	uint32_t timestampDD;
	};

#endif
