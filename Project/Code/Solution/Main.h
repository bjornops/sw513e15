#include "Interfaces.h"

private:
    static RadioHandler  *_radioHandler;
    static iSensor *_sensor;
    static bool _waitForAcknowledgement;
    static bool _readyToForward;
    
    static void readPackSend();
    static void forwardSignal(Packet);
	
    static void handlePacket(Packet);

