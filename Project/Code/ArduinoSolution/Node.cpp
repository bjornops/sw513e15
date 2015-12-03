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


// Andre declarations
static PacketType currentHandlingPacketType;

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
    while (true)
    {
      // Laeser fra radio og laver til pakke
      long remainingTimeToClear = (_lastPacketTime + TIMEOUT) - millis();
      Serial.println(remainingTimeToClear);
      char *res = _radio->listenFor((remainingTimeToClear > 0) ? remainingTimeToClear : 0);
      Packet packet(res);
      handlePacket(packet);   
    }
  }
}

void Node::saveID(int16_t id)
{
  char *val = (char *)&id;

  EEPROM.write(0, val[0]);
  EEPROM.write(1, val[1]);
}

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
  //printf("Packet i switch: %d - %d - %d - %d \n", packet.packetType , packet.addresser, packet.addressee, packet.origin);
  switch (packet.packetType)
  {
    case DataRequest:
      {
        //Kender vi parent skipper vi. Er lifespan på en request 0 eller mindre skipper vi også.
        if (parentID == -1 && packet.sensor1 > 0 /*&& packet.addresser != 0*/)
        {
          printf("Har modtaget datarequest fra %d\n", packet.addresser);
          //Vi kender nu parent.
          parentID = packet.addresser;
          
          //Prøver at sende data fra sensor. Hvis der modtages acknowledgement returneres true
          if(readPackSend())
          {
            broadcastNewDataRequest((packet.sensor1 - 1));
            _lastPacketTime = millis();
          }
        }
      }
      break;
      
    case Data: // Har modtaget data der skal videresendes
      {
        if (parentID != -1)
        {
          sendDataAcknowledgement(packet.addressee);
          forwardData(packet);
          _lastPacketTime = millis();
        }
      }
      break;

    case PairRequestAcknowledgement: // Har modtaget mit ID
      {
        receivedPairRequestAcknowledgement(packet.addressee);
      }
      break;
      
    case ClearSignal: // Slet alt!
      {
        if (parentID != -1)
        {
          printf("Clearsignal mainswitch\n");
          handleClearSignal(packet);
          _lastPacketTime = millis();
        }
      }
      break;
      
    case PairRequest:
      {

      }
      break;
    
    //Hvis pakken er Error, DataAcknowledgement, PairRequest. Primær funktion er at levere nulstilling efter ydre listenFor() har kørt den fulde tid uden at modtage en brugbar pakke.
    default:
      {
        //Hvis vi har en parent og der er timeout nu
        if( parentID != -1 && (_lastPacketTime + TIMEOUT) - millis() > 0)
        {
          parentID = -1;
          printf("Timeout now\n");
          _lastPacketTime = millis();
        }
        else if(parentID == -1)
        {
           _lastPacketTime = millis();
        }
      }
      break;
  }
}

void Node::broadcastNewDataRequest(int remainingLifespan)
{
  char *res;


  //Byg pakke
  Packet requestPacket(DataRequest, nodeID, 0, 0, remainingLifespan, 0, 0);
  char *enc = requestPacket.encode();
  unsigned int attempt = 1;
  unsigned long attemptTime = nextExponentialBackoff(attempt++);
  unsigned long totalTime = attemptTime;
  
  for (int i = 1; i <= REQUEST_AND_CLEAR_ATTEMPTS; i++)
  {
    // Broadcast
    _radio->broadcast(enc);
    
    long startTime = millis();
    long remainingTime = attemptTime;
    while(remainingTime > 0)
    {
      res = _radio->listenFor(remainingTime);

      // Backoff
      res = _radio->listenFor(nextExponentialBackoff(i));
      Packet receivedPacket(res);
  
      //printf("Packet i newdatarequest: %d - %d - %d - %d \n", receivedPacket.packetType , receivedPacket.addresser, receivedPacket.addressee, receivedPacket.origin);
      if (receivedPacket.packetType == ClearSignal)
      {
        printf("Clearsignal newdatarequest\n");
        handleClearSignal(receivedPacket);
        break;
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
  
  char *encoded = packet.encode();

  for (int i = 1; i <= REQUEST_AND_CLEAR_ATTEMPTS; i++)
  {
    _radio->broadcast(encoded);
    delay(nextExponentialBackoff(i));
  }
  free(encoded);
  printf("Clearsignal handled!!!!! (Venter 1 sec)\n");
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
  currentHandlingPacketType = PairRequest;

  beginBroadcasting(requestPacket);
}

// Begynder at sende pakke indtil den bliver bedt om at stoppe! (Exponential backoff handler!)


bool Node::beginBroadcasting(Packet packet)
{


  char *packetCoding = packet.encode();
  char *res;


  unsigned int attempt = 1;
  unsigned long attemptTime = nextExponentialBackoff(attempt);
  unsigned long totalTime = attemptTime;

  printf("Sender pakke med typen: %d og lytter for %d ms\n", packet.packetType, attemptTime);
  while (totalTime < TIMEOUT)
  {
    _radio->broadcast(packetCoding);
    
    long startTime = millis();
    long remainingTime = attemptTime;
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
    
    //fix tid.
    attempt++;
    attemptTime = nextExponentialBackoff(attempt);
    totalTime += attemptTime;
  }
  free(packetCoding);
  return false;
}

int Node::nextExponentialBackoff(unsigned int attemptNumber)
{
  //Delay mellem 1 og 1 * 2 ^ ( attemptnumber - 1 )
  unsigned int delay = random(1, (1 << (attemptNumber - 1)) + 1);
  return delay;
}


void Node::sendRequests()
{

}

// Request modtager. Send data hjem, og request videre
bool Node::readPackSend()
{
  int sensorData = random(10, 1000); // _sensor->read(); // Read
  printf("pakke fra: %d\n", nodeID);
  Packet dataPacket(Data, nodeID, parentID, nodeID, sensorData, 0, 0);
  return beginBroadcasting(dataPacket);
}


// Send data videre til min forlaeldr!
void Node::forwardData(Packet packet)
{
  bool originFound = checkRejectArray(packet.origin);

  if (!originFound)
  {
    _rejectArray[_rejectCount] = packet.origin;
    _rejectCount = (_rejectCount + 1) % REJECTSIZE;

    packet.addresser = Node::nodeID;
    packet.addressee = Node::parentID;
    packet.updateChecksum();
    beginBroadcasting(packet);

  }
}

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
  // Send acknowledgement til sender af data
  Packet acknowledgement(DataAcknowledgement, nodeID, addressee, nodeID, 0, 0, 0);
  // TODO: Following should be in another reuseable function.
  char *encoded = acknowledgement.encode();
  _radio->broadcast(encoded);
  free(encoded);
}


















