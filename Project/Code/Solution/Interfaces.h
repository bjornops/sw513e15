class iRadio
{
public:
  virtual void broadcast(Packet);
  virtual void waitForAccept();
  virtual void broadcastRequest();
};

class iSensor
{
public:
  virtual int read() = 0;
};