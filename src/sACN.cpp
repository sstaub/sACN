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

#include "sACN.h"
#include "sACNDefs.h"

Receiver::Receiver(UDP& udp, uint16_t universe, bool unicastMode) {
	this->udp = &udp;
	this->universe = universe;
	this->unicastMode = unicastMode;
	mcastIP[2] = universe >> 8;
	mcastIP[3] = universe;
	sacnPacket = new uint8_t [SACN_BUFFER_MAX];
	}

Receiver::~Receiver() {
	free(sacnPacket);
	}

void Receiver::begin() {
	if(unicastMode) udp->begin(ACN_SDT_MULTICAST_PORT);
	else udp->beginMulticast(mcastIP, ACN_SDT_MULTICAST_PORT);
	receiverTimeout = millis();
	}

void Receiver::stop() {
	udp->stop();
	}

bool Receiver::receive() {
	packetSize = udp->parsePacket();
	if(source.active && ((receiverTimeout + E131_NETWORK_DATA_LOSS_TIMEOUT) < millis())) {
		source = {};
		if (callTimeoutFunction != NULL) callTimeoutFunction();
		}
	if(packetSize > 0 && packetSize <= SACN_BUFFER_MAX) {
		udp->read(sacnPacket, SACN_BUFFER_MAX);
		if(parse()) {
			receiverTimeout = millis();
			return true;
			}
		}
	return false;
	}

