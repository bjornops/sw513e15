#import "Interfaces.h"
#import "Packet.h"

class PacketHandler
{
private:
    static iRadio  *radio;
    static iSensor *sensor;
    static bool waitForAcknowledgement;
    static bool readyToForward;
    
    static void readPackSend();
    static void sendRequest();
    static void forwardSignal();
    static void requestReceived();
    static void forwardSignal(Packet);

public:
    static void determineAction(Packet);
};