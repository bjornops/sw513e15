#include <iostream>
#include <stdint.h>
#include "Node.h"

bool Node::_waitForAcknowledgement = true;
bool Node::_readyToForward = true;
iSensor *Node::_sensor = NULL;
unsigned short Node::crcTable[256];

int main(int argc, char *argv[])
{
    Node::crcInit();
    MoistureSensor sensor(1);
    
    std::cout << "Main is running! \\o/" <<  std::endl << "Sensor value: " <<  sensor.read() << std::endl;
    
    return 0;
}

void Node::crcInit() //fill crcTable with values
{
    unsigned short remainder;	    // 2 byte remainder (according to CRC16/CCITT standard)
    unsigned short dividend;		// What are you?
	int bit;			// bit counter
	
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

    // FROM PACKETHANDLER //
void Node::handlePacket(Packet packet)
{
    switch(packet.packetType)
    {
        case Acknowledgement :
        {
            if (_waitForAcknowledgement)
            {
                _waitForAcknowledgement = false;
                _readyToForward = true;
            }
        }
        break;
        case Request :
        {
            if (!_waitForAcknowledgement && !_readyToForward)
            {
                _waitForAcknowledgement = true;
                readPackSend();
            }
        }
        break;
        case Data :
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
            std::cout << "Hello? Yes, this is default.";
        break;
    }
}

void Node::readPackSend()
{
    int sensorData = _sensor->read();              // Read
    
}

void Node::forwardSignal(Packet packet)
{
}