bool Receiver::parse() {
	// verify root layer
	if (sacnPacket[PREAMBLE_ADDR] != PREAMBLE[0]) return false;
	if (sacnPacket[PREAMBLE_ADDR + 1] != PREAMBLE[1]) return false;
	if (sacnPacket[POSTAMBLE_ADDR] != POSTAMBLE[0]) return false;
	if (sacnPacket[POSTAMBLE_ADDR + 1] != POSTAMBLE[1]) return false;
	for (uint8_t i = 0; i < ACN_IDENTIFIER_SIZE ; i++) {
		if (sacnPacket[i + ACN_IDENTIFIER_ADDR] != ACN_IDENTIFIER[i]) return false;
		}
	rootFlagAndLength = flagAndLength(sacnPacket[ROOT_FLAGS_AND_LENGTH_ADDR], sacnPacket[ROOT_FLAGS_AND_LENGTH_ADDR +1], ROOT_FLAGS_AND_LENGTH_ADDR);
	if (packetSize != rootFlagAndLength) return false;
	for (uint8_t i = 0; i < VECTOR_ROOT_E131_DATA_SIZE ; i++) {
		if (sacnPacket[i + VECTOR_ROOT_E131_DATA_ADDR] != VECTOR_ROOT_E131_DATA[i]) return false;
		}

	// verify framing layer
	framingFlagAndLength = flagAndLength(sacnPacket[FRAMING_FLAGS_AND_LENGTH_ADDR], sacnPacket[FRAMING_FLAGS_AND_LENGTH_ADDR +1], FRAMING_FLAGS_AND_LENGTH_ADDR);
	if (packetSize != framingFlagAndLength) return false;
	for (uint8_t i = 0; i < VECTOR_E131_DATA_PACKET_SIZE ; i++) {
		if (sacnPacket[i + VECTOR_E131_DATA_PACKET_ADDR] != VECTOR_E131_DATA_PACKET[i]) return false;
		}
	priority = sacnPacket[PRIORITY_ADDR];
	if (priority > PRIORITY_MAX) return false;
	seqNumber = sacnPacket[SEQ_NUM_ADDR];
	if (sacnPacket[OPTIONS_ADDR] != 0) {
		// TODO clear source if bit 6 true for 3 packets (stream terminated), then make a timeout callback
		return false;
		}
	if (universe != ((sacnPacket[UNIVERSE_ADDR] << 8) + sacnPacket[UNIVERSE_ADDR + 1])) return false;

	// verify data layer
	dmpFlagAndLength = flagAndLength(sacnPacket[DMP_FLAGS_AND_LENGTH_ADDR], sacnPacket[DMP_FLAGS_AND_LENGTH_ADDR +1], DMP_FLAGS_AND_LENGTH_ADDR);
	if (packetSize != dmpFlagAndLength) return false;
	if (sacnPacket[VECTOR_DMP_SET_PROPERTY_ADDR] != VECTOR_DMP_SET_PROPERTY) return false;
	if (sacnPacket[DMP_ADDRESS_AND_DATA_ADDR] != DMP_ADDRESS_AND_DATA) return false;
	if (sacnPacket[FIRST_PROPERTY_ADDRESS_ADDR] != FIRST_PROPERTY_ADDRESS[0]) return false;
	if (sacnPacket[FIRST_PROPERTY_ADDRESS_ADDR + 1] != FIRST_PROPERTY_ADDRESS[1]) return false;
	if (sacnPacket[ADDRESS_INC_ADDR] != ADDRESS_INC[0]) return false;
	if (sacnPacket[ADDRESS_INC_ADDR + 1] != ADDRESS_INC[1]) return false;
	propertyValueCount = (sacnPacket[PROPERTY_VALUE_COUNT_ADDR] << 8) + sacnPacket[PROPERTY_VALUE_COUNT_ADDR + 1];
	if ((packetSize - STARTCODE_ADDR) != propertyValueCount) return false;
	if (sacnPacket[STARTCODE_ADDR] != STARTCODE_DMX) return false;

	// copy message data to cid
	memcpy(cid, sacnPacket + CID_ADDR, CID_SIZE);
	//init source, init source with higher priority, init new source after timeout
	uint32_t timeout = millis() - source.timestamp;
	if ((source.active == false) || (priority > source.priority) || (timeout > E131_NETWORK_DATA_LOSS_TIMEOUT)) {
		memcpy(source.cid, sacnPacket + CID_ADDR, CID_SIZE);
		memcpy(source.name, sacnPacket + SOURCE_NAME_ADDR, SOURCE_NAME_SIZE - 1);
		source.priority = priority;
		source.active = true;
		source.newSource = true;
		if (callSourceFunction != NULL) callSourceFunction();
		source.frameRateTimestamp = millis();
		source.frameRateCount = 1;
		}
	// verify source
	if(memcmp(source.cid, cid, CID_SIZE) != 0) return false;
	// verify sequenznumber
	if (((seqNumber - source.seqNumber) <= 0) && ((seqNumber - source.seqNumber) > -20)) return false;
	// update source data
	source.timestamp = millis();
	source.seqNumber = seqNumber;
	// calculate framerate
	if((source.frameRateTimestamp + 1000) > millis()) {
		source.frameRateCount++;
		}
	else {
		source.frameRate = source.frameRateCount;
		source.frameRateCount = 0;
		source.frameRateTimestamp = millis();
		if (callFramerateFunction != NULL) callFramerateFunction();
		}
	// copy data to dmx buffer 
	uint16_t dmxLength = packetSize - DMX_VALUES_ADDR;
	if(memcmp(source.dmx, sacnPacket + DMX_VALUES_ADDR, dmxLength) != 0) {
		memcpy(source.dmx, sacnPacket + DMX_VALUES_ADDR, dmxLength);
		if (callDMXFunction != NULL) callDMXFunction();
		}
	return true;
	}

void Receiver::callbackDMX(fptr callDMX) {
	callDMXFunction = callDMX;
	}

void Receiver::callbackSource(fptr callSource) {
	callSourceFunction = callSource;
	}

void Receiver::callbackTimeout(fptr callTimeout) {
	callTimeoutFunction = callTimeout;
	}

void Receiver::callbackFramerate(fptr callFramerate) {
	callFramerateFunction = callFramerate;
	}

uint8_t* Receiver::dmx() {
	return source.dmx;
	}

void Receiver::dmx(uint8_t *data) {
	memcpy(data, source.dmx, DMX_SLOTS_MAX);
	}

uint8_t Receiver::dmx(uint16_t slot) {
	if(slot > 0 && slot <= DMX_SLOTS_MAX)
		return source.dmx[slot - 1];
	return 0;
	}

char* Receiver::name() {
	return source.name;
	}

