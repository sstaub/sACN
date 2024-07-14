/* Library for pseudo randomized UUID and MAC address for Raspberry Pi Pico
 *
 * (c) 2024 stefan staub
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

/*
 * The numbers are generated by the built-in pico random number generator.
 * The MAC address is only for local use.
 * At least you should use hardware chips with included serial and mac address like 
 * AT24MAC402/AT24MAC602 from microchip.
 * - UUID Tools following RFC 9562
 * - MAC Tools following RFC 9542
*/


#ifndef ID_TOOLS_PICO_H
#define ID_TOOLS_PICO_H

#include "Arduino.h"
#include <stdint.h>
#include "pico/stdlib.h"

/**
 * @brief genertate a random UUID
 * 
 * @param uuid array of 16 bytes
 */
void generateUUID(uint8_t uuid[]) {
	uint32_t rnd1 = get_rand_32();
	uuid[0] = rnd1 >> 24;
	uuid[1] = rnd1 >> 16;
	uuid[2] = rnd1 >> 8;
	uuid[3] = rnd1;
	uint32_t rnd2 = get_rand_32();
	uuid[4] = rnd2 >> 24;
	uuid[5] = rnd2 >> 16;
	uuid[6] = rnd2 >> 8;
	uuid[7] = rnd2;
	uint32_t rnd3 = get_rand_32();
	uuid[8] = rnd3 >> 24;
	uuid[9] = rnd3 >> 16;
	uuid[10] = rnd3 >> 8;
	uuid[11] = rnd3;
	uint32_t rnd4 = get_rand_32();
	uuid[12] = rnd4 >> 24;
	uuid[13] = rnd4 >> 16;
	uuid[14] = rnd4 >> 8;
	uuid[15] = rnd4;

	uuid[6] = 0x40 | (0x0F & uuid[6]); // version 4 / random based
	uuid[8] = 0x80 | (0x3F & uuid[8]); // variant RFC9562
	}

/**
 * @brief genertate a random UUID
 * 
 * @return uint8_t* uuid
 */
uint8_t* generateUUID() {
	static uint8_t uuid[16];
	uint32_t rnd1 = get_rand_32();
	uuid[0] = rnd1 >> 24;
	uuid[1] = rnd1 >> 16;
	uuid[2] = rnd1 >> 8;
	uuid[3] = rnd1;
	uint32_t rnd2 = get_rand_32();
	uuid[4] = rnd2 >> 24;
	uuid[5] = rnd2 >> 16;
	uuid[6] = rnd2 >> 8;
	uuid[7] = rnd2;
	uint32_t rnd3 = get_rand_32();
	uuid[8] = rnd3 >> 24;
	uuid[9] = rnd3 >> 16;
	uuid[10] = rnd3 >> 8;
	uuid[11] = rnd3;
	uint32_t rnd4 = get_rand_32();
	uuid[12] = rnd4 >> 24;
	uuid[13] = rnd4 >> 16;
	uuid[14] = rnd4 >> 8;
	uuid[15] = rnd4;
	// add uuid variant and version
	uuid[6] = 0x40 | (0x0F & uuid[6]); // version 4 / random based
	uuid[8] = 0x80 | (0x3F & uuid[8]); // variant RFC9562
	return uuid;
	}

/**
 * @brief verify if UUID is RFC9562 conform
 * 
 * @param uuid 
 * @return uint8_t version number, 0 if fail
 */
uint8_t verifyUUID(uint8_t uuid[]) {
	uint8_t version = uuid[6] >> 4;
	if ((version > 0) && (version < 8) && (uuid[8] >> 6 == 0x02)) { // check for version and variant
		return version;
		}
	else {
		return 0;
		}
	}

/**
 * @brief converts the UUID to a string
 * 
 * @param uuid 
 * @param uuidString 36 chars wide
 */
void printUUID(uint8_t uuid[], char uuidString[]) {
	sprintf(uuidString, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
	uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
	uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	}

/**
 * @brief converts the UUID to a string
 * 
 * @param uuid 
 * @return char* uuid tring 
 */
char* printUUID(uint8_t uuid[]) {
	static char uuidString[37];
	sprintf(uuidString, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
	uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
	uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
	return uuidString;
	}

/**
 * @brief generate a random mac address for local use
 * 
 * @param mac 
 */
void generateMAC(uint8_t mac[]) {
	uint32_t rnd1 = get_rand_32();
	mac[0] = rnd1 >> 24;
	mac[1] = rnd1 >> 16;
	mac[2] = rnd1 >> 8;
	mac[3] = rnd1;
	uint32_t rnd2 = get_rand_32();
	mac[4] = rnd2 >> 24;
	mac[5] = rnd2 >> 16;

	mac[0] &= ~(1 << 0); // unicast = 0, multicast / broadcast = 1
	mac[0] |= 1 << 1; // local = 1, universal = 0
	mac[0] &= ~(1 << 2) & ~(1 << 3); // administratively assigned
	}

/**
 * @brief generate a random mac address for local use
 * 
 * @return uint8_t* MAC address
 */
uint8_t* generateMAC() {
	static uint8_t mac[6];
	uint32_t rnd1 = get_rand_32();
	mac[0] = rnd1 >> 24;
	mac[1] = rnd1 >> 16;
	mac[2] = rnd1 >> 8;
	mac[3] = rnd1;
	uint32_t rnd2 = get_rand_32();
	mac[4] = rnd2 >> 24;
	mac[5] = rnd2 >> 16;

	mac[0] &= ~(1 << 0); // unicast = 0, multicast / broadcast = 1
	mac[0] |= 1 << 1; // local = 1, universal = 0
	mac[0] &= ~(1 << 2) & ~(1 << 3); // administratively assigned
	return mac;
	}

/**
 * @brief converts the MAC to a string
 * 
 * @param mac 
 * @param macString 
 */
void printMAC(uint8_t mac[], char macString[]) {
	sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X",
	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

/**
 * @brief converts the MAC to a string
 * 
 * @param mac 
 * @return char* MAC string
 */
char* printMAC(uint8_t mac[]) {
	static char macString[18];
	sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X",
	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return macString;
	}

#endif