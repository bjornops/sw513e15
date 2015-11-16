#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define size_m 14
#define size_t 16
#define POLYNOMIAL			0x1021
#define INITIAL_REMAINDER	0x1D0F
#define FINAL_XOR_VALUE		0x0000
#define WIDTH 	(sizeof(unsigned short)*8)
#define TOPBIT	(1 << (WIDTH - 1))

enum PacketType : uint16_t {
    Acknowledgement,
    Request,
    Data,
    PairRequest,
    PairRequestAcknowledgement,
    ClearSignal
};

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
    
    char *encode();
    bool verified();
    
    Packet(char *);
    Packet(PacketType, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);

private:
    void decode(char *);
    uint16_t getChecksum(unsigned char, unsigned int);
};
