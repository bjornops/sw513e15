#include "Packet.h"
#include "Node.h"
#include "iRadio.h"

Packet::Packet(char *input)
{
    decode(input);
    verified();
    printf("Pakke lavet med type: %d", this->packetType);
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
    this->checksum = getChecksum((unsigned char*)encode(), 14);
}


char *Packet::encode()
{
    char *returnstring;
    returnstring = (char*)malloc(sizeof(Packet));

    memcpy(returnstring, this, sizeof(Packet));

    return returnstring;
}

bool Packet::verified()
{
    unsigned char *encoded = (unsigned char*)encode();
    if (getChecksum(encoded, 16) == 0)
    {
        free(encoded)
        return true;
    }
    
    free(encoded)
    
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
        remainder = Node::crcTable[byte] ^ (remainder << 8);
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
