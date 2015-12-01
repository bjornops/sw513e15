#include "Arduino.h"

#include <EEPROM.h>
#include <stdint.h>

#include "Node.h"
#include "Packet.h"
#include "iRadio.h"
#include "iSensor.h"

// Static declarations
// Private
bool Node::_readyToForward = false;

uint16_t Node::_rejectArray[REJECTSIZE] = {0};
int      Node::_rejectCount = 0;

iSensor *Node::_sensor;
iRadio  *Node::_radio;

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
    while (true)
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
  printf("Packet i switch: %d - %d - %d - %d \n", packet.packetType , packet.addresser, packet.addressee, packet.origin);
  switch (packet.packetType)
  {
    case DataAcknowledgement: // Acknowledgement modtaget (Dvs. mit data er accepteret)
      {
        //Dunno wut 2 do
      }
      break;
    case DataRequest: // Request modtaget
      {
        if (!_readyToForward && parentID == -1)
        {
          printf("Har modtaget datarequest fra %d\n", packet.addresser);
          parentID = packet.addresser;
          
          //Prøver at sende data fra sensor. Hvis der modtages acknowledgement returneres true
          if(readPackSend())
          {
            broadcastNewDataRequest();
          }
        }
      }
      break;
    case Data: // Har modtaget data der skal videresendes
      {
        if (_readyToForward)
        {
          sendDataAcknowledgement(packet.addressee);
          forwardData(packet);
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
        }
      }
      break;
    case PairRequest:
      {

      }
      break;
    default:
      //std::cout << "Hello? Yes, this is default.";
      // Hello default, this is broken!
      break;
  }
}

void Node::broadcastNewDataRequest()
{
  int attemptsToDo = 6;
  char *res;

  //Byg pakke
  Packet requestPacket(DataRequest, nodeID, 0, 0, 0, 0, 0);
  char *enc = requestPacket.encode();

  //Try it
  for (int i = 1; i <= attemptsToDo; i++)
  {
    // Broadcast
    _radio->broadcast(enc);

    // Backoff
    res = _radio->listenFor(nextExponentialBackoff(i));
    Packet receivedPacket(res);

    printf("Packet i newdatarequest: %d - %d - %d - %d \n", receivedPacket.packetType , receivedPacket.addresser, receivedPacket.addressee, receivedPacket.origin);
    if (receivedPacket.packetType == ClearSignal)
    {
      printf("Clearsignal newdatarequest\n");
      handleClearSignal(receivedPacket);
      break;
    }
  }

  //Ryd op
  free(enc);
}

void Node::handleClearSignal(Packet packet)
{
  _readyToForward = false;
  parentID = -1;
  shouldKeepSendingPacket = false;
  
  char *encoded = packet.encode();

  for (int i = 0; i < 5; i++)
  {
    _radio->broadcast(encoded);
    delay(100);
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
    shouldKeepSendingPacket = false;
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
  shouldKeepSendingPacket = true;


  char *packetCoding = packet.encode();
  unsigned int attempt = 1;
  char *res;

  printf("Sender pakke med typen: %d og lytter for %d ms\n", packet.packetType, nextExponentialBackoff(attempt));

  while (shouldKeepSendingPacket)
  {
    _radio->broadcast(packetCoding);
    
    res = _radio->listenFor(nextExponentialBackoff(attempt));
    attempt++;
    Packet receivedPacket(res);
    printf("Packet: %d - %d - %d - %d \n", receivedPacket.packetType , receivedPacket.addresser, receivedPacket.addressee, receivedPacket.origin);

    if (receivedPacket.packetType == DataAcknowledgement && receivedPacket.addressee == nodeID)
    {
      printf("Modtaget acknowledgement fra %d\n", packet.addressee);
      _readyToForward = true;
      shouldKeepSendingPacket = false;
      return true;
    }
    else if (receivedPacket.packetType == ClearSignal)
    {
      printf("Clearsignal beginbroadcasting\n");
      handleClearSignal(receivedPacket);
      return false;
    }
  }
  free(packetCoding);
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

// Fill crcTable with values
void Node::crcInit()
{
  unsigned short remainder; // 2 byte remainder (according to CRC16/CCITT standard)
  unsigned short dividend; // What are you?
  int bit; // bit counter

  for (dividend = 0; dividend < 256; dividend++) //foreach value of 2 bytes/8 bits
  {
    remainder = dividend << (WIDTH - 8);//

    for (bit = 0; bit < 8; bit++)
    {
      if (remainder & TOPBIT) // MSB = 1 => divide by POLYNOMIAL
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



















