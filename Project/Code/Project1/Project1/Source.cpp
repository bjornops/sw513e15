#include <iostream>
#include <cstdint>
#include <string.h>
#define POLYNOMIAL			0x1021
#define INITIAL_REMAINDER	0x1D0F
#define FINAL_XOR_VALUE		0x0000
#define WIDTH 	(sizeof(unsigned short)*8)
#define TOPBIT	(1 << (WIDTH - 1))


using namespace std;

//Packettype values is of type uint8 to limit memory consumption
enum PacketType : uint8_t {
	Acknowledgement,
	Request,
	Data,
	PairRequest,
	PairRequestAcknowledgement,
	ClearSignal
};
unsigned short crcTable[256];
void crcInit(){//fill crcTable with values
	uint16_t remainder;	// 2 byte remainder (according to CRC16/CCITT standard)
	uint16_t dividend;		// What are you?
	int bit;			// bit counter

	for (dividend = 0; dividend < 256; dividend++){ //foreach value of 2 bytes/8 bits
		remainder = dividend << (WIDTH - 8);//

		for (bit = 0; bit < 8; bit++){
			if (remainder & TOPBIT){ // MSB = 1 => divide by POLYNOMIAL
				remainder = (remainder << 1) ^ POLYNOMIAL;//scooch and divide
			}
			else{
				remainder = remainder << 1;//scooch and do nothing (MSB = 0, move along)
			}
		}
		crcTable[dividend] = remainder;//save current crc value in crcTable
	}
}

class Packet
{
public:
	PacketType packetType;
	uint16_t addresser;
	uint16_t addressee;
	uint16_t origin;
	uint16_t sensor1;
	uint16_t sensor2;
	uint16_t sensor3;
	uint16_t checksum;


	Packet(char *input)
	{
		decode(input);
	}

	Packet(PacketType packetTypeInput, uint16_t addresserInput, uint16_t addresseeInput, uint16_t originInput, uint16_t sensor1Input,
		uint16_t sensor2Input, uint16_t sensor3Input)
	{
		this->packetType = packetTypeInput;
		this->addresser = addresserInput;
		this->addressee = addresseeInput;
		this->origin = originInput;
		this->sensor1 = sensor1Input;
		this->sensor2 = sensor2Input;
		this->sensor3 = sensor3Input;
		this->checksum = getChecksum((unsigned char*)encode(), 14);
	}


	char *encode(){

		char *returnstring;
		returnstring = (char*)malloc(sizeof(Packet));

		memcpy(returnstring, this, sizeof(Packet));


		return returnstring;
	}

	bool Packet::veryfied()
	{
		if (getChecksum((unsigned char*)encode(), 16) == 0)
		{
			return true;
		}
		return false;
	}


private:
	void decode(char *input)
	{
		memcpy(this, input, sizeof(Packet));
	}

	uint16_t Packet::getChecksum(unsigned char *message, unsigned int nBytes)
	{
		unsigned int offset;
		unsigned char byte;
		uint16_t remainder = INITIAL_REMAINDER;

		for (offset = 0; offset < nBytes; offset++){
			byte = (remainder >> (WIDTH - 8)) ^ message[offset];
			remainder = crcTable[byte] ^ (remainder << 8);
		}
		uint16_t result = remainder ^ FINAL_XOR_VALUE;

		char *toBeswapped = (char*)malloc(2);
		memcpy(toBeswapped, (char*)result, 2);
		char temp = toBeswapped[1];
		toBeswapped[1] = toBeswapped[0];
		toBeswapped[0] = temp;

		return (uint16_t)toBeswapped;
	}





};
void printPacket(Packet packet)
{
	cout << "Packet:" << endl << packet.packetType << endl << packet.addresser << endl << packet.addressee << endl
		<< packet.origin << endl << packet.sensor1 << endl << packet.sensor2 << endl << packet.sensor3 << endl << packet.checksum << endl << endl;
}

int main() {

	crcInit();

	Packet packet(PacketType::Request, 0, 0, 65535, 64535, 61535, 62535);

	char *test = packet.encode();

	Packet packet2(test);

	bool veryfied = packet2.veryfied();

	return 0;
}


