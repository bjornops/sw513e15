#include "Arduino.h"

#include <EEPROM.h>
#include <stdint.h>

#include "Node.h"
#include "Packet.h"
#include "iRadio.h"
#include "iSensor.h"

// Static declarations
// Private

uint16_t Node::_rejectArray[REJECTSIZE] = {0};
int      Node::_rejectCount = 0;
unsigned long Node::_lastPacketTime;

iSensor *Node::_sensor;
iRadio  *Node::_radio;


// Public
int Node::nodeID = -1;
int Node::parentID = -1;


// Sætter variabler op i Node
void Node::initializeNode(iSensor *sensor, iRadio *radio)
{
    printf("\nNode klar!\n");
    Packet::crcInit();
  
    _sensor = sensor;
    _radio = radio;
  
    randomSeed(analogRead(0));
}

// Starter hele lortet!
void Node::begin()
{
    // Find dit ID her
    bool shouldSendPairRequest = false;
    int ID = loadID();
    if (nodeID != 0)
    {
        nodeID = ID;
        printf("Har nodeID: %d\n", nodeID);
    }
    else
    {
        printf("Har ingen nodeID..\n");
        shouldSendPairRequest = true;
    }
  
    // Haandter!
    if (shouldSendPairRequest)
    {
        printf("Sender pair request!\n");
        sendPairRequest();
    }
    else
    {
        _lastPacketTime = millis();
        int attempt = 1;
        while (true)
        {
            long remainingTimeToClear = (long)((_lastPacketTime + TIMEOUT) - millis());
            
            // Laeser fra radio i maksimum tiden til timeout. Kommer der en pakke afbrydes listenfor og pakken haandteres
            char *res = _radio->listenFor((remainingTimeToClear > 0) ? remainingTimeToClear : 0);
            Packet packet(res);
            handlePacket(packet);  
        }
    }
}

// Gem ID i EEPROM
void Node::saveID(int16_t id)
{
    char *val = (char *)&id;

    EEPROM.write(0, val[0]);
    EEPROM.write(1, val[1]);
}

// Hent ID fra EEPROM
int16_t Node::loadID()
{
    char *val = (char *)malloc(2);
    val[0] = EEPROM.read(0);
    val[1] = EEPROM.read(1);

    int16_t id = 0;
    memcpy(&id, val, 2);
    free(val);
    return id;
}

void Node::handlePacket(Packet packet)
{
    //Pakke modtaget, håndter pakker der kan starte "forløb"
    printf("Packet i switch: %d - %d - %d - %d \n", packet.packetType , packet.addresser, packet.addressee, packet.origin);
    switch (packet.packetType)
    {
        case DataRequest:
        {
            //Kender vi parent skipper vi. Er lifespan på en request 0 eller mindre skipper vi også.
            if (parentID == -1 && packet.value1 > 0 )
            {
                handleDataRequest(packet);
            }
        }
        break;

        case Data: // Har modtaget data der skal videresendes
        {
            if (parentID != -1 && packet.addressee == Node::nodeID)
            {
                handleData(packet);
            }
        }
        break;

        case PairRequestAcknowledgement: // Har modtaget mit ID
        {
            handlePairRequestAcknowledgement(packet);
        }
        break;

        case ClearSignal: // Slet alt!
        {
            if (parentID != -1)
            {
                handleClearSignal(packet);
            }
        }
        break;
        
        //Hvis pakken er Error, DataAcknowledgement, PairRequest. Primær funktion er at levere nulstilling efter ydre listenFor() har kørt den fulde tid uden at modtage en brugbar pakke.
        default:
        {
            handleDefault();
        }
        break;
    }
}

void Node::handleDataRequest(Packet packet)
{
    printf("Har modtaget datarequest fra %d med lifetime %d\n", packet.addresser, packet.value1);
    //Vi kender nu parent.
    parentID = packet.addresser;

    //Prøver at sende data fra sensor. Hvis der modtages acknowledgement returneres true
    int sensorData = random(10, 1000); // _sensor->read(); // Read
    Packet dataPacket(Data, nodeID, parentID, nodeID, sensorData, 0, 0);

    if(beginBroadcasting(dataPacket))
    {
        //Videresender request, men med mindre lifespan end modtaget. Dette umuliggør uendelig videresending af request
        broadcastNewDataRequest((packet.value1 - 1));
        _lastPacketTime = millis();
    }
}

