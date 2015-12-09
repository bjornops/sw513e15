class iRadio;
class iSensor;
class Packet;

#ifndef NODE_H
#define NODE_H
#define REJECTSIZE 10
#define TIMEOUT 600000
#define REQUEST_AND_CLEAR_ATTEMPTS 5

class Node
{
public:
    static int nodeID, parentID;

    static void initializeNode(iSensor *, iRadio *);
    static void begin();

private:
    static iSensor *_sensor;
    static iRadio *_radio;
    static uint16_t _rejectArray[REJECTSIZE];
    static int _rejectCount;
    static unsigned long _lastPacketTime;

    //Eeprom
    static void saveID(int);
    static int loadID();

    //Handle packets
    static void handlePacket(Packet);
    static void handleDataRequest(Packet);
    static void handleData(Packet);
    static void handlePairRequestAcknowledgement(Packet);
    static void handleClearSignal(Packet);
    static void handleDefault();

    static unsigned long nextExponentialBackoff(int);
    static void sendRequests();
    static void sendPairRequest();
    static bool checkRejectArray(uint16_t);
    static void sendDataAcknowledgement(uint16_t);
    static void forwardData(Packet);
    static void broadcastNewDataRequest(int);
    static bool beginBroadcasting(Packet);
};

#endif
