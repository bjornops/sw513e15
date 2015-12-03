class iRadio;
class iSensor;
class Packet;

#ifndef NODE_H
#define NODE_H
#define REJECTSIZE 10
#define TIMEOUT 600000
#define REQUEST_AND_CLEAR_ATTEMPTS 5

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
    static uint16_t _rejectArray[REJECTSIZE]; 
    static int _rejectCount;
    static unsigned long _lastPacketTime;

    static void receivedPairRequestAcknowledgement(int);
    static void saveID(int);
    static int loadID();
    static void crcInit();
    static bool readPackSend();
    static void handlePacket(Packet);
    static void handleClearSignal(Packet);
    static bool beginBroadcasting(Packet);
    static int nextExponentialBackoff(unsigned int);
    static void sendRequests();
    static bool checkRejectArray(uint16_t);
    static void sendDataAcknowledgement(uint16_t);
    static void forwardData(Packet);
    static void broadcastNewDataRequest();
};

#endif
