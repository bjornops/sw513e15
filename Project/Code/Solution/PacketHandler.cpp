#include "PacketHandler.h"
#include <iostream>

iRadio *PacketHandler::radio = NULL;
iSensor *PacketHandler::sensor = NULL;

bool PacketHandler::waitForAcknowledgement = false;
bool PacketHandler::readyToForward = false;

void PacketHandler::determineAction(Packet packet)
{
    switch(packet.type)
    {
      case Acknowledgement : 
      {
          if (waitForAcknowledgement)
          {
            waitForAcknowledgement = false;
            readyToForward         = true;
          }
      }
      break;
    
      case Request :
      {
          if(!waitForAcknowledgement)
          {
            readPackSend();
            waitForAcknowledgement = true;
          }
          requestReceived();
      }
      break;
    
      case Data :
      {
          if (readyToForward)
          {
            forwardSignal();
          }
      }
      break;
    
      case PairRequest :
        // Wanna be a pair..
      break;
    
      case PairRequestAcknowledgement :
        // .. OF SOCKS!
      break;
    
      default:
          std::cout << "FUCK";
      break;
    }
}

// ACTIONS //
void PacketHandler::readPackSend()
{
    int sensorData = sensor->read();     //Read from sensor 
    Packet packet = Packet(sensorData); //Pack data

    radio->broadcast(packet);            //Send data
    radio->waitForAccept();            //Wait for accept here?
}

void PacketHandler::sendRequest()
{
    radio->broadcastRequest();
}

void PacketHandler::forwardSignal(Packet forwardData)
{
    radio->broadcast(forwardData);
}

void PacketHandler::requestReceived()
{

}

void PacketHandler::forwardSignal()
{

}