void Node::handleData(Packet packet)
{
    sendDataAcknowledgement(packet.addresser);
    forwardData(packet);
    _lastPacketTime = millis();
}

void Node::handlePairRequestAcknowledgement(Packet packet)
{
    printf("Modtaget ack paa pair!\n");
    if (nodeID == -1) // Default ID
    {
        printf("Har nu faaet ID: %d\n", packet.addressee);
        saveID(packet.addressee);
    }
}

void Node::handleClearSignal(Packet packet)
{
    parentID = -1;
    memset(_rejectArray, 0, REJECTSIZE);

    char *encoded = packet.encode();

    for (int i = 1; i <= REQUEST_AND_CLEAR_ATTEMPTS; i++)
    {
        _radio->broadcast(encoded);
        delay(nextExponentialBackoff(i));
    }
    free(encoded);
    printf("Clearsignal handled - klar igen om 1 sekund\n");
    delay(1000);
    _lastPacketTime = millis();
}

void Node::handleDefault()
{
    //Hvis vi har en parent og der er timeout nu
    printf("Time remaining: %lu", ((_lastPacketTime + TIMEOUT) - millis()));
    if( parentID != -1 && (_lastPacketTime + TIMEOUT) - millis() < 0)
    {
        parentID = -1;
        memset(_rejectArray, 0, REJECTSIZE);
        printf("Timeout now - switch\n");
        _lastPacketTime = millis();
    }
    else if(parentID == -1)
    {
        _lastPacketTime = millis();
    }
}

void Node::broadcastNewDataRequest(int remainingLifespan)
{
    if(remainingLifespan <= 0)
    {
      return;
    }
    
    char *res;

    //Byg pakke
    Packet requestPacket(DataRequest, nodeID, 0, 0, remainingLifespan, 0, 0);
    char *enc = requestPacket.encode();
    int attempt = 1;
    unsigned long attemptTime = nextExponentialBackoff(attempt++);
    unsigned long totalTime = attemptTime;

    for (int i = 1; i <= REQUEST_AND_CLEAR_ATTEMPTS; i++)
    {
        // Broadcast
        _radio->broadcast(enc);

        unsigned long startTime = millis();
        //attemptTime kan i denne kontekst ikke overstige 10 minutter (600000) og derfor kan vi sagtens dette:
        long remainingTime = (long)attemptTime;
        while(remainingTime > 0)
        {
            res = _radio->listenFor((remainingTime > 0) ? remainingTime : 0);

            // Backoff
            res = _radio->listenFor(nextExponentialBackoff(i));
            Packet receivedPacket(res);

            //printf("Packet i newdatarequest: %d - %d - %d - %d \n", receivedPacket.packetType , receivedPacket.addresser, receivedPacket.addressee, receivedPacket.origin);
            if (receivedPacket.packetType == ClearSignal)
            {
                printf("Clearsignal newdatarequest\n");
                handleClearSignal(receivedPacket);
                free(enc);
                return;
            }
            remainingTime = (startTime + attemptTime) - millis();
        }

        //fix tid.
        attemptTime = nextExponentialBackoff(attempt++);
        totalTime += attemptTime;
    }

    //Ryd op
    free(enc);
}

void Node::handleClearSignal(Packet packet)
{
    parentID = -1;
    memset(_rejectArray, 0, REJECTSIZE);
    
    char *encoded = packet.encode();
  
    for (int i = 1; i <= REQUEST_AND_CLEAR_ATTEMPTS; i++)
    {
        _radio->broadcast(encoded);
        delay(nextExponentialBackoff(i));
    }
    free(encoded);
    printf("Clearsignal handled - klar igen om 1 sekund\n");
    delay(1000);
}

void Node::receivedPairRequestAcknowledgement(int newID)
{
    printf("Modtaget ack paa pair!\n");
    if (nodeID == -1) // Default ID
    {
        printf("Har nu faaet ID: %d\n", newID);
        saveID(newID);
    }
}
// Sender pair request
void Node::sendPairRequest()
{
    Packet requestPacket(PairRequest, 0, 0, 0, 0, 0, 0); // Data does not matter, only need 'type'.

    beginBroadcasting(requestPacket);
}

