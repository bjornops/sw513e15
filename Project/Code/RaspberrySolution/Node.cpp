#include "Node.h"

#define MAIN_NODE_ID 0
#define REQUEST_GENERATE_ID_TIMER 5000
#define REQUEST_TIMEOUT 10000

void signalHandler(int);
void getAndSavePID();

iRadio *Node::_radio;
int Node::_currentID = 0;
unsigned int Node::_lastPairRequestMillis = 0;
unsigned int Node::_lastSavedFile = 0;
bool Node::_requested = false;
unsigned int Node::_requestTime = 0;
std::map<int, int> Node::_receivedThisSession;

volatile sig_atomic_t Node::signalReceived = 0;

int main(int argc, char* argv[])
{
    printf("Initializing process ... ");
    signal(SIGUSR1, signalHandler);
    getAndSavePID();
    printf("Done!\n");

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
    Node::setSignalReceived(1);
}

void Node::setSignalReceived(int value)
{
    Node::signalReceived = value;
    Node::_radio->signalReceivedR = value;
}

char *Node::getResultFilename()
{
    struct tm *timeinfo;
    time_t rawtime;
    char *strResponse = (char *)calloc(128,1);

    rawtime = time(NULL);
    timeinfo = localtime(&rawtime);
    strftime(strResponse,128,"%d-%b-%Y %H:%M:%S",timeinfo);

    return strResponse;
}

// Sætter variabler op i Node
void Node::initializeNode()
{
    Packet::crcInit();

    Node::_radio = new NRF24Radio();
    Node::_lastPairRequestMillis = bcm2835_millis();
    Node::_lastSavedFile = bcm2835_millis();

    // Find kendte noder (i home/pi/wasp.conf)
    FILE *optionsFile;
    optionsFile = fopen("/home/pi/wasp/wasp.conf", "a+");
    if(optionsFile != NULL)
    {
        int nodeID = 0;
        char nodeName[100];
    	  while(fscanf(optionsFile, "%d*%s", &nodeID, &nodeName) != EOF)
    	  {
            printf("Kendt node: %d, med navn: %s\n", nodeID, nodeName);
            Node::_receivedThisSession[nodeID] = -1;
        }
    }

    //setup rand() til brug i exponential backoff
    srand(time(NULL));

    fflush(stdout);
}

// Starter hele lortet!
void Node::begin()
{
    printf("Entering main listening loop!\n");
    fflush(stdout);

    char *res = (char *)malloc(32*sizeof(char));
    while(true)
    {
        //Er flaget sat så vi skal sende requests?
        if(!Node::_requested && Node::signalReceived)
        {
            printf("Signal received, sending requests ... ");
            Node::sendRequest(); //Mængde af forsøg defineret i funktion.
            printf("Done!\n");
            Node::setSignalReceived(0);
        }

        //Har vi modtaget fra alle noder?
        if(Node::_requested && Node::receivedFromAllNodes())
        {
            printf("All nodes have returned a value!\n");
            Node::saveSessionResults();
        }

        //Er vi løbet tør for tid?
        if(Node::_requested && Node::_requestTime + REQUEST_TIMEOUT < bcm2835_millis())
        {
            printf("Timelimit reached!\n");
            Node::saveSessionResults();
        }

        //Vent og håndter kommende pakker. listenFor() i stedet?
        res = _radio->listenFor(1000);
        Packet packet(res);
        Node::handlePacket(packet);

        fflush(stdout);
    }
}

