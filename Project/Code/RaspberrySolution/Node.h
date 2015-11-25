#include <map>

class iRadio;
class Packet;

#ifndef NODE_H
#define NODE_H

class Node
{
public:
    static unsigned short crcTable[256];
    
    static void initializeNode();
    static void begin();
    static void sendRequest(int, int);
    
    
private:
    // ID generating
    static int _currentID;
    static unsigned int _lastPairRequestMillis;
    
    // Current 'session'
    static std::map<int, int> _receivedThisSession;
    
    // misc.
    static iRadio *_radio;
    static void sendRequest(int);

    static void crcInit();
    static void handlePacket(Packet);
    static bool receivedFromAllNodes();
    static void saveSessionResults();
    static void clearSession();
};

#endif