// Begynder at sende pakke indtil den bliver bedt om at stoppe!
bool Node::beginBroadcasting(Packet packet)
{
    char *packetCoding = packet.encode();
    char *res;

    int attempt = 1;
    unsigned long attemptTime = nextExponentialBackoff(attempt);
    unsigned long totalTime = attemptTime;

    while (totalTime < TIMEOUT)
    {
        printf("Attemptnumber: %d\n", attempt);
        printf("Sender pakke med typen: %d og lytter for %ld ms\n\n\n\n", packet.packetType, attemptTime);
        _radio->broadcast(packetCoding);

        unsigned long startTime = millis();
        //attemptTime kan i denne kontekst ikke overstige 10 minutter (600000) og derfor kan vi sagtens dette:
        long remainingTime = (long)attemptTime;
        //Så længe der er remainingTime i det pågældende attempt fortsætter vi med at lytte uden at broadcaste.
        //Nødvendigt fordi vi kun lytter indtil vi modtager en pakke - om den er relevant eller ej er lige meget.
        while(remainingTime > 0)
        {
            res = _radio->listenFor(remainingTime);
            Packet receivedPacket(res);
            //printf("Packet: %d - %d - %d - %d \n", receivedPacket.packetType , receivedPacket.addresser, receivedPacket.addressee, receivedPacket.origin);

            if (receivedPacket.packetType == DataAcknowledgement && receivedPacket.addressee == nodeID)
            {
                printf("Modtaget acknowledgement fra %d\n", packet.addressee);
                free(packetCoding);
                return true;
            }
            else if (receivedPacket.packetType == ClearSignal)
            {
                printf("Clearsignal beginbroadcasting\n");
                handleClearSignal(receivedPacket);
                free(packetCoding);
                return false;
            }

            remainingTime = (startTime + attemptTime) - millis();
        }

        //Opdater tiden for næste attemt
        attempt++;
        attemptTime = nextExponentialBackoff(attempt);
        totalTime += attemptTime;
    }
    //Hvis vi ryger ud hat vi nået et timeout
    free(packetCoding);
    parentID = -1;
    memset(_rejectArray, 0, REJECTSIZE);
    printf("Timeout now\n");
    _lastPacketTime = millis();
    return false;
}

// Udregner exp. backoff delay
unsigned long Node::nextExponentialBackoff(int attemptNumber)
{
  //Sikkerhedstjek så vi ikke ender i overflow hvor 1ms returneres
    attemptNumber = (attemptNumber <= 32) ? attemptNumber : 32;

    //Delay mellem 1 og 1 * 2 ^ ( attemptnumber - 1 )
    unsigned long potentiallyBiggest = ((unsigned long)1 << (attemptNumber - 1));
    unsigned long delay = random(1, potentiallyBiggest);

    //Mængden af tid vi minimum skal lytte for at få svar.
    //f.eks. 1ms er for lidt og derfor skal delay altid være minimum 5
    delay = (delay < 5) ? 5 : delay;
    
    return delay;
}

// Send data videre til parent!
void Node::forwardData(Packet packet)
{
    bool originFound = checkRejectArray(packet.origin);

    //Tjek om data allerede er hørt. Kan ske hvis et barn ikke hørte acknowledgement
    if (!originFound)
    {
        printf("trying to relay now");
        _rejectArray[_rejectCount] = packet.origin;
        _rejectCount = (_rejectCount + 1) % REJECTSIZE;

        packet.addresser = Node::nodeID;
        packet.addressee = Node::parentID;
        packet.updateChecksum();
        beginBroadcasting(packet);
    }
}

//Tjekker om en origin findes i vores rejectarray (origins vi allerede har modtaget fra siden sidste request)
bool Node::checkRejectArray(uint16_t origin)
{
    for (int i = 0; i < REJECTSIZE; i++)
    {
        if (_rejectArray[i] == origin)
        {
            return true;
        }
    }
    return false;
}

void Node::sendDataAcknowledgement(uint16_t addressee)
{
  //Opret og send data acknowledgement til en node
    Packet acknowledgement(DataAcknowledgement, nodeID, addressee, nodeID, 0, 0, 0);
    char *encoded = acknowledgement.encode();
    _radio->broadcast(encoded);
    free(encoded);
}
