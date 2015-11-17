#include "Arduino.h"

#include <stdint.h>

#include "Node.h"
#include "Packet.h"
#include "iRadio.h"
#include "iSensor.h"

// Static declarations
bool Node::_waitForAcknowledgement = true;
bool Node::_readyToForward = true;
iSensor *Node::_sensor;
iRadio *Node::_radio;
unsigned short Node::crcTable[256];

// Sætter variabler op i Node
void Node::initializeNode(iSensor *sensor, iRadio *radio)
{
    printf("\nNode klar!\n");
    crcInit();
    
    _sensor = sensor;
    _radio = radio;
}

// Starter hele lortet!
void Node::begin(bool sendPairRequest)
{
    if(sendPairRequest)
    {
        printf("Sender pair request!\n");
        sendPairRequest();
    }
    else
    {
        printf("Begynder at lytte.!");
        
        // Find dit ID her.. (Evt. brug EEPROM bibliotek)
        
        
        // Laeser fra radio og laver til pakke
        char *res = _radio->listen();
        Packet packet(res);
        handlePacket(packet);
    }
}

// Sender pair request
void Node::sendPairRequest()
{
    Packet requestPacket(PairRequest, 0, 0, 0, 0, 0, 0); // Data does not matter, only need 'type'.
    beginBroadcasting(requestPacket);
}

// Begynder at sende pakke indtil den bliver bedt om at stoppe! (Exponential backoff handler!)
void Node::beginBroadcasting(Packet packet)
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
            // Ignorer hvis paa almindelig Node.
        }
        break;
        case PairRequestAcknowledgement :
        {
            nodeID = packet.addressee;
        }
        break;
        case ClearSignal: // Slet alt!
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
    int sensorData = _sensor->read(); // Read
    
}

void Node::forwardSignal(Packet packet)
{
}
