#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "Node.h"
#include "Packet.h"
#include "Radio.h"

#define MAIN_NODE_ID 0

void sigal(int);
void getAndSavePID();

// Static declarations
bool Node::_waitForAcknowledgement = true;
bool Node::_readyToForward = true;
iRadio *Node::_radio;
unsigned short Node::crcTable[256];

int main(int argc, char* argv[])
{
    getAndSavePID();

    Node::initializeNode();
    Node::begin();

    return 0;
}

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

void sigal(int al)
{
    printf("Signal alarm (Sender requests!)!\n");
    fflush(stdout);
    Node::sendRequest(1, 31); // Meget tilfældigt interval.
}

// Sætter variabler op i Node
void Node::initializeNode()
{
    crcInit();
    _radio = new NRF24Radio();

    printf("\nDone initializing.");
    fflush(stdout);
}

// Starter hele lortet!
void Node::begin()
{
    signal(SIGALRM, sigal);

    // Læser fra radio
    while(true)
    {
        printf("i loop i begin!\n");
        char *res = _radio->listen();
        Packet packet(res);
        printf("\nPacket received with type: %d\n",packet.packetType);
        handlePacket(packet);

        printf("\nPacket er handlet.\n");
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
        case Data: // Har modtaget data der skal gemmes!
        {
            printf("Har modtaget data fra node med ID: %d\n", packet.addresser);
            fflush(stdout);

            Packet ackPacket(DataAcknowledgement, MAIN_NODE_ID, packet.addresser, MAIN_NODE_ID, 0, 0, 0);
            char *enc = ackPacket.encode();
            _radio->broadcast(enc);
            free(enc);
        }
        break;
        case PairRequest :
        {
            //Print
            Packet ackPacket(PairRequestAcknowledgement, 0, 1337, 0, 0, 0, 0);
            char *enc = ackPacket.encode();
            _radio->broadcast(enc);
            free(enc);
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
        sendRequest(turn+1, delay+30); // Meget tilfældigt interval.
    }
    else
    {
        printf("Sender ikke flere requests!\n");
        Node::begin();
    }
}
