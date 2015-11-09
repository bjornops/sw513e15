#include "Packet.h"

	Packet::Packet(string input)
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

	uint16_t Packet::getChecksum()
	{
		return 65535;
	}
