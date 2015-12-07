#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PACKET_H
#define PACKET_H

#define POLYNOMIAL			0x1021
#define INITIAL_REMAINDER	0x1D0F
#define FINAL_XOR_VALUE		0x0000
#define WIDTH 	(sizeof(unsigned short)*8)
#define TOPBIT	(1 << (WIDTH - 1))

enum PacketType : uint16_t {
    Error,
    DataAcknowledgement,
    DataRequest,
    Data,
    PairRequest,
    PairRequestAcknowledgement,
    ClearSignal
};

class Packet
{
public:
    //fields
    PacketType packetType;
    uint16_t addresser;
    uint16_t addressee;
    uint16_t origin;
    uint16_t value1;
    uint16_t value2;
    uint16_t value3;
    uint16_t checksum;
   
    //crc 
    static unsigned short crcTable[256];
    static void crcInit();

    //methods
    char *encode();
    bool verified();
    void updateChecksum();
    
    //constructors
    Packet(char *);
    Packet(PacketType, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);

private:
    void decode(char *);
    uint16_t getChecksum(unsigned char *, unsigned int);
};

#endif