void Receiver::name(char *sourceName) {
	memcpy(sourceName, source.name, SOURCE_NAME_SIZE);
	}

uint8_t Receiver::framerate() {
	return source.frameRate;;
	}

bool Receiver::sources() {
	return source.active;
	}

uint16_t Receiver::flagAndLength(uint8_t highByte, uint8_t lowByte, uint16_t startAddress) {
	return (highByte << 8) + lowByte - 0x7000 + startAddress;
	}


Source::Source(UDP& udp, uint16_t universe, uint16_t priority, uint8_t cid[16], const char *name, bool priorityDD) {
	this->udp = &udp;
	this->universe = universe;
	this->priority = priority;
	this->priorityDD = priorityDD;
	memcpy(this->cid, cid, CID_SIZE);
	memcpy(this->name, name, SOURCE_NAME_SIZE);
	mcastIP[2] = universe >> 8;
	mcastIP[3] = universe;
	sacnPacket = new uint8_t [SACN_BUFFER_MAX];
	initPacket(sacnPacket);
	if(priorityDD == true) {
		sacnPacketDD = new uint8_t [SACN_BUFFER_MAX];
		initPacket(sacnPacketDD);
		sacnPacketDD[STARTCODE_ADDR] = 0xDD;
		memset(sacnPacketDD + DMX_VALUES_ADDR, PRIORITY_STANDARD, DMX_SLOTS_MAX );
		}
	}

Source::Source(UDP& udp, IPAddress ip, uint16_t universe, uint16_t priority, uint8_t cid[16], const char *name, bool priorityDD) {
	unicastMode = true;
	this->udp = &udp;
	this->universe = universe;
	this->priority = priority;
	this->ip = ip;
	this->priorityDD = priorityDD;
	memcpy(this->cid, cid, CID_SIZE);
	memcpy(this->name, name, SOURCE_NAME_SIZE - 1);
	sacnPacket = new uint8_t [SACN_BUFFER_MAX];
	initPacket(sacnPacket);
	if(priorityDD == true) {
		sacnPacketDD = new uint8_t [SACN_BUFFER_MAX];
		initPacket(sacnPacketDD);
		sacnPacketDD[STARTCODE_ADDR] = 0xDD;
		memset(sacnPacketDD + DMX_VALUES_ADDR, PRIORITY_STANDARD, DMX_SLOTS_MAX );
		}
	}

Source::~Source() {
	free(sacnPacket);
	if(priorityDD == true) 
		free(sacnPacketDD);
	}

void Source::begin() {
	if(unicastMode) udp->begin(ACN_SDT_MULTICAST_PORT);
	else udp->beginMulticast(mcastIP, ACN_SDT_MULTICAST_PORT);
	for(uint8_t i = 0; i < 3; i++) {
		send();
		if(priorityDD) sendDD();
		delay(40);
		}
	timestamp = millis();
	timestampDD = millis();
	}

void Source::stop() {
	sacnPacket[OPTIONS_ADDR] = STREAM_TERMINATED;
	if(priorityDD) sacnPacketDD[OPTIONS_ADDR] = STREAM_TERMINATED;
	for(uint8_t i = 0; i < 3; i++) {
		send();
		if(priorityDD) sendDD();
		delay(40);
		}
	udp->stop();
	}

void Source::CID(uint8_t cid[16]) {
	memcpy(sacnPacket + CID_ADDR, cid, CID_SIZE);
	if(priorityDD) memcpy(sacnPacketDD + CID_ADDR, cid, CID_SIZE);
	}

void Source::dmx(uint8_t *data) {
	memcpy(sacnPacket + DMX_VALUES_ADDR, data, DMX_SLOTS_MAX);
	}

void Source::dmx(uint16_t slot, uint8_t data) {
	if(slot > 0 && slot <= DMX_SLOTS_MAX) {
		sacnPacket[DMX_VALUES_ADDR + slot - 1] = data;
		}
	}

void Source::dd(uint8_t *priorityData) {
	if(priorityDD) {
		memcpy(sacnPacketDD + DMX_VALUES_ADDR, priorityData, DMX_SLOTS_MAX);
		}
	}

