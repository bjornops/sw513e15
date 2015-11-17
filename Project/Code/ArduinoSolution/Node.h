class iRadio;
class iSensor;
class Packet;

#ifndef NODE_H
#define NODE_H

enum NodeState
{
    WaitingState,
    RequestingState,
    RelayingDataState,
    SendingOwnDataState
};

class Node
{
public:
    static void initializeNode(iSensor *, iRadio *);
    static unsigned short crcTable[256];
    
    static void begin();
    static void sendPairRequest();
    
private:
    static iSensor *_sensor;
    static iRadio *_radio;
    static bool _waitForAcknowledgement;
    static bool _readyToForward;
    
    static void crcInit();
    static void readPackSend();
    static void forwardSignal(Packet);
    static void handlePacket(Packet);
};

#endif
