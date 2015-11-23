#include <stdint.h>

#include <stdio.h>
#include "Node.h"
#include "Packet.h"
#include "Radio.h"

// Static declarations
bool Node::_waitForAcknowledgement = true;
bool Node::_readyToForward = true;
iRadio *Node::_radio;
unsigned short Node::crcTable[256];

int main(int argc, char* argv[])
{
    Node::initializeNode();
    Node::begin();
    return 0;
}

// Sætter variabler op i Node
void Node::initializeNode()
{
    printf("\nNode is being initialized.");
    crcInit();
    printf("\ncrc has been set up.");
    _radio = new NRF24Radio();
    printf("\nDone initializing.");
}

// Starter hele lortet!
void Node::begin()
{
    // Lser fra radio
    
            Packet ackPacket(DataAcknowledgement, 0, 4, 0, 3, 0, 0); 
            printf("\n%d\n\n",sizeof(Packet));
            char *encoded = ackPacket.encode();
            Packet rebuild(encoded);
            char *reencoded = rebuild.encode();
            for (int i = 0; i < 16; i++)
            {
                printf("\n%d - %d",(int)encoded[i], (int)reencoded[i]);
            }
            //printf("\n");
            //printf(PairRequestAcknowledgement);
            printf("\n");            
            _radio->broadcast(ackPacket.encode());
    return;
    while(true)
    {
        char *res = _radio->listen();
        Packet packet(res);
        printf("\nPacket received with type: %d",packet.packetType);
        handlePacket(packet);
        printf("\nPacket er handlet.");
        fflush(stdout);
    }
}

// Fill crcTable with values
void Node::crcInit()
{
    unsigned short remainder; // 2 byte remainder (according to CRC16/CCITT standard)
    unsigned short dividend; // What are you?
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

void Node::handlePacket(Packet packet)
{
    switch(packet.packetType)
    {
        case DataAcknowledgement: // Acknowledgement modtaget (Dvs. mit data er accepteret)
        {
            if (_waitForAcknowledgement)
            {
                _waitForAcknowledgement = false;
                _readyToForward = true;
            }
        }
        break;
        case DataRequest: // Request modtaget
        {
        }
        break;
        case Data: // Har modtaget data der skal videresendes
        {
        }
        break;
        case PairRequest :
        {
            //Print
            Packet ackPacket(PairRequestAcknowledgement, 0, 1337, 0, 0, 0, 0);
            _radio->broadcast(ackPacket.encode());
        }
        break;
        case PairRequestAcknowledgement: // Har modtaget mit ID
        {
        }
        break;
        case ClearSignal: // Slet alt!
        {
            // I had nothing to do with it!
        }
        break;
        default:
            //std::cout << "Hello? Yes, this is default. No this is patrick!";
            // Hello default, this is broken!
        break;
    }
}