void Node::handlePacket(Packet packet)
{
    switch(packet.packetType)
    {
        case Data: // Har modtaget data der skal gemmes!
        {
            //If statement added for at sikre sig at data er tiltænkt main, og ikke en anden. Check af adresee er egentligt overflødig, men bruges under debugging.
            if(packet.addressee == MAIN_NODE_ID && Node::_requested)
            {
                // Send acknowledgement
                Packet ackPacket(DataAcknowledgement, MAIN_NODE_ID, packet.addresser, MAIN_NODE_ID, 0, 0, 0);
                char *enc = ackPacket.encode();
                _radio->broadcast(enc);
                free(enc);
                if(Node::_receivedThisSession[packet.origin] == -1)
                {
                    printf("Received node %d's value %d from node %d\n", packet.origin, packet.value1, packet.addresser);

                    // Ikke modtaget før, så gem værdi!
                    Node::_receivedThisSession[packet.origin] = packet.value1;
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

        case ClearSignal:
            //This is here to do nothing on received clearsignals, relayed by the subnodes.
        break;

        case PairRequestAcknowledgement:
            //Should not happen as only this node sends these.
        break;

        case DataAcknowledgement:
            //No acknowledgements are aimed at this node, as this node never sends data to get acknowledged. Acknowledgements for other nodes are ignored.
        break;

        case DataRequest:
            //Ignoring this, as there is no reason for this to handle realyed datarequests from subnodes.
        break;

        default: //Pretty much only catches error.
            // Hello? Yes, this is default.
            // Hello default, this is broken!
            // No, this is Patrick.
            // Hello Patrick. Fuck off Jonathan
        break;
    }
}

// Gemmer denne sessions resulteter
void Node::saveSessionResults()
{
    Node::_requested = false;

    char path[200] = "/home/pi/wasp/results/";
    char *name = getResultFilename();
    strcat(path,name);
    FILE *resFile = fopen(path, "w");

    printf("Saving results ... ");
    if(resFile != NULL)
    {
        std::map<int, int>::iterator it;
        for (it = Node::_receivedThisSession.begin(); it != Node::_receivedThisSession.end(); it++)
        {
            fprintf(resFile, "%d:%d\n", it->first, it->second);
        }
        fclose(resFile);
        printf("Done! (%s)\n",path);

        //Lav ping-fil med information om skabt fil
        char pingPath[100] = "/var/www/html/ping.txt";

        FILE *pingFile = fopen(pingPath, "w");

        printf("Saving ping-file ... ");
        if(pingFile != NULL)
        {
            fprintf(pingFile,"%s",name);
            fclose(pingFile);
            printf("Done! (%s)\n",pingPath);
        }
        else
        {
            printf("ERROR\n");
        }
    }
    else
    {
        printf("ERROR\n");
    }
    fflush(stdout);
    free(name);

    Node::clearSession();
}

// Renser section
void Node::clearSession()
{
	printf("Clears local data ... \n");
    std::map<int, int>::iterator it;
    for (it = Node::_receivedThisSession.begin(); it != Node::_receivedThisSession.end(); it++)
    {
        Node::_receivedThisSession[it->first] = -1;
    }
    printf("Done!\n");

    // Clear request!
    int attemptsToDo = 12; //Magisk udvalgt på baggrund af... stuff. 6 er i hvert fald for lidt.

    //Byg pakke
    Packet clearPacket(ClearSignal, 0, MAIN_NODE_ID, 0, 0, 0, 0);
    char *enc = clearPacket.encode();

    printf("Sending clear packets ... ");
    //Try it
    for (int i = 1; i <= attemptsToDo; i++)
    {
        // Broadcast
        _radio->broadcast(enc);

        // Backoff
        bcm2835_delay(nextExponentialBackoffDelay(i));
    }
    printf("Done\n");

    //Ryd op
    free(enc);
}

bool Node::receivedFromAllNodes()
{
    bool isDone = true;

    std::map<int, int>::iterator it;
    for (it = Node::_receivedThisSession.begin(); it != Node::_receivedThisSession.end(); it++)
    {
        if(it->second == -1)
        {
            isDone = false;
            break;
        }
    }

    return isDone;
}

void Node::sendRequest()
{
    int attemptsToDo = 6;

    //sæt flag
    _requested = true;
    _requestTime = bcm2835_millis();

    //Byg pakke
    Packet requestPacket(DataRequest, 0, MAIN_NODE_ID, 0, (int)_receivedThisSession.size(), 0, 0);
    char *enc = requestPacket.encode();

    //temp packet
    char *res;

    //Try it
    for (int i = 1; i <= attemptsToDo; i++)
    {
        // Broadcast
        _radio->broadcast(enc);
        // Backoff
        unsigned int totalTimeToWait = nextExponentialBackoffDelay(i);
        unsigned int startTime = bcm2835_millis();
        unsigned int totalTimeWaited = 0;
        while(totalTimeWaited < totalTimeToWait)
        { 
            res = _radio->listenFor();
            Packet packet(res);
            Node::handlePacket(packet);
            totalTimeWaited = bcm2835millis() - startTime
        }
    }

    //Ryd op
    free(enc);
}

unsigned int Node::nextExponentialBackoffDelay(int attemptNumber)
{
    //Minimum backoff
    unsigned int min = 1;
    //Maximum backoff
    unsigned int max = (unsigned int) 1 << (attemptNumber -1);

    //Random value from min to max
    unsigned int ans = (((unsigned int) rand()) % max - min) + min;

    //5 ms added to help ensure a minimum time for listening
    ans += 5;

    return ans;
}
