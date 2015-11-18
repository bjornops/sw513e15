#include <stdint.h>

#include <stdio.h>
#include "Node.h"
#include "Packet.h"
#include "iRadio.h"

// Static declarations
bool Node::_waitForAcknowledgement = true;
bool Node::_readyToForward = true;
iRadio *Node::_radio;
unsigned short Node::crcTable[256];

int main(int argc, char* argv[])
{
    return 0;
}

// Sætter variabler op i Node
void Node::initializeNode(iRadio *radio)
{
    printf("Node klar!");
    crcInit();
    
    _radio = radio;
}

// Starter hele lortet!
void Node::begin()
{
    // Lser fra radio
    char *res = _radio->listen();
    for(int n = 0; n < 32; n++)
    {
       printf("Char %d: %c - %d\n", n, res[n], (int)res[n]); 
    }
}

// Sender pair request
void Node::sendPairRequest()
{
      
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
        case Acknowledgement: // Acknowledgement modtaget (Dvs. mit data er accepteret)
        {
            if (_waitForAcknowledgement)
            {
                _waitForAcknowledgement = false;
                _readyToForward = true;
            }
        }
        break;
        case Request: // Request modtaget
        {
            if (!_waitForAcknowledgement && !_readyToForward)
            {
                _waitForAcknowledgement = true;
                readPackSend();
            }
        }
        break;
        case Data: // Har modtaget data der skal videresendes
        {
            if (_readyToForward)
            {
                forwardSignal(packet);
            }
        }
        break;
        case PairRequest :
        {
            // Pair me up Scotty!
        }
        break;
        case PairRequestAcknowledgement :
        {
            // Ur paired dood
        }
        break;
        case ClearSignal :
        {
            // I had nothing to do with it!
        }
        break;
        default:
            //std::cout << "Hello? Yes, this is default.";
            // Hello default, this is broken!
        break;
    }
}

void Node::readPackSend()
{    
}

void Node::forwardSignal(Packet packet)
{
}