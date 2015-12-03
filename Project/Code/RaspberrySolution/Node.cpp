#include "Node.h"

#define MAIN_NODE_ID 0
#define REQUEST_GENERATE_ID_TIMER 5000
#define FILE_SAVE_TIMER 10000
#define AFTER_SAVE_TIMEOUT 10000 //3sec for lulz

void signalHandler(int);
void getAndSavePID();



iRadio *Node::_radio;
int Node::_currentID = 0;
unsigned int Node::_lastPairRequestMillis = 0;
unsigned int Node::_lastSavedFile = 0;
std::map<int, int> Node::_receivedThisSession;

volatile sig_atomic_t Node::signalReceived = 0;

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
    Node::signalReceived = 1;
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
        //Skal der sendes requests?
        if(Node::signalReceived)
        {
            printf("Signal modtaget! - Sender requests!\n");
            Node::sendRequest(); //Mængde af forsøg defineret i funktion.
            printf("Signal handling overstået! - Requests sendt!\n");
            Node::signalReceived = 0;
        }

        //Vent og håndter kommende pakker. Listen() returnerer en fejlpakke ved signal.
        res = _radio->listen();
        Packet packet(res);
        Node::handlePacket(packet);

        // Er vi færdige?
        if(Node::receivedFromAllNodes())
        {
            Node::saveSessionResults();
            printf("Alt done. Yay!\n");
        }
        fflush(stdout);
    }
}

void Node::handlePacket(Packet packet)
{
    switch(packet.packetType)
    {
        case Data: // Har modtaget data der skal gemmes!
        {
            //If statement added for at sikre sig at data er tiltænkt main, og ikke en anden. Er egentligt overflødig, men bruges under debugging.
            if(packet.addressee == MAIN_NODE_ID)
            {
                printf("Noden %d sendte node %d's værdi, som er %d.\n", packet.addresser, packet.origin, packet.sensor1);

                if(Node::_receivedThisSession[packet.origin] == -1 && bcm2835_millis() - AFTER_SAVE_TIMEOUT >= Node::_lastSavedFile)
                {
                    // Ikke modtaget før, så gem værdi!
                    Node::_receivedThisSession[packet.origin] = packet.sensor1;
                }

                // Send acknowledgement
                Packet ackPacket(DataAcknowledgement, MAIN_NODE_ID, packet.addresser, MAIN_NODE_ID, 0, 0, 0);
                char *enc = ackPacket.encode();
                _radio->broadcast(enc);
                free(enc);
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
            printf("Har fået en skipable packet fra %d med typen: %d\n", packet.addresser, packet.packetType);
        break;
    }
}

// Gemmer denne sessions resulteter
void Node::saveSessionResults()
{
    Node::_lastSavedFile = bcm2835_millis();

    printf("Ikke mere data at indsamle, gemmer.\n");

    char path[200] = "/home/pi/wasp/results/";
    char *name = getResultFilename();
    strcat(path,name);
    FILE *resFile = fopen(path, "w");

    //Lav fil med data
    if(resFile != NULL)
    {
        std::map<int, int>::iterator it;
        for (it = Node::_receivedThisSession.begin(); it != Node::_receivedThisSession.end(); it++)
        {
            fprintf(resFile, "%d:%d\n", it->first, it->second);
        }
        fclose(resFile);
        printf("%s successfuldt gemt!\n",path);

        //Lav ping-fil med information om skabt fil
        char pingPath[100] = "/var/www/html/ping.txt";

        FILE *pingFile = fopen(pingPath, "w");
        if(pingFile != NULL)
        {
            fprintf(pingFile,"%s",name);
            fclose(pingFile);
            printf("%s successfuldt gemt!\n",pingPath);
        }
        else
        {
            printf("Noget gik galt under skrivning til %s\n",pingPath);
        }
    }
    else
    {
        printf("Noget gik galt under skrivning til %s\n",path);
    }

    fflush(stdout);

    free(name);
    Node::clearSession();
}

// Renser section
void Node::clearSession()
{
	printf("Rydder data!\n");
    std::map<int, int>::iterator it;
    for (it = Node::_receivedThisSession.begin(); it != Node::_receivedThisSession.end(); it++)
    {
        Node::_receivedThisSession[it->first] = -1;
    }

    // Clear request!
    int attemptsToDo = 12; //Magisk udvalgt på baggrund af... stuff. 6 er i hvert fald for lidt.

    //Byg pakke
    Packet clearPacket(ClearSignal, 0, MAIN_NODE_ID, 0, 0, 0, 0);
    char *enc = clearPacket.encode();

    printf("Sender clear packets .. ");
    //Try it
    for (int i = 1; i <= attemptsToDo; i++)
    {
        // Broadcast
        _radio->broadcast(enc);

        // Backoff
        nextExponentialBackoffDelay(i);
    }
    printf("Done\n");

    //Ryd op
    free(enc);
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
    Packet requestPacket(DataRequest, 0, MAIN_NODE_ID, 0, (int)_receivedThisSession.size(), 0, 0);
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
