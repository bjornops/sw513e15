#include <iostream>

#include "MoistureSensor.h" // Burde jo .h   men altså.. Det virker!
#include "Main.h"
#include "Packet.h"
//#include "RadioHandler.h"

int main(int argc, char *argv[])
{
    _waitForAcknowledgement = true;
    _readyToForward = false;
    
    MoistureSensor sensor(1);
    
    std::cout << "Main is running! \\o/" <<  std::endl << "Sensor value: " <<  sensor.read() << std::endl;
    
    return 0;
}

    // FROM PACKETHANDLER //
void handlePacket(Packet packet)
{
    switch(packet.type);
    {
        case Acknowledgement :
        {
            if (_waitForAcknowledgement)
            {
                _waitForAcknowledgement = false;
                _readyToForward = true;
            }
        }
        break;
        case Request :
        {
            if (!_waitForAcknowledgement && !_readyToForward)
            {
                _waitForAcknowledgement = true;
                readPackSend();
            }
        }
        break;
        case Data :
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
        }
        break;
        case PairRequestAcknowledgement :
        {
            // Ur paired dood
        }
        break;
        case ClearSignal :
        {
            // I had nothing to do with it!
        }
        break;
        default:
            std::cout << "Hello? Yes, this is default.";
        break;
    }
}

void readPackSend()
{
    int sensorData = sensor -> read();              // Read
    
    Packet packet = new Packet(sensorData);         // Pack
    
    _radioHandler -> broadcast(packet->toString()); // Send
    _radioHandler -> waitForAccept();
}

void forwardSignal(Packet packet)
{
    _radioHandler -> broadcast(packet->toString()); //TODO
    _radioHandler -> waitForAccept();
}