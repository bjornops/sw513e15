class iRadio;
class Packet;

#ifndef NODE_H
#define NODE_H
#include <map>
#include <time.h>
#include "Packet.h"
#include <stdint.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Radio.h"

class Node
{
public:
    static volatile sig_atomic_t signalReceived;
    
    static void initializeNode();
    static void begin();
    static void sendRequest();
    
    
private:
    // ID generating
    static int _currentID;
    static unsigned int _lastPairRequestMillis;
    static unsigned int _lastSavedFile;
    
    // Current 'session'
    static std::map<int, int> _receivedThisSession;

    // misc.
    static iRadio *_radio;

    static void nextExponentialBackoffDelay(int);
    static void handlePacket(Packet);
    static bool receivedFromAllNodes();
    static char *getResultFilename();
    static void saveSessionResults();
    static void clearSession();
};

#endif
