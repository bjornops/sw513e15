#include "Packet.h"

	Packet::Packet(char *input)
	{
		decode(input);
	}

	Packet::Packet(PacketType packetTypeInput, uint16_t addresserInput, uint16_t addresseeInput, uint16_t originInput, uint16_t sensor1Input,
		uint16_t sensor2Input, uint16_t sensor3Input)
	{
		this->packetType = packetTypeInput;
		this->addresser = addresserInput;
		this->addressee = addresseeInput;
		this->origin = originInput;
		this->sensor1 = sensor1Input;
		this->sensor2 = sensor2Input;
		this->sensor3 = sensor3Input;
		this->checksum = getChecksum();
	}


	char* Packet::encode(){

		char *returnstring;
		returnstring = (char*)malloc(sizeof(Packet));

		memcpy(returnstring, this, sizeof(Packet));


		return returnstring;
	}
  
  bool Packet::verified()
	{
		if (getChecksum((unsigned char*)encode(), 16) == 0)
		{
			return true;
		}
		return false;
	}


private:
	void Packet::decode(string input)
	{
		if (input.length() == 15)
		{
			memcpy(this, input.c_str(), sizeof(Packet)-1);
		}
		else
		{
			throw new exception("String not accepted for decoding");
		}
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

		char *toBeswapped = (char*)malloc(sizeof(char)*2);
		memcpy(toBeswapped, (char*)&result, sizeof(char)*2);
		char temp = toBeswapped[1];
		toBeswapped[1] = toBeswapped[0];
		toBeswapped[0] = temp;

		memcpy((char*)&result, toBeswapped, sizeof(char) * 2);
		return result;
	}
	
	bool Packet::correctChecksum(unsigned char *message, unsigned int nBytes)
	{
		if(message.substr(0,2) == getChecksum(message.substr(2,13), 13))
		{
			return true;
		}
		return false;
	}
