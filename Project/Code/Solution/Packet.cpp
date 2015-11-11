#include "Packet.h"

	Packet::Packet(string input)
	{
		if(correctChecksum(input))
		{
			decode(input);
		}
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


	string Packet::encode(){

		char returnstring[sizeof(Packet)-1];

		memcpy(returnstring, this, sizeof(Packet)-1);

		string mystring = string(returnstring, returnstring + sizeof(Packet)-1);

		return mystring;
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
		
		for(offset = 0; offset < nBytes; offset++){
			byte = (remainder >> (WIDTH - 8)) ^ message[offset];
			remainder = crcTable[byte] ^ (remainder << 8);
		}
		return (remainder ^ FINAL_XOR_VALUE);
	}
	
	bool Packet::correctChecksum(unsigned char *message, unsigned int nBytes)
	{
		if(message.substr(0,2) == getChecksum(message.substr(2,13), 13))
		{
			return true;
		}
		return false;
	}
