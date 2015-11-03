enum PacketType
{
  Acknowledgement,
  Request,
  Data,
  PairRequest,
  PairRequestAcknowledgement
};

class Packet
{
public:
  PacketType type;
  Packet(int sensorData)
  {
    
  }
};

class iRadio
{
public:
  virtual void broadcast(Packet packet);
  virtual void waitForAccept();
  virtual void broadcastRequest();
};

class iSensor
{
public:
  virtual int read() = 0;
};


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