void Source::dd(uint16_t slot, uint8_t priorityData) {
	if(priorityDD) {
		if(slot > 0 && slot <= DMX_SLOTS_MAX) {
			sacnPacketDD[DMX_VALUES_ADDR + slot - 1] = priorityData;
			}
		}
	}

void Source::send() {
	if(unicastMode) udp->beginPacket(ip, ACN_SDT_MULTICAST_PORT);
	else udp->beginPacket(mcastIP, ACN_SDT_MULTICAST_PORT);
	udp->write(sacnPacket, SACN_BUFFER_MAX);
	udp->endPacket();
	timestamp = millis();
	sacnPacket[SEQ_NUM_ADDR]++;
	}

void Source::idle() {
	if(millis() > timestamp + SACN_POLLING_TIME) {
		send();
		timestamp = millis();
		}
	}

void Source::sendDD() {
	if(priorityDD) {
		sacnPacketDD[SEQ_NUM_ADDR] = sacnPacket[SEQ_NUM_ADDR];
		if(unicastMode) udp->beginPacket(ip, ACN_SDT_MULTICAST_PORT);
		else udp->beginPacket(mcastIP, ACN_SDT_MULTICAST_PORT);
		udp->write(sacnPacketDD, SACN_BUFFER_MAX);
		udp->endPacket();
		sacnPacket[SEQ_NUM_ADDR]++;
		timestampDD = millis();
		}
	}

void Source::idleDD() {
	if(priorityDD) {
		if(millis() > timestampDD + SACN_POLLING_TIME_DD) {
			sendDD();
			timestampDD = millis();
			}
		}
	}

void Source::initPacket(uint8_t *packet) {
	memset(packet, 0x00, SACN_BUFFER_MAX);
	// root layer
	packet[PREAMBLE_ADDR] = PREAMBLE[0];
	packet[PREAMBLE_ADDR + 1] = PREAMBLE[1];
	packet[POSTAMBLE_ADDR] = POSTAMBLE[0];
	packet[POSTAMBLE_ADDR + 1] = POSTAMBLE[1];
	memcpy(packet + ACN_IDENTIFIER_ADDR, ACN_IDENTIFIER, ACN_IDENTIFIER_SIZE);
	memcpy(packet + ROOT_FLAGS_AND_LENGTH_ADDR, ROOT_FLAGS_AND_LENGTH, ROOT_FLAGS_AND_LENGTH_SIZE);
	memcpy(packet + VECTOR_ROOT_E131_DATA_ADDR, VECTOR_ROOT_E131_DATA, VECTOR_ROOT_E131_DATA_SIZE);
	memcpy(packet + CID_ADDR, cid, CID_SIZE);
	// framing layer
	memcpy(packet + FRAMING_FLAGS_AND_LENGTH_ADDR, FRAMING_FLAGS_AND_LENGTH, FRAMING_FLAGS_AND_LENGTH_SIZE);
	memcpy(packet + VECTOR_E131_DATA_PACKET_ADDR, VECTOR_E131_DATA_PACKET, VECTOR_E131_DATA_PACKET_SIZE);
	memcpy(packet + SOURCE_NAME_ADDR, name, SOURCE_NAME_SIZE);
	packet[PRIORITY_ADDR] = priority;
	packet[UNIVERSE_ADDR] = universe >> 8;
	packet[UNIVERSE_ADDR + 1] = universe;
	// dmp layer
	memcpy(packet + DMP_FLAGS_AND_LENGTH_ADDR, DMP_FLAGS_AND_LENGTH, DMP_FLAGS_AND_LENGTH_SIZE);
	packet[VECTOR_DMP_SET_PROPERTY_ADDR] = VECTOR_DMP_SET_PROPERTY;
	packet[DMP_ADDRESS_AND_DATA_ADDR] = DMP_ADDRESS_AND_DATA;
	memcpy(packet + ADDRESS_INC_ADDR, ADDRESS_INC, ADDRESS_INC_SIZE);
	memcpy(packet + PROPERTY_VALUE_COUNT_ADDR, PROPERTY_VALUE_COUNT, PROPERTY_VALUE_COUNT_SIZE);
	}

