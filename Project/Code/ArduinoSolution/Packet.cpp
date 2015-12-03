#include "Packet.h"

// Static declarations
unsigned short Node::crcTable[256];

Packet::Packet(char *input)
{
    decode(input);
    verified();
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
    unsigned char *temp = (unsigned char *)encode();
    this->checksum = getChecksum(temp, 14);
    free(temp);
}

char *Packet::encode()
{
    char *returnstring;
    returnstring = (char*)malloc(sizeof(Packet));

    memcpy(returnstring, this, sizeof(Packet));

    return returnstring;
}

void Packet::updateChecksum()
{
    unsigned char *temp = (unsigned char *)encode();
    this->checksum = getChecksum(temp, 14);
    free(temp);
}

bool Packet::verified()
{
    char *encoded = encode();
    if (getChecksum((unsigned char*)encoded, 16) == 0)
    {
        free(encoded);
        return true;
    }
    
    free(encoded);

    this->packetType = Error;
    return false;
}

void Packet::decode(char *input)
{
    memcpy(this, input, sizeof(Packet));
}

uint16_t Packet::getChecksum(unsigned char *message, unsigned int nBytes)
{
    unsigned int offset;
    unsigned char byte;
    uint16_t remainder = INITIAL_REMAINDER;

    for (offset = 0; offset < nBytes; offset++)
    {
        byte = (remainder >> (WIDTH - 8)) ^ message[offset];
        remainder = Packet::crcTable[byte] ^ (remainder << 8);
    }
    uint16_t result = remainder ^ FINAL_XOR_VALUE;

    char *toBeSwapped = (char*)malloc(sizeof(char)*2);
    memcpy(toBeSwapped, (char*)&result, sizeof(char)*2);
    char temp = toBeSwapped[1];
    toBeSwapped[1] = toBeSwapped[0];
    toBeSwapped[0] = temp;

    memcpy((char*)&result, toBeSwapped, sizeof(char) * 2);
    free(toBeSwapped);
    return result;
}

// Fill crcTable with values
void Node::crcInit()
{
    unsigned short remainder; // 2 byte remainder (according to CRC16/CCITT standard)
    unsigned short dividend;  // What are you?
    int bit; // bit counter

    for(dividend = 0; dividend < 256; dividend++) //foreach value of 2 bytes/8 bits
    {
        remainder = dividend << (WIDTH - 8);//

        for(bit = 0; bit < 8; bit++)
        {
            if(remainder & TOPBIT) // MSB = 1 => divide by POLYNOMIAL
            {
                remainder = (remainder << 1) ^ POLYNOMIAL; //scooch and divide
            }
            else
            {
		        remainder = remainder << 1;//scooch and do nothing (MSB = 0, move along)
	        }
	    }
    	crcTable[dividend] = remainder;//save current crc value in crcTable
    }
}
