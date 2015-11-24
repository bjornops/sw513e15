class iRadio;
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
    static void initializeNode();
    static unsigned short crcTable[256];
    static void begin();
    static void sendRequest(int, int);
    
private:
    static iRadio *_radio;
    static bool _waitForAcknowledgement;
    static bool _readyToForward;
    
    static void crcInit();
    static void handlePacket(Packet);
};

#endif
