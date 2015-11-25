#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "Node.h"
#include "Packet.h"
#include "Radio.h"

#define MAIN_NODE_ID 0
#define REQUEST_GENERATE_ID_TIMER 5000

void sigal(int);
void getAndSavePID();

// Static declarations
unsigned short Node::crcTable[256];

iRadio *Node::_radio;
int Node::_currentID = 0;
unsigned int Node::_lastPairRequestMillis = 0;
std::map<int, bool> Node::_receivedThisSession;

int main(int argc, char* argv[])
{
    signal(SIGALRM, sigal);
    getAndSavePID();

    Node::initializeNode();
    Node::begin();

    return 0;
}

// Finder og gemmer PID så det kan bruges af andre!
void getAndSavePID()
{
    int myPID = getpid();
    FILE *f;
    f = fopen("/var/run/wasp.pid", "w");
    if(f == NULL)
    {
        return;
    }

    fprintf(f, "%d", myPID);
    fclose(f);
}

// Når vi får en 'alarm', send requests
void sigal(int al)
{
    printf("Signal alarm (Sender requests!)!\n");
    fflush(stdout);
    
    Node::sendRequest(1, 31); // Meget tilfældigt interval.
    
    signal(al, sigal);
}

// Sætter variabler op i Node
void Node::initializeNode()
{
    Node::crcInit();
    
    _radio = new NRF24Radio();
    _lastPairRequestMillis = bcm2835_millis();
    
<<<<<<< HEAD
    // TODO: Find alle kendte noder her!
=======
    // Find kendte noder (i home/pi/wasp.conf)
    FILE *optionsFile;
    optionsFile = fopen("/home/pi/wasp.conf", "a+");
    if(optionsFile != NULL)
    {
        int nodeID = 0;
    	while(fscanf(optionsFile, "%d", &nodeID) != EOF)
    	{
            printf("Kendt node: %d\n", nodeID);
            Node::_receivedThisSession[nodeID] = false;
        }
    }
>>>>>>> origin/master
    
    printf("Done initializing.\n");
    fflush(stdout);
}

// Starter hele lortet!
void Node::begin()
{
    // Læser fra radio
    while(true)
    {
        printf("Begynder at lytte efter pakke!\n");
        
        char *res = _radio->listen();
        Packet packet(res);
        printf("Packet received with type: %d\n", packet.packetType);
        Node::handlePacket(packet);

        printf("Packet er handlet.\n");
        fflush(stdout);
    }
}

void Node::handlePacket(Packet packet)
{
    switch(packet.packetType)
    {
        case Data: // Har modtaget data der skal gemmes!
        {
            printf("Noden %d sender værdien %d.\n", packet.origin, packet.sensor1);
            fflush(stdout);
            
            // TODO: Gem data
            // TODO: Undersøg om vi er færdige, eller vi venter flere
            
            // Send acknowledgement
            Packet ackPacket(DataAcknowledgement, MAIN_NODE_ID, packet.addresser, MAIN_NODE_ID, 0, 0, 0);
            char *enc = ackPacket.encode();
            _radio->broadcast(enc);
            free(enc);
        }
        break;
        
        case PairRequest: // En eller anden stakkel vil gerne have et ID.
        {
            // Skal der genereres et nyt ID?
            unsigned int currentMillis = bcm2835_millis();
            if(currentMillis - _lastPairRequestMillis >= REQUEST_GENERATE_ID_TIMER)
            {
                _currentID += 1;
                printf("Genererer nyt ID til node: %d\n", _currentID);
                fflush(stdout);
            }
                
            _lastPairRequestMillis = bcm2835_millis();
            Packet ackPacket(PairRequestAcknowledgement, 0, _currentID, 0, 0, 0, 0);
            char *enc = ackPacket.encode();
            _radio->broadcast(enc);
            free(enc);
        }
        break;
        
        case PairRequestAcknowledgement:
        case DataAcknowledgement:
        case DataRequest:
        case ClearSignal:
        default:
            //std::cout << "Hello? Yes, this is default.";
            // Hello default, this is broken!
        break;
    }
}

void Node::sendRequest(int turn, int delay)
{
    // Send pakke
    Packet requestPacket(DataRequest, 0, MAIN_NODE_ID, 0, 0, 0, 0);
    char *enc = requestPacket.encode();
    _radio->broadcast(enc);
    free(enc);

    // Vent og send igen!
    if(turn+1 <= 6)
    {
        bcm2835_delay(delay);
        Node::sendRequest(turn+1, delay+12); // Meget tilfældigt interval.
    }
    else
    {
        printf("Sender ikke flere requests!\n");
        //Node::begin();
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