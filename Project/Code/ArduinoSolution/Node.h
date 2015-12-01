class iRadio;
class iSensor;
class Packet;

#ifndef NODE_H
#define NODE_H
#define REJECTSIZE 10

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
    static unsigned short crcTable[256];
    static int nodeID, parentID;
    
    static void initializeNode(iSensor *, iRadio *);
    
    static void begin();
    static void sendPairRequest();
    
private:
    static iSensor *_sensor;
    static iRadio *_radio;
    static bool _readyToForward;
    static uint16_t _rejectArray[REJECTSIZE]; 
    static int _rejectCount;

    static void receivedPairRequestAcknowledgement(int);
    static void saveID(int);
    static int loadID();
    static void crcInit();
    static void readPackSend();
    static void handlePacket(Packet);
    static void beginBroadcasting(Packet);
    static int nextExponentialBackoff(int);
    static void sendRequests();
    static bool checkRejectArray(uint16_t);
    static void sendDataAcknowledgement(uint16_t);
    static void forwardData(Packet);
};

#endif
