#import "Interfaces.h"
#include "Packet.h"
#include "MoistureSensor.h"

class Node
{
private:
    static iSensor *_sensor;
    static bool _waitForAcknowledgement;
    static bool _readyToForward;
    
    static void readPackSend();
    static void forwardSignal(Packet);
	
    static void handlePacket(Packet);
};