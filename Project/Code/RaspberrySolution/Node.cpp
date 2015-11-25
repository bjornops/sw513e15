#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "Node.h"
#include "Packet.h"
#include "Radio.h"

#define MAIN_NODE_ID 0
#define REQUEST_GENERATE_ID_TIMER 5000

void signalHandler(int);
void getAndSavePID();

// Static declarations
unsigned short Node::crcTable[256];

iRadio *Node::_radio;
int Node::_currentID = 0;
unsigned int Node::_lastPairRequestMillis = 0;
std::map<int, int> Node::_receivedThisSession;

int main(int argc, char* argv[])
{
    signal(SIGUSR1, signalHandler);
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
void signalHandler(int signum)
{
    printf("Signal modtaget (Sender requests!)!\n");
    fflush(stdout);
    
    Node::sendRequest(); //Mængde af forsøg defineret i funktion.
    printf("Signal handling overstået\n");
}

// Sætter variabler op i Node
void Node::initializeNode()
{
    Node::crcInit();
    
    Node::_radio = new NRF24Radio();
    Node::_lastPairRequestMillis = bcm2835_millis();
    
    // Find kendte noder (i home/pi/wasp.conf)
    FILE *optionsFile;
    optionsFile = fopen("/home/pi/wasp/wasp.conf", "a+"); 
    if(optionsFile != NULL)
    {
        int nodeID = 0;
    	while(fscanf(optionsFile, "%d", &nodeID) != EOF)
    	{
            printf("Kendt node: %d\n", nodeID);
            Node::_receivedThisSession[nodeID] = -1;
        }
    }

    //setup rand() til brug i exponential backoff
    srand(time(NULL));
    
    printf("Done initializing.\n");
    fflush(stdout);
}

// Starter hele lortet!
void Node::begin()
{
    printf("Begynder at lytte efter pakker.\n");

    // Læser fra radio
    char *res = (char *)malloc(32*sizeof(char));
    while(true)
    {        
        res = _radio->listen();
        Packet packet(res);

        Node::handlePacket(packet);
        
        printf("Færdig med at handle packet");
        fflush(stdout);
    }
}

void Node::handlePacket(Packet packet)
{
    switch(packet.packetType)
    {
        case Data: // Har modtaget data der skal gemmes!
        {
            printf("Noden %d er værdien %d.\n", packet.origin, packet.sensor1);
            fflush(stdout);
            
            if(Node::_receivedThisSession[packet.origin] == -1)
            {
                // Ikke modtaget før, så gem værdi!
                Node::_receivedThisSession[packet.origin] = packet.sensor1;
                
                // Send acknowledgement
                Packet ackPacket(DataAcknowledgement, MAIN_NODE_ID, packet.addresser, MAIN_NODE_ID, 0, 0, 0);
                char *enc = ackPacket.encode();
                _radio->broadcast(enc);
                free(enc);
                
                // Er vi færdige?
                if(Node::receivedFromAllNodes())
                {
                    Node::saveSessionResults();
                    printf("Alt done. Yay!");
                }
            }
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
            // Hello? Yes, this is default.
            // Hello default, this is broken!
            // No, this is Patrick.
        break;
    }
}

// Gemmer denne sessions resulteter
void Node::saveSessionResults()
{
    FILE *resFile = fopen("/home/pi/wasp/results/test.txt", "w");

    if(resFile != NULL) 
    {
        std::map<int, int>::iterator it;
        for (it = Node::_receivedThisSession.begin(); it != Node::_receivedThisSession.end(); it++)
        {
            fprintf(resFile, "%d:%d\n", it->first, it->second);
        }
    }
    
    fclose(resFile);

    Node::clearSession();
}

// Renser section
void Node::clearSession()
{
    std::map<int, int>::iterator it;
    for (it = Node::_receivedThisSession.begin(); it != Node::_receivedThisSession.end(); it++)
    {
        Node::_receivedThisSession[it->first] = -1;
    }
}

bool Node::receivedFromAllNodes()
{
    bool done = true;
    
    std::map<int, int>::iterator it;
    for (it = Node::_receivedThisSession.begin(); it != Node::_receivedThisSession.end(); it++)
    {
        if(it->second == -1)
        {
            done = false;
            break;
        }
    }
    
    return done;
}

void Node::sendRequest()
{
    int attemptsToDo = 6;
    
    //Byg pakke
    Packet requestPacket(DataRequest, 0, MAIN_NODE_ID, 0, 0, 0, 0);
    char *enc = requestPacket.encode();

    //Try it
    for (int i = 1; i <= attemptsToDo; i++)
    {
        // Broadcast
        _radio->broadcast(enc);
        
        // Backoff
        nextExponentialBackoffDelay(i);
    }
    
    //Ryd op
    free(enc);
}

void Node::nextExponentialBackoffDelay(int attemptNumber)
{   
    //Delay mellem 1 og 1 * 2 ^ ( attemptnumber - 1 )
    int delay = (rand() % (1<<(attemptNumber-1))) + 1;
    bcm2835_delay(delay);
}

// Fill crcTable with values
void Node::crcInit()
{
    unsigned short remainder; // 2 byte remainder (according to CRC16/CCITT standard)
    unsigned short dividend;  // What are you?
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
