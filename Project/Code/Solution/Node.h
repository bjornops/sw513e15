#include "Packet.h"
#include "MoistureSensor.h"
#include "Radio.h"

class Node
{
public:
    static void initializeNode(iSensor, iRadio);
    
private:
    static iSensor *_sensor;
    static iRadio *_radio;
    static bool _waitForAcknowledgement;
    static bool _readyToForward;
    static unsigned short _crcTable[256];
    
    static void crcInit();
    static void readPackSend();
    static void forwardSignal(Packet);
    static void handlePacket(Packet);
};