#include "Arduino.h"

#include <EEPROM.h>
#include <stdint.h>

#include "Node.h"
#include "Packet.h"
#include "iRadio.h"
#include "iSensor.h"

// Static declarations
// Private
bool Node::_waitForAcknowledgement = true;
bool Node::_readyToForward = true;

iSensor *Node::_sensor;
iRadio *Node::_radio;

// Public
unsigned short Node::crcTable[256];
int Node::nodeID = -1;


// Andre declarations
static bool shouldKeepSendingPacket = false;
static PacketType currentHandlingPacketType;

// Sætter variabler op i Node
void Node::initializeNode(iSensor *sensor, iRadio *radio)
{
    printf("\nNode klar!\n");
    crcInit();
    
    _sensor = sensor;
    _radio = radio;
    
    randomSeed(analogRead(0));
}

// Starter hele lortet!
void Node::begin(bool shouldSendPairRequest)
{
    if(shouldSendPairRequest)
    {
        printf("Sender pair request!\n");
        sendPairRequest();
    }
    else
    {
        printf("Begynder at lytte.!\n");
        
        // Find dit ID her.. (Evt. brug EEPROM bibliotek)
        int ID = EEPROM.read(0);
        if(nodeID != 0)
        {
            nodeID = ID;
        }
        
        // Laeser fra radio og laver til pakke
        while(true)
        {
            char *res = _radio->listen();
            for(int n = 0; n < 32; n++)
            {
                 printf("Packet (%d): %d\n", n, (uint16_t)res[n]); 
            }
            Packet packet(res);
        }
//        handlePacket(packet);
    }
}

// Sender pair request
void Node::sendPairRequest()
{
    Packet requestPacket(PairRequest, 0, 0, 0, 0, 0, 0); // Data does not matter, only need 'type'.
    currentHandlingPacketType = PairRequest;
    
    beginBroadcasting(requestPacket);
}

// Begynder at sende pakke indtil den bliver bedt om at stoppe! (Exponential backoff handler!)
void Node::beginBroadcasting(Packet packet)
{
    int startingWait = 100; // 1 ms. start
    shouldKeepSendingPacket = true;
    
    broadcast(packet, startingWait);
}

void Node::broadcast(Packet packet, int msWait)
{
    printf("Sender pakke med typen: %d og lytter for %d ms\n", packet.packetType, msWait);
    
    char *packetCoding = packet.encode();
    int tmpWait = msWait;
    char *res;
    
    while(true)
    {
        _radio->broadcast(packetCoding);
        res = _radio->listenFor(tmpWait);
        
        printf("Modtaget: %d\n", (int)res[0]);
        if((int)res[0] != 0) // Data modtaget, bail out!
        {
            printf("Inde i if.");
            Packet receivedPacket(res);
            printf("Packet modtaget har typen: %d", receivedPacket.packetType);
            if(receivedPacket.packetType != 0)
            {
              handlePacket(receivedPacket);
              
              printf("Packet haandteret!\n");
              
              shouldKeepSendingPacket = false;
              break;
            }
        }
        
        printf("Modtog ingen pakke, proever igen!\n");
        tmpWait = nextExponentialBackoff(tmpWait);
    }
}

int Node::nextExponentialBackoff(int cur)
{
   printf("Foer: %d\n", cur);
   int nextBackoff = cur;
   int randAdd = 10; //random(1, 5);
   
   nextBackoff += randAdd;
   printf("Efter: %d\n", nextBackoff);
   /*
   if(nextBackoff >= 1000)
   {
       nextBackoff = randAdd;
   }
   */
   return nextBackoff;
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
        case PairRequestAcknowledgement: // Har modtaget mit ID
        {
            printf("Modtaget ack paa pair!");
            if(nodeID == -1) // Default ID
            {
                printf("Har nu faaet ID: %d\n", packet.addressee);
                nodeID = packet.addressee;
                EEPROM.write(0, nodeID);
            }
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
