#include "Arduino.h"

#include <EEPROM.h>
#include <stdint.h>

#include "Node.h"
#include "Packet.h"
#include "iRadio.h"
#include "iSensor.h"

// Static declarations
// Private
bool Node::_waitForAcknowledgement = false;
bool Node::_readyToForward = false;

iSensor *Node::_sensor;
iRadio *Node::_radio;

// Public
unsigned short Node::crcTable[256];
int Node::nodeID = -1;
int Node::parentID = -1;


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
        int ID = loadID();
        if(nodeID != 0)
        {
            nodeID = ID;
            printf("Har nodeID: %d\n", nodeID);
        }
        else
        {
            printf("Har ingen nodeID..\n");
        }
        
        while(true)
        {
            // Laeser fra radio og laver til pakke
            char *res = _radio->listen();
            Packet packet(res);
            handlePacket(packet);
        }
    }
}

void Node::saveID(int16_t id)
{
  char *val = (char *)&id;
  EEPROM.write(0,val[0]);
  EEPROM.write(1,val[1]);
  
  printf("%d\n",val[0]);
  printf("%d\n",val[1]);
}

int16_t Node::loadID()
{
  char *val = (char *)malloc(2);
  val[0] = EEPROM.read(0);
  val[1] = EEPROM.read(1);
  
  int16_t id = 0;
  memcpy(&id,val,2);
  free(val);
  return id;
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
                printf("Modtaget acknowledgement fra %d\n", packet.addresser);
                _waitForAcknowledgement = false;
                _readyToForward = true;
                shouldKeepSendingPacket = false;
            }
        }
        break;
        case DataRequest: // Request modtaget
        {
            printf("Har modtaget datarequest. Sender data tilbage!\n");
            parentID = packet.addresser;
            
            if (!_waitForAcknowledgement && !_readyToForward)
            {
                _waitForAcknowledgement = true;
                shouldKeepSendingPacket = true;
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
            printf("Modtaget ack paa pair!\n");
            if(nodeID == -1) // Default ID
            {
                printf("Har nu faaet ID: %d\n", packet.addressee);
                
                saveID(packet.addressee);
                shouldKeepSendingPacket = false;
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
    int startingWait = 200; // 1 ms. start
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
            Packet receivedPacket(res);

            if(receivedPacket.packetType != 0) // Ikke en error
            {
                handlePacket(receivedPacket);
                printf("Packet haandteret!\n");
            }
        }
        
        if(!shouldKeepSendingPacket)
        {
            break;
        }
        tmpWait = nextExponentialBackoff(tmpWait);
    }
    
    free(packetCoding);
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

void Node::sendRequests()
{
  
}

// Request modtager. Send data hjem, og request videre
void Node::readPackSend()
{
    int sensorData = 100; //_sensor->read(); // Read
    Packet dataPacket(Data, nodeID, parentID, nodeID, sensorData, 0, 0);
    beginBroadcasting(dataPacket);
}


// Send data videre til min forlaeldr!
void Node::forwardSignal(Packet packet)
{
    // acket(PacketType packetTypeInput, uint16_t addresserInput, uint16_t addresseeInput, uint16_t originInput, uint16_t sensor1Input,
//	uint16_t sensor2Input, uint16_t sensor3Input)

    // Send acknowledgement til sender af data
    Packet acknowledgement(DataAcknowledgement, nodeID, parentID, packet.origin, packet.sensor1, packet.sensor2, packet.sensor3);
    
    
    // Hent min foraeldr
    // Relay data til foraeldr
    
}
