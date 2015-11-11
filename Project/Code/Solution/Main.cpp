#include <iostream>

#include "MoistureSensor.h" // Burde jo .h   men altså.. Det virker!
#include "Main.h"
#include "Packet.h"
//#include "RadioHandler.h"

int main(int argc, char *argv[])
{
    _waitForAcknowledgement = true;
    _readyToForward = false;
    
    MoistureSensor sensor(1);
    
    std::cout << "Main is running! \\o/" <<  std::endl << "Sensor value: " <<  sensor.read() << std::endl;
    
    return 0;
}

// CRC shit
unsigned short crcTable[256];
void crcInit(){//fill crcTable with values
	width remainder;	// 2 byte remainder (according to CRC16/CCITT standard)
	width dividend;		// What are you?
	int bit;			// bit counter
	
	for(dividend = 0; dividend < 256; dividend++){ //foreach value of 2 bytes/8 bits
		remainder = dividend << (WIDTH - 8);//
		
		for(bit = 0; bit < 8; bit++){
			if (remainder & TOPBIT){ // MSB = 1 => divide by POLYNOMIAL
				remainder = (remainder << 1) ^ POLYNOMIAL;//scooch and divide
			}else{
				remainder = remainder << 1;//scooch and do nothing (MSB = 0, move along)
			}
		}
		crcTable[dividend] = remainder;//save current crc value in crcTable
	}
}

    // FROM PACKETHANDLER //
void handlePacket(Packet packet)
{
    switch(packet.type);
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

void readPackSend()
{
    int sensorData = sensor -> read();              // Read
    
    Packet packet = new Packet(sensorData);         // Pack
    
    _radioHandler -> broadcast(packet->toString()); // Send
    _radioHandler -> waitForAccept();
}

void forwardSignal(Packet packet)
{
    _radioHandler -> broadcast(packet->toString()); //TODO
    _radioHandler -> waitForAccept();
}