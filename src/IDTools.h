/* Arduino library for pseudo randomized UUID and MAC address
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

/* CID Tools following RFC4122
 * To initialise the pseudo random generators add an usefull input like AnalogIn(x)
 * At least you should use hardware chips with included serial and mac address like 
 * AT24MAC402/AT24MAC602 from microchip
*/

#ifndef ID_TOOLS_H
#define ID_TOOLS_H

#include "Arduino.h"

/**
 * @brief genertate a random CID / UUID
 * 
 * @param uuid 
 * @param srnd start number for random function
 */
void generateCID(uint8_t uuid[], unsigned int srnd) {
	srand(srnd);
	for (uint8_t i = 0; i < 16; i++) {
		uuid[i] = rand() %256;
		}
	uuid[6] = 0x40 | (0x0F & uuid[6]); // version 4 / random based
	uuid[8] = 0x80 | (0x3F & uuid[8]); // variant RFC4122
	}

/**
 * @brief genertate a random CID / UUID
 * 
 * @param srnd start number for random function
 * @return uint8_t* uuid
 */
uint8_t* generateCID(unsigned int srnd) {
	static uint8_t uuid[16];
	srand(srnd);
	for (uint8_t i = 0; i < 16; i++) {
		uuid[i] = rand() %256;
		}
	// add uuid variant and version
	uuid[6] = 0x40 | (0x0F & uuid[6]); // version 4 / random based
	uuid[8] = 0x80 | (0x3F & uuid[8]); // variant RFC4122
	return uuid;
	}

/**
 * @brief verify if CID / UUID is RFC4122 conform
 * 
 * @param uuid 
 * @return true if variant and version ok
 * @return false 
 */
bool verifyCID(uint8_t uuid[]) {
	if (uuid[6] >> 4 == 0x04 && uuid[8] >> 6 == 0x02) {
		return true;
		}
	else {
		return false;
		}
	}

/**
 * @brief converts the CID / UUID to a string
 * 
 * @param cid 
 * @param cidString 
 */
void CIDtoString(uint8_t cid[], char cidString[]) {
	sprintf(cidString, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
	cid[0], cid[1], cid[2], cid[3], cid[4], cid[5], cid[6], cid[7],
	cid[8], cid[9], cid[10], cid[11], cid[12], cid[13], cid[14], cid[15]);
	}

/**
 * @brief converts the CID / UUID to a string
 * 
 * @param cid 
 * @return char* cid tring 
 */
char* CIDtoString(uint8_t cid[]) {
	static char cidString[70];
	sprintf(cidString, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
	cid[0], cid[1], cid[2], cid[3], cid[4], cid[5], cid[6], cid[7],
	cid[8], cid[9], cid[10], cid[11], cid[12], cid[13], cid[14], cid[15]);
	return cidString;
	}

/**
 * @brief generate a random mac address for local use
 * 
 * @param mac 
 * @param srnd start number for random function
 */
void generateMAC(uint8_t mac[], unsigned int srnd) {
	srand(srnd);
	for (uint8_t i = 0; i < 6; i++) {
		mac[i] = rand() %256;
		}
	mac[0] &= ~(1 << 0); // unicast = 0, multicast / broadcast = 1
	mac[0] |= 1 << 1; // local = 1, universal = 0
	}

/**
 * @brief generate a random mac address for local use
 * 
 * @param srnd start number for random function
 * @return uint8_t* MAC address
 */
uint8_t* generateMAC(unsigned int srnd) {
	static uint8_t mac[6];
	srand(srnd);
	for (uint8_t i = 0; i < 6; i++) {
		mac[i] = rand() %256;
		}
	mac[0] &= ~(1 << 0); // unicast = 0, multicast / broadcast = 1
	mac[0] |= 1 << 1; // local = 1, universal = 0
	return mac;
	}

/**
 * @brief converts the MAC to a string
 * 
 * @param mac 
 * @param macString 
 */
void MACtoString(uint8_t mac[], char macString[]) {
	sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X",
	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

/**
 * @brief converts the MAC to a string
 * 
 * @param mac 
 * @return char* MAC string
 */
char* MACtoString(uint8_t mac[]) {
	static char macString[31];
	sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X",
	mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return macString;
	}

#endif
