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
#include "NRF24Radio.h"

class Node
{
public:
    static volatile sig_atomic_t signalReceived;
    static void setSignalReceived(int);

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
    static bool _requested;
    static unsigned int _requestTime;

    // misc.
    static iRadio *_radio;

    static unsigned int nextExponentialBackoffDelay(int);
    static void handlePacket(Packet);
    static bool receivedFromAllNodes();
    static char *getResultFilename();
    static void saveSessionResults();
    static void clearSession();
};

#endif
